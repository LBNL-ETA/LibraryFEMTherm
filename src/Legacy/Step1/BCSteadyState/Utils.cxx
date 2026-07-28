#include <variant>
#include <optional>

#include "Utils.hxx"
#include "SteadyState.hxx"
#include "LibraryUtilities/Common.hxx"

namespace BCSteadyStateLibrary
{
    namespace
    {
        struct RadiationEnclosureVisitor
        {
            bool operator()(const AutomaticEnclosure &) const
            {
                return true;
            }

            bool operator()(const ManualEnclosure &) const
            {
                return true;
            }

            template<typename T>
            bool operator()(const T &) const
            {
                return false;
            }
        };

        struct BoundaryConditionGreyBodyVisitor
        {
            bool operator()(const Comprehensive & comprehensive) const
            {
                if(comprehensive.radiation.has_value() && comprehensive.radiation->radiation.has_value())
                {
                    return std::visit(RadiationEnclosureVisitor{}, *(comprehensive.radiation->radiation));
                }
                return false;
            }

            bool operator()(const RadiationSurface &) const
            {
                return true;
            }

            template<typename T>
            bool operator()(const T &) const
            {
                return false;
            }
        };
    }   // namespace

    bool isGreyBodyRadiation(const BCSteadyStateLibrary::BoundaryCondition & bc)
    {
        return std::visit(BoundaryConditionGreyBodyVisitor{}, bc.data);
    }

    namespace
    {
        struct RadiationAutoEnclosureVisitor
        {
            bool operator()(const AutomaticEnclosure &) const
            {
                return true;
            }

            template<typename T>
            bool operator()(const T &) const
            {
                return false;
            }
        };

        struct BoundaryConditionAutoEnclosureVisitor
        {
            bool operator()(const Comprehensive & comprehensive) const
            {
                if(comprehensive.radiation.has_value() && comprehensive.radiation->radiation.has_value())
                {
                    return std::visit(RadiationAutoEnclosureVisitor{}, *(comprehensive.radiation->radiation));
                }
                return false;
            }

            template<typename T>
            bool operator()(const T &) const
            {
                return false;
            }
        };
    }   // namespace

    bool isAutoGreyBodyRadiation(const BCSteadyStateLibrary::BoundaryCondition & bc)
    {
        return std::visit(BoundaryConditionAutoEnclosureVisitor{}, bc.data);
    }

    namespace
    {
        struct BlackBodyVisitor
        {
            bool operator()(const BlackBodyRadiation &) const
            {
                return true;
            }

            template<typename T>
            bool operator()(const T &) const
            {
                return false;
            }
        };

        struct BoundaryConditionBlackBodyVisitor
        {
            bool operator()(const Comprehensive & comprehensive) const
            {
                if(comprehensive.radiation.has_value() && comprehensive.radiation->radiation.has_value())
                {
                    return std::visit(BlackBodyVisitor{}, *(comprehensive.radiation->radiation));
                }
                return false;
            }

            template<typename T>
            bool operator()(const T &) const
            {
                return false;
            }
        };
    }   // namespace

    bool isBlackBodyRadiation(const BCSteadyStateLibrary::BoundaryCondition & bc)
    {
        return std::visit(BoundaryConditionBlackBodyVisitor{}, bc.data);
    }

    namespace
    {
        struct LinearizedRadiationVisitor
        {
            bool operator()(const LinearizedRadiation &) const
            {
                return true;
            }

            template<typename T>
            bool operator()(const T &) const
            {
                return false;
            }
        };

        struct BoundaryConditionLinearizedRadiationVisitor
        {
            bool operator()(const Comprehensive & comprehensive) const
            {
                if(comprehensive.radiation.has_value() && comprehensive.radiation->radiation.has_value())
                {
                    return std::visit(LinearizedRadiationVisitor{}, comprehensive.radiation->radiation.value());
                }
                return false;
            }

            template<typename T>
            bool operator()(const T &) const
            {
                return false;
            }
        };
    }   // namespace

