#include <algorithm>
#include <array>
#include <cmath>
#include <format>
#include <numeric>
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

        //! Days in `month`, 1..12, of the nominal year.
        size_t daysInMonth(const size_t month)
        {
            constexpr size_t daysPerYear{365U};
            const size_t nextStart{month < daysBeforeMonth.size() ? daysBeforeMonth[month] : daysPerYear};
            return nextStart - daysBeforeMonth[month - 1U];
        }

        std::string dayOutsideMonth(const size_t month, const size_t day)
        {
            const auto days{daysInMonth(month)};
            return std::format("day {} is not 1..{}: days count from 1, and {} has {} days in the nominal year, "
                               "which has no leap day",
                               day,
                               days,
                               monthNames[month - 1U],
                               days);
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
      const size_t month, const size_t day, const size_t clockHour, const size_t minute, const size_t second)
    {
        const auto days{daysBeforeMonth[monthIndex(month)] + (std::max<size_t>(day, 1U) - 1U)};
        return static_cast<double>(days) * 86400.0 + static_cast<double>(clockHour) * 3600.0
               + static_cast<double>(minute) * 60.0 + static_cast<double>(second);
    }

    size_t clockHour(const TimeAxis & axis)
    {
        return std::clamp<size_t>(axis.hour, 1U, 24U) - 1U;
    }

    double startSecondsIntoYear(const TimeAxis & axis)
    {
        return secondsIntoYear(axis.month, axis.day, clockHour(axis), axis.minute);
    }

    std::optional<std::string>
      calendarPositionError(const size_t month, const size_t day, const size_t hour, const size_t minute)
    {
        if(month < 1U || month > 12U)
        {
            return std::format("month {} is not 1..12: months count from 1, as a calendar and an EnergyPlus "
                               "weather file count them",
                               month);
        }
        if(day < 1U || day > daysInMonth(month))
        {
            return dayOutsideMonth(month, day);
        }
        if(hour < 1U || hour > 24U)
        {
            return std::format("hour {} is not 1..24: hours count from 1, as an EnergyPlus weather file "
                               "counts them, hour 1 being 00:00 to 01:00",
                               hour);
        }
        if(minute > 59U)
        {
            return std::format("minute {} is not 0..59", minute);
        }
        return std::nullopt;
    }

    TimeAxis axisAt(const double secondsIntoYear)
    {
        constexpr double minutesPerDay{24.0 * 60.0};
        double wrapped{std::fmod(secondsIntoYear, secondsPerNominalYear())};
        if(wrapped < 0.0)
        {
            wrapped += secondsPerNominalYear();
        }
        const auto minutes{static_cast<size_t>(std::llround(wrapped / 60.0))};
        const auto days{static_cast<size_t>(static_cast<double>(minutes) / minutesPerDay)};
        // The first month whose start lies past the day is the one after the day's month.
        const auto monthAfter{std::ranges::upper_bound(daysBeforeMonth, days)};
        const auto month{static_cast<size_t>(monthAfter - daysBeforeMonth.begin())};
        return TimeAxis{.month = month,
                        .day = days - daysBeforeMonth[month - 1U] + 1U,
                        .hour = (minutes / 60U) % 24U + 1U,
                        .minute = minutes % 60U};
    }

    bool evenlyStepped(const TimeAxis & axis)
    {
        return std::holds_alternative<double>(axis.stepSeconds);
    }

    std::vector<double> stepLengths(const TimeAxis & axis, const size_t steps)
    {
        if(const auto * every{std::get_if<double>(&axis.stepSeconds)})
        {
            return std::vector<double>(steps, *every);
        }
        auto lengths{std::get<std::vector<double>>(axis.stepSeconds)};
        lengths.resize(steps, lengths.back());
        return lengths;
    }

    double stepSecondsAt(const TimeAxis & axis, const size_t index)
    {
        if(const auto * every{std::get_if<double>(&axis.stepSeconds)})
        {
            return *every;
        }
        const auto & lengths{std::get<std::vector<double>>(axis.stepSeconds)};
        return lengths[std::min(index, lengths.size() - 1U)];
    }

    StepSeconds stepSecondsOf(const std::vector<double> & lengths)
    {
        constexpr double sameWithin{0.5};
        const bool even{std::ranges::all_of(
          lengths, [&lengths](const double length) { return std::abs(length - lengths.front()) <= sameWithin; })};
        if(even)
        {
            return lengths.empty() ? StepSeconds{3600.0} : StepSeconds{lengths.front()};
        }
        return lengths;
    }

    TimeAxis axisAfter(const TimeAxis & axis, const size_t steps)
    {
        const auto lengths{stepLengths(axis, steps)};
        const double elapsed{std::accumulate(lengths.begin(), lengths.end(), 0.0)};
        auto moved{axisAt(startSecondsIntoYear(axis) + elapsed)};
        moved.stepSeconds = axis.stepSeconds;
        if(const auto * stated{std::get_if<std::vector<double>>(&axis.stepSeconds)}; stated != nullptr && steps < stated->size())
        {
            moved.stepSeconds = std::vector<double>(stated->begin() + static_cast<std::ptrdiff_t>(steps), stated->end());
        }
        return moved;
    }

    std::string startLabel(const TimeAxis & axis)
    {
        return std::format(
          "{} {} {:02}:{:02}", axis.day, monthNames[monthIndex(axis.month)], clockHour(axis), axis.minute);
    }

    namespace
    {
        std::string lengthLabel(const double seconds)
        {
            if(divides(seconds, 3600.0))
            {
                return std::format("{:g} h", seconds / 3600.0);
            }
            if(divides(seconds, 60.0))
            {
                return std::format("{:g} min", seconds / 60.0);
            }
            return std::format("{:g} s", seconds);
        }
    }   // namespace

    std::string stepLabel(const TimeAxis & axis)
    {
        if(const auto * every{std::get_if<double>(&axis.stepSeconds)})
        {
            return lengthLabel(*every);
        }
        const auto [shortest, longest]{std::ranges::minmax_element(std::get<std::vector<double>>(axis.stepSeconds))};
        if(*shortest == *longest)
        {
            return lengthLabel(*shortest);
        }
        return std::format("{} to {}", lengthLabel(*shortest), lengthLabel(*longest));
    }

    std::string describeAxis(const TimeSeriesData & data)
    {
        return std::format("{}, {} {}, {} rows",
                           startLabel(data.axis),
                           evenlyStepped(data.axis) ? "step" : "steps",
                           stepLabel(data.axis),
                           steps(data));
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

    namespace
    {
        size_t longestSeries(const TimeSeriesData & data)
        {
            const auto counts{
              lbnl::transform_to_vector(data.series, [](const Series & series) { return series.values.size(); })};
            return counts.empty() ? 0U : std::ranges::max(counts);
        }

        std::optional<DatasetWarning> seriesLengthWarning(const TimeSeriesData & data)
        {
            const auto longest{longestSeries(data)};
            if(longest == steps(data))
            {
                return std::nullopt;
            }
            return DatasetWarning{
              .kind = DatasetWarningKind::SeriesLengthsDiffer, .steps = steps(data), .stated = longest};
        }

        std::optional<DatasetWarning> stepLengthWarning(const TimeSeriesData & data)
        {
            const auto * lengths{std::get_if<std::vector<double>>(&data.axis.stepSeconds)};
            if(lengths == nullptr || lengths->size() == steps(data))
            {
                return std::nullopt;
            }
            return DatasetWarning{
              .kind = DatasetWarningKind::StepLengthsDiffer, .steps = steps(data), .stated = lengths->size()};
        }

        std::optional<DatasetWarning> uuidWarning(const TimeSeriesData & data)
        {
            return data.UUID.empty() ? std::optional{DatasetWarning{.kind = DatasetWarningKind::NoUuid}}
                                     : std::nullopt;
        }

        std::string stepLengthsText(const DatasetWarning & warning)
        {
            return std::format("the axis lists {} step lengths for {} steps: {}",
                               warning.stated,
                               warning.steps,
                               warning.stated < warning.steps ? "the last length is repeated for the rest"
                                                              : "the lengths past the last step are not read");
        }
    }   // namespace

    std::vector<DatasetWarning> datasetWarnings(const TimeSeriesData & data)
    {
        const std::array candidates{seriesLengthWarning(data), stepLengthWarning(data), uuidWarning(data)};
        return lbnl::transform_filter(
          candidates,
          [](const std::optional<DatasetWarning> & candidate) { return candidate.has_value(); },
          [](const std::optional<DatasetWarning> & candidate) { return candidate.value(); });
    }

    std::string describeWarning(const TimeSeriesData & data, const DatasetWarning & warning)
    {
        switch(warning.kind)
        {
            case DatasetWarningKind::SeriesLengthsDiffer:
                return std::format("{}: the series have from {} to {} values; a run takes {} steps, as many as "
                                   "the shortest series has, and leaves the rest unread",
                                   data.Name,
                                   warning.steps,
                                   warning.stated,
                                   warning.steps);
            case DatasetWarningKind::StepLengthsDiffer:
                return std::format("{}: {}", data.Name, stepLengthsText(warning));
            case DatasetWarningKind::NoUuid:
                return std::format("{}: no UUID; two datasets without one, written into one archive, become one. "
                                   "Its content UUID gives it one of its own",
                                   data.Name);
        }
        return {};
    }
}   // namespace TimeSeriesLibrary
