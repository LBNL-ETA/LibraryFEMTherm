#include <variant>

#include <lbnl/optional.hxx>

#include "Utils.hxx"

#include "LibraryUtilities/Common.hxx"

namespace BCLibrary
{
    namespace
    {
        //! A steady value, or nothing when the input reads a series instead.
        std::optional<double> constantOf(const Source & source)
        {
            return lbnl::extend(lbnl::get_if_opt<Constant>(source))
              .transform([](const Constant & constant) { return constant.value; })
              .raw();
        }

        std::optional<double> constantOf(const std::optional<Source> & source)
        {
            return lbnl::extend(source)
              .and_then([](const Source & inner) { return constantOf(inner); })
              .raw();
        }

        const SurfaceExchange * asExchange(const BoundaryCondition & bc)
        {
            return std::get_if<SurfaceExchange>(&bc.data);
        }

        const RadiationOptions * radiationOf(const BoundaryCondition & bc)
        {
            const auto * exchange{asExchange(bc)};
            if(exchange == nullptr || !exchange->radiation.has_value())
            {
                return nullptr;
            }
            return &exchange->radiation.value();
        }
    }   // namespace

    bool isRadiationSurface(const BoundaryCondition & bc)
    {
        return std::holds_alternative<RadiationSurface>(bc.data);
    }

    bool isAdiabatic(const BoundaryCondition & bc)
    {
        if(std::holds_alternative<NoExchange>(bc.data))
        {
            return true;
        }
        const auto * exchange{asExchange(bc)};
        if(exchange == nullptr || !exchange->convection.has_value())
        {
            return false;
        }
        const auto film{constantOf(exchange->convection->filmCoefficient)};
        return film.has_value() && LibraryCommon::isEqual(film.value(), 0.0);
    }

    bool isConvective(const BoundaryCondition & bc)
    {
        const auto * exchange{asExchange(bc)};
        if(exchange == nullptr || !exchange->convection.has_value())
        {
            return false;
        }
        const auto film{constantOf(exchange->convection->filmCoefficient)};
        return film.has_value() && film.value() > 0.0;
    }

    bool isConstantFlux(const BoundaryCondition & bc)
    {
        const auto * exchange{asExchange(bc)};
        if(exchange == nullptr)
        {
            return false;
        }
        const auto flux{constantOf(exchange->flux)};
        return flux.has_value() && !LibraryCommon::isEqual(flux.value(), 0.0);
    }

    bool isAutoGreyBodyRadiation(const BoundaryCondition & bc)
    {
        const auto * radiation{radiationOf(bc)};
        return radiation != nullptr && std::holds_alternative<AutomaticEnclosure>(*radiation);
    }

    bool isGreyBodyRadiation(const BoundaryCondition & bc)
    {
        if(isRadiationSurface(bc))
        {
            return true;
        }
        const auto * radiation{radiationOf(bc)};
        return radiation != nullptr
               && (std::holds_alternative<AutomaticEnclosure>(*radiation)
                   || std::holds_alternative<ManualEnclosure>(*radiation));
    }

    bool isBlackBodyRadiation(const BoundaryCondition & bc)
    {
        const auto * radiation{radiationOf(bc)};
        return radiation != nullptr && std::holds_alternative<BlackBodyRadiation>(*radiation);
    }

    bool isLinearizedRadiation(const BoundaryCondition & bc)
    {
        const auto * radiation{radiationOf(bc)};
        return radiation != nullptr && std::holds_alternative<FixedCoefficientRadiation>(*radiation);
    }

    bool isRadiationBC(const BoundaryCondition & bc)
    {
        return isGreyBodyRadiation(bc) || isAutoGreyBodyRadiation(bc) || isBlackBodyRadiation(bc)
               || isLinearizedRadiation(bc);
    }

    std::optional<double> getRelativeHumidity(const BoundaryCondition & bc)
    {
        if(const auto * exchange{asExchange(bc)})
        {
            return constantOf(exchange->relativeHumidity);
        }
        if(const auto * prescribed{std::get_if<PrescribedState>(&bc.data)})
        {
            return constantOf(prescribed->relativeHumidity);
        }
        if(std::holds_alternative<NoExchange>(bc.data))
        {
            return constantOf(SurfaceExchange{}.relativeHumidity);
        }
        return std::nullopt;
    }