    bool isLinearizedRadiation(const BCSteadyStateLibrary::BoundaryCondition & bc)
    {
        return std::visit(BoundaryConditionLinearizedRadiationVisitor{}, bc.data);
    }

    bool isRadiationBC(const BoundaryCondition & bc)
    {
        return isGreyBodyRadiation(bc) || isAutoGreyBodyRadiation(bc) || isBlackBodyRadiation(bc)
               || isLinearizedRadiation(bc);
    }

    bool isRadiationSurface(const BoundaryCondition & bc)
    {
        return std::holds_alternative<RadiationSurface>(bc.data);
    }

    bool isSimplified(const BoundaryCondition& bc)
    {
        return std::holds_alternative<Simplified>(bc.data);
    }

    bool isComprehensive(const BoundaryCondition & bc)
    {
        return std::holds_alternative<Comprehensive>(bc.data);
    }

    namespace
    {
        class CheckAdiabaticVisitor
        {
        public:
            bool operator()(const BCSteadyStateLibrary::Simplified & simplified) const
            {
                return simplified.filmCoefficient == 0.0;
            }

            bool operator()(const BCSteadyStateLibrary::Comprehensive & comprehensive) const
            {
                if(comprehensive.convection.has_value())
                {
                    return comprehensive.convection.value().filmCoefficient == 0.0;
                }
                return false;
            }

            bool operator()(const BCSteadyStateLibrary::RadiationSurface &) const
            {
                return false;
            }
        };

    }   // end anonymous namespace

    bool isAdiabatic(const BCSteadyStateLibrary::BoundaryCondition & bc)
    {
        return std::visit(CheckAdiabaticVisitor(), bc.data);
    }

    namespace
    {
        class CheckConvectiveVisitor
        {
        public:
            bool operator()(const BCSteadyStateLibrary::Simplified & simplified) const
            {
                return simplified.filmCoefficient > 0.0;
            }

            bool operator()(const BCSteadyStateLibrary::Comprehensive & comprehensive) const
            {
                if(comprehensive.convection.has_value())
                {
                    return comprehensive.convection.value().filmCoefficient > 0.0;
                }
                return false;
            }

            bool operator()(const BCSteadyStateLibrary::RadiationSurface &) const
            {
                return false;
            }
        };

    }   // end anonymous namespace

    bool isConvective(const BoundaryCondition & bc)
    {
        return std::visit(CheckConvectiveVisitor{}, bc.data);
    }

    namespace
    {
        struct ConstantFluxVisitor
        {
            bool operator()(const Comprehensive & comprehensive) const
            {
                return comprehensive.constantFlux.has_value()
                       && !LibraryCommon::isEqual(comprehensive.constantFlux->flux, 0.0);
            }

            template<typename T>
            bool operator()(const T &) const
            {
                return false;
            }
        };

        struct ComprehensiveConvectionVisitor
        {
            bool operator()(const Comprehensive & comprehensive) const
            {
                return comprehensive.convection.has_value();
            }

            template<typename T>
            bool operator()(const T &) const
            {
                return false;
            }
        };
    }   // namespace

    bool isConstantFlux(const BoundaryCondition & bc)
    {
        return std::visit(ConstantFluxVisitor{}, bc.data);
    }

    bool isComprehensiveWithConvection(const BoundaryCondition & bc)
    {
        return std::visit(ComprehensiveConvectionVisitor{}, bc.data);
    }

    namespace
    {
        class RelativeHumidityVisitor
        {
        public:
            std::optional<double> operator()(const BCSteadyStateLibrary::Simplified & simplified) const
            {
                return simplified.relativeHumidity;
            }

            std::optional<double> operator()(const BCSteadyStateLibrary::Comprehensive & comprehensive) const
            {
                return comprehensive.relativeHumidity;
            }

            std::optional<double> operator()(const BCSteadyStateLibrary::RadiationSurface &) const
            {
                return std::nullopt;
            }
        };

    }   // end anonymous namespace

    std::optional<double> getRelativeHumidity(const BCSteadyStateLibrary::BoundaryCondition & bc)
    {
        return std::visit(RelativeHumidityVisitor{}, bc.data);
    }

