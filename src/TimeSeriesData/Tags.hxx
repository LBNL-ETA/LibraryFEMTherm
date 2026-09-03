#pragma once

#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "TimeSeriesData.hxx"

namespace TimeSeriesLibrary
{
    class Tags
    {
    public:
        [[nodiscard]] std::string timeSeriesData() const;
        [[nodiscard]] std::string timeSeries() const;
        [[nodiscard]] std::string version() const;
        [[nodiscard]] std::string uuid() const;
        [[nodiscard]] std::string name() const;
        [[nodiscard]] std::string projectName() const;
        [[nodiscard]] std::string protectedTag() const;
        [[nodiscard]] std::string description() const;
        [[nodiscard]] std::string color() const;
        [[nodiscard]] std::string source() const;
        [[nodiscard]] std::string timeAxis() const;
        [[nodiscard]] std::string month() const;
        [[nodiscard]] std::string day() const;
        [[nodiscard]] std::string hour() const;
        [[nodiscard]] std::string minute() const;
        [[nodiscard]] std::string stepSeconds() const;
        [[nodiscard]] std::string series() const;
        [[nodiscard]] std::string role() const;
        [[nodiscard]] std::string values() const;

    private:
        const std::string timeSeriesData_{"TimeSeriesData"};
        const std::string timeSeries_{"TimeSeries"};
        const std::string version_{"Version"};
        const std::string uuid_{"UUID"};
        const std::string name_{"Name"};
        const std::string projectName_{"ProjectName"};
        const std::string protectedTag_{"Protected"};
        const std::string description_{"Description"};
        const std::string color_{"Color"};
        const std::string source_{"Source"};
        const std::string timeAxis_{"TimeAxis"};
        const std::string month_{"Month"};
        const std::string day_{"Day"};
        const std::string hour_{"Hour"};
        const std::string minute_{"Minute"};
        const std::string stepSeconds_{"StepSeconds"};
        const std::string series_{"Series"};
        const std::string role_{"Role"};
        const std::string values_{"Values"};
    };

    [[nodiscard]] std::string seriesRoleToString(SeriesRole role);
    [[nodiscard]] SeriesRole seriesRoleFromString(std::string_view value);
    [[nodiscard]] std::vector<std::string> seriesRoleStrings();
}   // namespace TimeSeriesLibrary
