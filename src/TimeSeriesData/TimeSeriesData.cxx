#include <algorithm>
#include <array>
#include <cmath>
#include <format>
#include <ranges>

#include <lbnl/algorithm.hxx>

#include "TimeSeriesData.hxx"

namespace TimeSeriesLibrary
{
    namespace
    {
        constexpr std::array<size_t, 12> daysBeforeMonth{
          0U, 31U, 59U, 90U, 120U, 151U, 181U, 212U, 243U, 273U, 304U, 334U};

        constexpr std::array<const char *, 12> monthNames{"Jan",
                                                          "Feb",
                                                          "Mar",
                                                          "Apr",
                                                          "May",
                                                          "Jun",
                                                          "Jul",
                                                          "Aug",
                                                          "Sep",
                                                          "Oct",
                                                          "Nov",
                                                          "Dec"};

        size_t monthIndex(const size_t month)
        {
            return std::clamp<size_t>(month, 1U, 12U) - 1U;
        }

        bool divides(const double step, const double unit)
        {
            return std::abs(std::remainder(step, unit)) < 1e-6;
        }
    }   // namespace

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

    double secondsIntoYear(
      const size_t month, const size_t day, const size_t hour, const size_t minute, const size_t second)
    {
        const auto days{daysBeforeMonth[monthIndex(month)] + (std::max<size_t>(day, 1U) - 1U)};
        return static_cast<double>(days) * 86400.0 + static_cast<double>(hour) * 3600.0
               + static_cast<double>(minute) * 60.0 + static_cast<double>(second);
    }

    double startSecondsIntoYear(const TimeAxis & axis)
    {
        return secondsIntoYear(axis.month, axis.day, axis.hour, axis.minute);
    }

    std::string startLabel(const TimeAxis & axis)
    {
        return std::format(
          "{} {} {:02}:{:02}", axis.day, monthNames[monthIndex(axis.month)], axis.hour, axis.minute);
    }

    std::string stepLabel(const TimeAxis & axis)
    {
        if(divides(axis.stepSeconds, 3600.0))
        {
            return std::format("{:g} h", axis.stepSeconds / 3600.0);
        }
        if(divides(axis.stepSeconds, 60.0))
        {
            return std::format("{:g} min", axis.stepSeconds / 60.0);
        }
        return std::format("{:g} s", axis.stepSeconds);
    }

    std::string describeAxis(const TimeSeriesData & data)
    {
        return std::format(
          "{}, step {}, {} rows", startLabel(data.axis), stepLabel(data.axis), steps(data));
    }

    bool aligned(const TimeSeriesData & lhs, const TimeSeriesData & rhs)
    {
        return lhs.axis == rhs.axis && steps(lhs) == steps(rhs);
    }

    std::vector<std::string> alignmentIssues(const std::vector<TimeSeriesData> & datasets)
    {
        if(datasets.empty())
        {
            return {};
        }
        const bool allAligned{std::ranges::all_of(
          datasets, [&datasets](const TimeSeriesData & data) { return aligned(datasets.front(), data); })};
        if(allAligned)
        {
            return {};
        }
        return lbnl::transform_to_vector(datasets, [](const TimeSeriesData & data) {
            return data.Name + ": " + describeAxis(data);
        });
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