    namespace
    {
        struct AirTemperatureVisitor
        {
            std::optional<double> operator()(const BCSteadyStateLibrary::Comprehensive & comprehensive) const
            {
                if(comprehensive.convection.has_value())
                {
                    return comprehensive.convection.value().temperature;
                }
                return std::nullopt;
            }

            std::optional<double> operator()(const BCSteadyStateLibrary::Simplified & simplified) const
            {
                return simplified.temperature;
            }

            std::optional<double> operator()(const BCSteadyStateLibrary::RadiationSurface &) const
            {
                return std::nullopt;
            }
        };
    }   // namespace

    std::optional<double> airTemperature(const BCSteadyStateLibrary::BoundaryCondition & bc)
    {
        return std::visit(AirTemperatureVisitor{}, bc.data);
    }

    namespace
    {
        class FilmCoefficientVisitor
        {
        public:
            std::optional<double> operator()(const BCSteadyStateLibrary::Simplified & simplified) const
            {
                return simplified.filmCoefficient;
            }

            std::optional<double> operator()(const BCSteadyStateLibrary::Comprehensive & comprehensive) const
            {
                if(comprehensive.convection.has_value())
                {
                    return comprehensive.convection.value().filmCoefficient;
                }
                return std::nullopt;
            }

            std::optional<double> operator()(const BCSteadyStateLibrary::RadiationSurface &) const
            {
                return std::nullopt;
            }
        };

    }   // end anonymous namespace

    std::optional<double> filmCoefficient(const BoundaryCondition& bc)
    {
        return std::visit(FilmCoefficientVisitor{}, bc.data);
    }

    namespace
    {
        class RadiationTemperatureVisitor
        {
        public:
            std::optional<double> operator()(const BCSteadyStateLibrary::LinearizedRadiation & radiation) const
            {
                return radiation.temperature;
            }

            std::optional<double> operator()(const BCSteadyStateLibrary::AutomaticEnclosure & radiation) const
            {
                return radiation.temperature;
            }

            std::optional<double> operator()(const BCSteadyStateLibrary::BlackBodyRadiation & radiation) const
            {
                return radiation.temperature;
            }

            template<typename T>
            std::optional<double> operator()(const T &) const
            {
                return std::nullopt;
            }
        };

        class BoundaryConditionRadiationTemperatureVisitor
        {
        public:
            std::optional<double> operator()(const BCSteadyStateLibrary::Comprehensive & comprehensive) const
            {
                if(comprehensive.radiation.has_value() && comprehensive.radiation->radiation.has_value())
                {
                    return std::visit(RadiationTemperatureVisitor{}, comprehensive.radiation->radiation.value());
                }
                return std::nullopt;
            }

            std::optional<double> operator()(const BCSteadyStateLibrary::RadiationSurface & radiation) const
            {
                return radiation.temperature;
            }

            template<typename T>
            std::optional<double> operator()(const T &) const
            {
                return std::nullopt;
            }
        };

    }   // end anonymous namespace

    std::optional<double> radiationTemperature(const BoundaryCondition& bc)
    {
        return std::visit(BoundaryConditionRadiationTemperatureVisitor{}, bc.data);
    }

    namespace
    {
        class ViewFactorVisitor
        {
        public:
            std::optional<double> operator()(const BCSteadyStateLibrary::BlackBodyRadiation & radiation) const
            {
                return radiation.emissivity;
            }

            template<typename T>
            std::optional<double> operator()(const T &) const
            {
                return std::nullopt;
            }
        };

        class BoundaryConditionViewFactorVisitor
        {
        public:
            std::optional<double> operator()(const BCSteadyStateLibrary::Comprehensive & comprehensive) const
            {
                if(comprehensive.radiation.has_value() && comprehensive.radiation->radiation.has_value())
                {
                    return std::visit(ViewFactorVisitor{}, comprehensive.radiation->radiation.value());
                }
                return std::nullopt;
            }

            template<typename T>
            std::optional<double> operator()(const T &) const
            {
                return std::nullopt;
            }
        };

    }