    std::optional<double> airTemperature(const BoundaryCondition & bc)
    {
        // No exchange has always reached a steady consumer as a zero-film convection
        // against a zero air temperature, so that is what it answers here too.
        if(std::holds_alternative<NoExchange>(bc.data))
        {
            return 0.0;
        }
        const auto * exchange{asExchange(bc)};
        if(exchange == nullptr || !exchange->convection.has_value())
        {
            return std::nullopt;
        }
        return constantOf(exchange->convection->airTemperature);
    }

    std::optional<double> filmCoefficient(const BoundaryCondition & bc)
    {
        if(std::holds_alternative<NoExchange>(bc.data))
        {
            return 0.0;
        }
        const auto * exchange{asExchange(bc)};
        if(exchange == nullptr || !exchange->convection.has_value())
        {
            return std::nullopt;
        }
        return constantOf(exchange->convection->filmCoefficient);
    }

    std::optional<double> radiationTemperature(const BoundaryCondition & bc)
    {
        if(const auto * surface{std::get_if<RadiationSurface>(&bc.data)})
        {
            return surface->temperature;
        }
        const auto * radiation{radiationOf(bc)};
        if(radiation == nullptr)
        {
            return std::nullopt;
        }
        if(const auto * automatic{std::get_if<AutomaticEnclosure>(radiation)})
        {
            // An enclosure carrying no temperature of its own follows the air, the way
            // libraries converted before the field existed always have. Answering nothing
            // here would leave the enclosure without a temperature at all.
            if(automatic->temperature.has_value())
            {
                return constantOf(automatic->temperature.value());
            }
            return airTemperature(bc);
        }
        if(const auto * blackBody{std::get_if<BlackBodyRadiation>(radiation)})
        {
            return constantOf(blackBody->temperature);
        }
        if(const auto * fixed{std::get_if<FixedCoefficientRadiation>(radiation)})
        {
            return constantOf(fixed->temperature);
        }
        return std::nullopt;
    }

    std::optional<double> radiationFilmCoefficient(const BoundaryCondition & bc)
    {
        const auto * radiation{radiationOf(bc)};
        if(radiation == nullptr)
        {
            return std::nullopt;
        }
        if(const auto * fixed{std::get_if<FixedCoefficientRadiation>(radiation)})
        {
            return constantOf(fixed->coefficient);
        }
        return std::nullopt;
    }

    std::optional<double> emissivity(const BoundaryCondition & bc)
    {
        if(const auto * surface{std::get_if<RadiationSurface>(&bc.data)})
        {
            return surface->emissivity;
        }
        const auto * radiation{radiationOf(bc)};
        if(radiation == nullptr)
        {
            return std::nullopt;
        }
        // Only the enclosure reports one. A black body's emissivity reaches the solver
        // through viewFactor instead, which is the shape the legacy accessors had.
        if(const auto * automatic{std::get_if<AutomaticEnclosure>(radiation)})
        {
            return constantOf(automatic->emissivity);
        }
        return std::nullopt;
    }

    std::optional<double> constantHeatFlux(const BoundaryCondition & bc)
    {
        const auto * exchange{asExchange(bc)};
        if(exchange == nullptr)
        {
            return std::nullopt;
        }
        return constantOf(exchange->flux);
    }

    std::optional<double> viewFactor(const BoundaryCondition & bc)
    {
        // Deliberately the emissivity - see the header. The legacy accessor reported it
        // in this slot and the solver input has been built from it ever since.
        const auto * radiation{radiationOf(bc)};
        if(radiation == nullptr)
        {
            return std::nullopt;
        }
        if(const auto * blackBody{std::get_if<BlackBodyRadiation>(radiation)})
        {
            return constantOf(blackBody->emissivity);
        }
        return std::nullopt;
    }
}   // namespace BCLibrary
