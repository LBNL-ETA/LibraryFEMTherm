#include <algorithm>
#include <ranges>

#include <lbnl/algorithm.hxx>

#include "TimeSeriesData.hxx"

namespace TimeSeriesLibrary
{
    size_t steps(const TimeSeriesData & data)
    {
        if(data.series.empty())
        {
            return 0U;
        }

        const auto minimum{std::ranges::min_element(
          data.series, [](const Series & lhs, const Series & rhs) {
              return lhs.values.size() < rhs.values.size();
          })};

        return minimum->values.size();
    }

    bool hasRole(const TimeSeriesData & data, SeriesRole role)
    {
        return std::ranges::any_of(data.series,
                                   [role](const Series & series) { return series.role == role; });
    }

    std::optional<std::vector<double>> valuesForRole(const TimeSeriesData & data, SeriesRole role)
    {
        const auto found{std::ranges::find_if(
          data.series, [role](const Series & series) { return series.role == role; })};

        if(found == data.series.end())
        {
            return std::nullopt;
        }

        return found->values;
    }

    std::vector<SeriesRole> providedRoles(const TimeSeriesData & data)
    {
        return lbnl::transform_to_vector(data.series,
                                         [](const Series & series) { return series.role; });
    }
}   // namespace TimeSeriesLibrary
