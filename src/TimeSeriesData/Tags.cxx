#include <algorithm>
#include <format>
#include <stdexcept>

#include "Tags.hxx"

#include "LibraryUtilities/Common.hxx"

namespace TimeSeriesLibrary
{
    namespace
    {
        const std::map<SeriesRole, std::string> & seriesRoleMap()
        {
            static const std::map<SeriesRole, std::string> roles{
              {SeriesRole::AirTemperature, "Air Temperature"},
              {SeriesRole::RelativeHumidity, "Relative Humidity"},
              {SeriesRole::WindSpeed, "Wind Speed"},
              {SeriesRole::WindDirection, "Wind Direction"},
              {SeriesRole::SolarIrradiance, "Solar Irradiance"},
              {SeriesRole::HeatFlux, "Heat Flux"},
              {SeriesRole::ConvectiveCoefficient, "Convective Coefficient"},
              {SeriesRole::RadiantTemperature, "Radiant Temperature"},
              {SeriesRole::RadiativeCoefficient, "Radiative Coefficient"},
              {SeriesRole::Emissivity, "Emissivity"},
              {SeriesRole::SurfaceTemperature, "Surface Temperature"},
              {SeriesRole::SurfaceHumidity, "Surface Humidity"}};
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

    std::string Tags::timeAxis() const
    {
        return timeAxis_;
    }

    std::string Tags::month() const
    {
        return month_;
    }

    std::string Tags::day() const
    {
        return day_;
    }

    std::string Tags::hour() const
    {
        return hour_;
    }

    std::string Tags::minute() const
    {
        return minute_;
    }

    std::string Tags::stepSeconds() const
    {
        return stepSeconds_;
    }

    std::string Tags::stepLengths() const
    {
        return stepLengths_;
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
        const auto & roles{seriesRoleMap()};
        const auto found{std::ranges::find_if(roles, [value](const auto & entry) { return entry.second == value; })};
        if(found == roles.end())
        {
            throw std::invalid_argument(std::format("'{}' is not a time series role", value));
        }
        return found->first;
    }

    std::vector<std::string> seriesRoleStrings()
    {
        return LibraryCommon::stringVector(seriesRoleMap());
    }
}   // namespace TimeSeriesLibrary
