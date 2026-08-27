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
        [[nodiscard]] std::string channel() const;
        [[nodiscard]] std::string role() const;
        [[nodiscard]] std::string value() const;

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
        const std::string channel_{"Channel"};
        const std::string role_{"Role"};
        const std::string value_{"Value"};
    };

    [[nodiscard]] std::string channelRoleToString(ChannelRole role);
    [[nodiscard]] ChannelRole channelRoleFromString(std::string_view value);
    [[nodiscard]] std::vector<std::string> channelRoleStrings();
}   // namespace TimeSeriesLibrary
