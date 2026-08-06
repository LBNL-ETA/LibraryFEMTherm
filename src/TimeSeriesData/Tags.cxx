#include "Tags.hxx"

#include "LibraryUtilities/Common.hxx"

namespace TimeSeriesLibrary
{
    namespace
    {
        const std::map<ChannelRole, std::string> & channelRoleMap()
        {
            static const std::map<ChannelRole, std::string> roles{
              {ChannelRole::AirTemperature, "AirTemperature"},
              {ChannelRole::RelativeHumidity, "RelativeHumidity"},
              {ChannelRole::WindSpeed, "WindSpeed"},
              {ChannelRole::WindDirection, "WindDirection"},
              {ChannelRole::SolarIrradiance, "SolarIrradiance"},
              {ChannelRole::HeatFlux, "HeatFlux"},
              {ChannelRole::ConvectiveCoefficient, "ConvectiveCoefficient"},
              {ChannelRole::RadiantTemperature, "RadiantTemperature"},
              {ChannelRole::RadiativeCoefficient, "RadiativeCoefficient"},
              {ChannelRole::Emissivity, "Emissivity"},
              {ChannelRole::PrescribedTemperature, "PrescribedTemperature"},
              {ChannelRole::PrescribedHumidity, "PrescribedHumidity"}};
            return roles;
        }
    }   // namespace

    std::string Tags::timeSeriesData() const
    {
        return timeSeriesData_;
    }

    std::string Tags::timeSeries() const
    {
        return timeSeries_;
    }

    std::string Tags::version() const
    {
        return version_;
    }

    std::string Tags::uuid() const
    {
        return uuid_;
    }

    std::string Tags::name() const
    {
        return name_;
    }

    std::string Tags::projectName() const
    {
        return projectName_;
    }

    std::string Tags::protectedTag() const
    {
        return protectedTag_;
    }

    std::string Tags::description() const
    {
        return description_;
    }

    std::string Tags::channel() const
    {
        return channel_;
    }

    std::string Tags::role() const
    {
        return role_;
    }

    std::string Tags::value() const
    {
        return value_;
    }

    std::string channelRoleToString(ChannelRole role)
    {
        return LibraryCommon::enumToString(role, channelRoleMap());
    }

    ChannelRole channelRoleFromString(std::string_view value)
    {
        return LibraryCommon::enumFromString(value, channelRoleMap());
    }

    std::vector<std::string> channelRoleStrings()
    {
        return LibraryCommon::stringVector(channelRoleMap());
    }
}   // namespace TimeSeriesLibrary
