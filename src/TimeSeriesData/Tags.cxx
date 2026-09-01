#include "Tags.hxx"

#include "LibraryUtilities/Common.hxx"

namespace TimeSeriesLibrary
{
    namespace
    {
        const std::map<SeriesRole, std::string> & seriesRoleMap()
        {
            static const std::map<SeriesRole, std::string> roles{
              {SeriesRole::AirTemperature, "AirTemperature"},
              {SeriesRole::RelativeHumidity, "RelativeHumidity"},
              {SeriesRole::WindSpeed, "WindSpeed"},
              {SeriesRole::WindDirection, "WindDirection"},
              {SeriesRole::SolarIrradiance, "SolarIrradiance"},
              {SeriesRole::HeatFlux, "HeatFlux"},
              {SeriesRole::ConvectiveCoefficient, "ConvectiveCoefficient"},
              {SeriesRole::RadiantTemperature, "RadiantTemperature"},
              {SeriesRole::RadiativeCoefficient, "RadiativeCoefficient"},
              {SeriesRole::Emissivity, "Emissivity"},
              {SeriesRole::PrescribedTemperature, "PrescribedTemperature"},
              {SeriesRole::PrescribedHumidity, "PrescribedHumidity"}};
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

    std::string Tags::color() const
    {
        return color_;
    }

    std::string Tags::source() const
    {
        return source_;
    }

    std::string Tags::series() const
    {
        return series_;
    }

    std::string Tags::role() const
    {
        return role_;
    }

    std::string Tags::values() const
    {
        return values_;
    }

    std::string seriesRoleToString(SeriesRole role)
    {
        return LibraryCommon::enumToString(role, seriesRoleMap());
    }

    SeriesRole seriesRoleFromString(std::string_view value)
    {
        return LibraryCommon::enumFromString(value, seriesRoleMap());
    }

    std::vector<std::string> seriesRoleStrings()
    {
        return LibraryCommon::stringVector(seriesRoleMap());
    }
}   // namespace TimeSeriesLibrary
