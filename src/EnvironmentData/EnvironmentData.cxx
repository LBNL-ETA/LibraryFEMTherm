#include <algorithm>
#include <ranges>

#include <lbnl/algorithm.hxx>

#include "EnvironmentData.hxx"

namespace EnvironmentDataLibrary
{
    size_t steps(const EnvironmentData & data)
    {
        if(data.channels.empty())
        {
            return 0U;
        }

        const auto minimum{std::ranges::min_element(
          data.channels, [](const Channel & lhs, const Channel & rhs) {
              return lhs.values.size() < rhs.values.size();
          })};

        return minimum->values.size();
    }

    bool hasRole(const EnvironmentData & data, ChannelRole role)
    {
        return std::ranges::any_of(data.channels,
                                   [role](const Channel & channel) { return channel.role == role; });
    }

    std::optional<std::vector<double>> valuesForRole(const EnvironmentData & data, ChannelRole role)
    {
        const auto found{std::ranges::find_if(
          data.channels, [role](const Channel & channel) { return channel.role == role; })};

        if(found == data.channels.end())
        {
            return std::nullopt;
        }

        return found->values;
    }

    std::vector<ChannelRole> providedRoles(const EnvironmentData & data)
    {
        return lbnl::transform_to_vector(data.channels,
                                         [](const Channel & channel) { return channel.role; });
    }
}   // namespace EnvironmentDataLibrary
