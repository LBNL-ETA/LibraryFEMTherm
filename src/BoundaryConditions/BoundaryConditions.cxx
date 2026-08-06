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
        using TimeSeriesLibrary::ChannelRole;

        void appendRole(std::vector<ChannelRole> & roles, const Source & source)
        {
            if(const auto * environment = std::get_if<FromTimeSeries>(&source))
            {
                if(!lbnl::contains(roles, environment->role))
                {
                    roles.push_back(environment->role);
                }
            }
        }

        void appendRole(std::vector<ChannelRole> & roles, const std::optional<Source> & source)
        {
            if(source.has_value())
            {
                appendRole(roles, source.value());
            }
        }

        void appendConvectionRoles(std::vector<ChannelRole> & roles, const std::optional<Convection> & convection)
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

        void appendRadiationRoles(std::vector<ChannelRole> & roles,
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
            std::vector<ChannelRole> operator()(const SurfaceExchange & exchange) const
            {
                std::vector<ChannelRole> roles;
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

            std::vector<ChannelRole> operator()(const PrescribedState & prescribed) const
            {
                std::vector<ChannelRole> roles;
                appendRole(roles, prescribed.temperature);
                appendRole(roles, prescribed.relativeHumidity);
                return roles;
            }

            std::vector<ChannelRole> operator()(const RadiationSurface &) const
            {
                return {};
            }
        };
    }   // namespace

    std::vector<TimeSeriesLibrary::ChannelRole> requiredRoles(const BoundaryCondition & boundaryCondition)
    {
        return std::visit(RequiredRolesVisitor{}, boundaryCondition.data);
    }

    bool isSteadyCapable(const BoundaryCondition & boundaryCondition)
    {
        return requiredRoles(boundaryCondition).empty();
    }
}   // namespace BCLibrary
