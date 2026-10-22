#include <format>

#include <lbnl/algorithm.hxx>

#include "ContentHash.hxx"
#include "Window.hxx"

namespace TimeSeriesLibrary
{
    namespace
    {
        Series rows(const Series & series, const std::size_t firstStep, const std::size_t count)
        {
            const auto first{series.values.begin() + static_cast<std::ptrdiff_t>(firstStep)};
            return Series{.role = series.role, .values = std::vector<double>(first, first + static_cast<std::ptrdiff_t>(count))};
        }
    }   // namespace

    lbnl::ExpectedExt<TimeSeriesData, std::string>
      window(const TimeSeriesData & data, const std::size_t firstStep, const std::size_t count)
    {
        if(count == 0U)
        {
            return lbnl::Unexpected<std::string>{"a window needs at least one row"};
        }
        if(firstStep + count > steps(data))
        {
            return lbnl::Unexpected<std::string>{std::format(
              "rows {} to {} reach past the {} rows of '{}'", firstStep, firstStep + count, steps(data), data.Name)};
        }
        auto result{data};
        result.axis = axisAfter(data.axis, firstStep);
        if(!evenlyStepped(result.axis))
        {
            result.axis.stepLengths = stepLengths(result.axis, count);
        }
        result.series = lbnl::transform_to_vector(
          data.series, [firstStep, count](const Series & series) { return rows(series, firstStep, count); });
        result.UUID = contentUuid(result);
        return result;
    }
}   // namespace TimeSeriesLibrary