    std::optional<double> viewFactor(const BoundaryCondition& bc)
    {
        return std::visit(BoundaryConditionViewFactorVisitor{}, bc.data);
    }


    namespace
    {
        class RadiationFilmCoefficientVisitor
        {
        public:
            std::optional<double> operator()(const BCSteadyStateLibrary::LinearizedRadiation & radiation) const
            {
                return radiation.filmCoefficient;
            }

            template<typename T>
            std::optional<double> operator()(const T &) const
            {
                return std::nullopt;
            }
        };

        class BoundaryConditionRadiationFilmCoefficientVisitor
        {
        public:
            std::optional<double> operator()(const BCSteadyStateLibrary::Comprehensive & comprehensive) const
            {
                if(comprehensive.radiation.has_value() && comprehensive.radiation->radiation.has_value())
                {
                    return std::visit(RadiationFilmCoefficientVisitor{}, comprehensive.radiation->radiation.value());
                }
                return std::nullopt;
            }

            std::optional<double> operator()(const BCSteadyStateLibrary::Simplified & simplified) const
            {
                return simplified.filmCoefficient;
            }

            template<typename T>
            std::optional<double> operator()(const T &) const
            {
                return std::nullopt;
            }
        };

    }   // end anonymous namespace

    std::optional<double> radiationFilmCoefficient(const BoundaryCondition& bc)
    {
        return std::visit(BoundaryConditionRadiationFilmCoefficientVisitor{}, bc.data);
    }

    namespace
    {
        class EmissivityVisitor
        {
        public:
            std::optional<double> operator()(const BCSteadyStateLibrary::AutomaticEnclosure & radiation) const
            {
                return radiation.emissivity;
            }

            template<typename T>
            std::optional<double> operator()(const T &) const
            {
                return std::nullopt;
            }
        };

        class BoundaryConditionEmissivityVisitor
        {
        public:
            std::optional<double> operator()(const BCSteadyStateLibrary::Comprehensive & comprehensive) const
            {
                if(comprehensive.radiation.has_value() && comprehensive.radiation->radiation.has_value())
                {
                    return std::visit(EmissivityVisitor{}, comprehensive.radiation->radiation.value());
                }
                return std::nullopt;
            }

            std::optional<double> operator()(const BCSteadyStateLibrary::RadiationSurface & surface) const
            {
                return surface.emissivity;
            }

            template<typename T>
            std::optional<double> operator()(const T &) const
            {
                return std::nullopt;
            }
        };

    }   // end anonymous namespace

    std::optional<double> emissivity(const BoundaryCondition& bc)
    {
        return std::visit(BoundaryConditionEmissivityVisitor{}, bc.data);
    }

    namespace
    {
        class ConstantHeatFluxVisitor
        {
        public:
            std::optional<double> operator()(const BCSteadyStateLibrary::Comprehensive & comprehensive) const
            {
                if(comprehensive.constantFlux.has_value())
                {
                    return comprehensive.constantFlux.value().flux;
                }
                return std::nullopt;
            }

            template<typename T>
            std::optional<double> operator()(const T &) const
            {
                return std::nullopt;
            }
        };

    }   // end anonymous namespace

    std::optional<double> constantHeatFlux(const BoundaryCondition& bc)
    {
        return std::visit(ConstantHeatFluxVisitor{}, bc.data);
    }

    namespace
    {
        struct BoundaryConditionTypeVisitor
        {
            BoundaryConditionType operator()(const Comprehensive &)
            {
                return BoundaryConditionType::Comprehensive;
            }

            BoundaryConditionType operator()(const Simplified &)
            {
                return BoundaryConditionType::Simplified;
            }

            BoundaryConditionType operator()(const RadiationSurface &)
            {
                return BoundaryConditionType::RadiationSurface;
            }
        };
    }   // namespace

    BoundaryConditionType getBoundaryConditionType(const BoundaryCondition & bc)
    {
        return std::visit(BoundaryConditionTypeVisitor{}, bc.data);
    }


}   // namespace BCSteadyState