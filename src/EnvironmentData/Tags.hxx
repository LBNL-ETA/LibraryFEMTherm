#pragma once

#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "EnvironmentData.hxx"

namespace EnvironmentDataLibrary
{
    class Tags
    {
    public:
        [[nodiscard]] std::string environmentData() const;
        [[nodiscard]] std::string version() const;
        [[nodiscard]] std::string uuid() const;
        [[nodiscard]] std::string name() const;
        [[nodiscard]] std::string protectedTag() const;
        [[nodiscard]] std::string description() const;
        [[nodiscard]] std::string channel() const;
        [[nodiscard]] std::string role() const;
        [[nodiscard]] std::string value() const;

    private:
        const std::string environmentData_{"EnvironmentData"};
        const std::string version_{"Version"};
        const std::string uuid_{"UUID"};
        const std::string name_{"Name"};
        const std::string protectedTag_{"Protected"};
        const std::string description_{"Description"};
        const std::string channel_{"Channel"};
        const std::string role_{"Role"};
        const std::string value_{"Value"};
    };

    [[nodiscard]] std::string channelRoleToString(ChannelRole role);
    [[nodiscard]] ChannelRole channelRoleFromString(std::string_view value);
    [[nodiscard]] std::vector<std::string> channelRoleStrings();
}   // namespace EnvironmentDataLibrary
