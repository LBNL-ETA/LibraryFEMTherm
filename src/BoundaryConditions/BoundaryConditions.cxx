#include <algorithm>

#include <lbnl/algorithm.hxx>

#include "BoundaryConditions.hxx"

namespace BCLibrary
{
    bool isConstant(const Source & source)
    {
        return std::holds_alternative<Constant>(source);
    }

    bool isFromTimeSeries(const Source & source)
    {
        return std::holds_alternative<FromTimeSeries>(source);
    }

    namespace
    {
        using TimeSeriesLibrary::SeriesRole;

        void appendRole(std::vector<SeriesRole> & roles, const Source & source)
        {
            if(const auto * environment = std::get_if<FromTimeSeries>(&source))
            {
                if(!lbnl::contains(roles, environment->role))
                {
                    roles.push_back(environment->role);
                }
            }
        }

        void appendRole(std::vector<SeriesRole> & roles, const std::optional<Source> & source)
        {
            if(source.has_value())
            {
                appendRole(roles, source.value());
            }
        }

        void appendConvectionRoles(std::vector<SeriesRole> & roles, const std::optional<Convection> & convection)
        {
            if(!convection.has_value())
            {
                return;
            }

            appendRole(roles, convection->airTemperature);
            appendRole(roles, convection->filmCoefficient);
            appendRole(roles, convection->windSpeed);
            appendRole(roles, convection->windDirection);
        }

        void appendRadiationRoles(std::vector<SeriesRole> & roles,
                                  const std::optional<RadiationOptions> & radiation)
        {
            if(!radiation.has_value())
            {
                return;
            }

            std::visit(
              [&roles](const auto & option) {
                  using OptionType = std::decay_t<decltype(option)>;
                  if constexpr(std::is_same_v<OptionType, AutomaticEnclosure>)
                  {
                      appendRole(roles, option.emissivity);
                  }
                  else if constexpr(std::is_same_v<OptionType, BlackBodyRadiation>)
                  {
                      appendRole(roles, option.temperature);
                      appendRole(roles, option.emissivity);
                  }
                  else if constexpr(std::is_same_v<OptionType, FixedCoefficientRadiation>)
                  {
                      appendRole(roles, option.temperature);
                      appendRole(roles, option.coefficient);
                  }
              },
              radiation.value());
        }

        struct RequiredRolesVisitor
        {
            std::vector<SeriesRole> operator()(const SurfaceExchange & exchange) const
            {
                std::vector<SeriesRole> roles;
                appendRole(roles, exchange.relativeHumidity);
                appendConvectionRoles(roles, exchange.convection);
                appendRadiationRoles(roles, exchange.radiation);
                if(exchange.solar.has_value())
                {
                    appendRole(roles, exchange.solar->irradiance);
                }
                appendRole(roles, exchange.flux);
                return roles;
            }

            std::vector<SeriesRole> operator()(const PrescribedState & prescribed) const
            {
                std::vector<SeriesRole> roles;
                appendRole(roles, prescribed.temperature);
                appendRole(roles, prescribed.relativeHumidity);
                return roles;
            }

            std::vector<SeriesRole> operator()(const RadiationSurface &) const
            {
                return {};
            }

            std::vector<SeriesRole> operator()(const NoExchange &) const
            {
                return {};
            }
        };
    }   // namespace

    std::vector<TimeSeriesLibrary::SeriesRole> requiredRoles(const BoundaryCondition & boundaryCondition)
    {
        return std::visit(RequiredRolesVisitor{}, boundaryCondition.data);
    }

    bool isSteadyCapable(const BoundaryCondition & boundaryCondition)
    {
        return requiredRoles(boundaryCondition).empty();
    }

    std::vector<TimeSeriesLibrary::SeriesRole>
      missingRoles(const BoundaryCondition & boundaryCondition,
                   const std::optional<TimeSeriesLibrary::TimeSeriesData> & dataset)
    {
        using TimeSeriesLibrary::SeriesRole;
        const auto required{requiredRoles(boundaryCondition)};
        if(!dataset.has_value())
        {
            return required;
        }
        const auto provided{TimeSeriesLibrary::providedRoles(dataset.value())};
        const auto coveredByAir{[&provided](const SeriesRole role) {
            return role == SeriesRole::RadiantTemperature && lbnl::contains(provided, SeriesRole::AirTemperature);
        }};
        return lbnl::filter(required, [&](const SeriesRole role) {
            return !lbnl::contains(provided, role) && !coveredByAir(role);
        });
    }
}   // namespace BCLibrary
