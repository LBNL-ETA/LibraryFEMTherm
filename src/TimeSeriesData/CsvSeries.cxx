#include <algorithm>
#include <cctype>
#include <charconv>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <format>
#include <initializer_list>
#include <map>
#include <numeric>
#include <optional>
#include <ranges>
#include <sstream>
#include <tuple>

#include <lbnl/algorithm.hxx>

#include "CsvSeries.hxx"

#include "ContentHash.hxx"
#include "Tags.hxx"

#include "LibraryUtilities/FileManipulation.hxx"

namespace TimeSeriesLibrary::Csv
{
    namespace
    {
        std::string normalise(std::string_view text)
        {
            std::string result;
            for(const char letter : text)
            {
                if(!std::isspace(static_cast<unsigned char>(letter)))
                {
                    result.push_back(
                      static_cast<char>(std::tolower(static_cast<unsigned char>(letter))));
                }
            }
            return result;
        }

        //! Accepted spellings of the time column, matching the mockup reader
        //! ("date time" and "datetime" normalise to the same entry).
        bool isTimeHeader(std::string_view header)
        {
            static const std::vector<std::string> timeHeaders{
              "time", "date", "datetime", "date/time", "timestamp"};
            return lbnl::contains(timeHeaders, normalise(header));
        }

        //! Accepted spellings of the timestep column: one length in seconds per row.
        bool isTimestepHeader(std::string_view header)
        {
            static const std::vector<std::string> stepHeaders{"timestep", "step", "stepseconds", "dt"};
            return lbnl::contains(stepHeaders, normalise(header));
        }

        const std::map<std::string, SeriesRole> & rolesByNormalisedHeader()
        {
            static const auto roles{[]() {
                std::map<std::string, SeriesRole> result;
                for(const auto & name : seriesRoleStrings())
                {
                    const auto role{seriesRoleFromString(name)};
                    result[normalise(name)] = role;
                    result[normalise(displayHeaderForRole(role))] = role;
                }
                return result;
            }()};
            return roles;
        }

        std::optional<SeriesRole> roleForHeader(std::string_view header)
        {
            const auto & roles{rolesByNormalisedHeader()};
            const auto found{roles.find(normalise(header))};
            return found != roles.end() ? std::optional{found->second} : std::nullopt;
        }

        //! A spreadsheet's separator follows its locale; a European export is
        //! usually semicolon separated. Counted on the header line only: data rows
        //! of a semicolon-separated file are full of decimal commas, which would
        //! outvote the real separator.
        char sniffSeparator(std::string_view sample)
        {
            const auto headerLine{sample.substr(0, sample.find('\n'))};
            const auto count{[&headerLine](const char sep) {
                return std::ranges::count(headerLine, sep);
            }};
            char best{','};
            for(const char candidate : {';', '\t'})
            {
                if(count(candidate) > count(best))
                {
                    best = candidate;
                }
            }
            return best;
        }

        //! Minimal quoted-field split: double quotes group a field, "" inside a
        //! quoted field is a literal quote.
        std::vector<std::string> splitLine(std::string_view line, const char separator)
        {
            std::vector<std::string> cells{std::string{}};
            bool quoted{false};
            for(size_t index = 0; index < line.size(); ++index)
            {
                const char letter{line[index]};
                if(letter == '"')
                {
                    const bool escaped{quoted && index + 1 < line.size() && line[index + 1] == '"'};
                    if(escaped)
                    {
                        cells.back().push_back('"');
                        ++index;
                    }
                    else
                    {
                        quoted = !quoted;
                    }
                }
                else if(letter == separator && !quoted)
                {
                    cells.emplace_back();
                }
                else
                {
                    cells.back().push_back(letter);
                }
            }
            return cells;
        }

        std::string trimmed(std::string_view text)
        {
            const auto begin{text.find_first_not_of(" \t\r\n")};
            if(begin == std::string_view::npos)
            {
                return {};
            }
            const auto end{text.find_last_not_of(" \t\r\n")};
            return std::string{text.substr(begin, end - begin + 1)};
        }

        //! Locale-tolerant number parse: standard first, then decimal comma.
        std::optional<double> toDouble(std::string_view cell)
        {
            auto parse{[](const std::string & text) -> std::optional<double> {
                double value{};
                const auto [ptr, ec]{std::from_chars(text.data(), text.data() + text.size(), value)};
                const bool complete{ec == std::errc{} && ptr == text.data() + text.size()};
                return complete ? std::optional{value} : std::nullopt;
            }};

            const auto plain{trimmed(cell)};
            if(plain.empty())
            {
                return std::nullopt;
            }
            if(const auto value{parse(plain)}; value.has_value())
            {
                return value;
            }
            auto commas{plain};
            std::ranges::replace(commas, ',', '.');
            return parse(commas);
        }

        //! Carry the last good reading across a gap: a blank cell mid-run is not a
        //! zero. A leading gap takes the first good reading (or zero if none).
        std::vector<double> fillGaps(const std::vector<std::optional<double>> & values)
        {
            const auto firstGood{lbnl::find_element(
              values, [](const std::optional<double> & value) { return value.has_value(); })};
            double previous{firstGood.has_value() ? firstGood->value() : 0.0};

            std::vector<double> filled;
            filled.reserve(values.size());
            for(const auto & value : values)
            {
                previous = value.value_or(previous);
                filled.push_back(previous);
            }
            return filled;
        }

        std::vector<std::vector<std::string>> parseRows(const std::string & content)
        {
            const auto separator{sniffSeparator(std::string_view{content}.substr(
              0, std::min<size_t>(content.size(), 4096U)))};

            std::vector<std::vector<std::string>> rows;
            std::istringstream stream{content};
            std::string line;
            while(std::getline(stream, line))
            {
                auto cells{splitLine(line, separator)};
                const bool blank{std::ranges::all_of(
                  cells, [](const std::string & cell) { return trimmed(cell).empty(); })};
                if(!blank)
                {
                    rows.emplace_back(std::move(cells));
                }
            }
            return rows;
        }

        std::string cellAt(const std::vector<std::string> & row, const size_t index)
        {
            return index < row.size() ? row[index] : std::string{};
        }

        void storeSeries(TimeSeriesData & data, const SeriesRole role, std::vector<double> values)
        {
            for(auto & series : data.series)
            {
                if(series.role == role)
                {
                    series.values = std::move(values);
                    return;
                }
            }
            data.series.push_back(Series{.role = role, .values = std::move(values)});
        }

        //! Column `index` from data row `firstDataRow` (1-based, the header being row 0) on.
        std::vector<std::optional<double>> readColumn(const std::vector<std::vector<std::string>> & rows,
                                                      const size_t index,
                                                      const size_t firstDataRow)
        {
            std::vector<std::optional<double>> column;
            column.reserve(rows.size() - firstDataRow);
            std::transform(rows.begin() + static_cast<std::ptrdiff_t>(firstDataRow),
                           rows.end(),
                           std::back_inserter(column),
                           [index](const auto & row) { return toDouble(cellAt(row, index)); });
            return column;
        }

        ///////////////////////////////////////////////////////////////////////////////////
        // Time column -> TimeAxis
        ///////////////////////////////////////////////////////////////////////////////////

        //! A calendar position as read from a time cell. The year is parsed and dropped.
        struct Moment
        {
            size_t month{1U};
            size_t day{1U};
            size_t hour{0U};
            size_t minute{0U};
            size_t second{0U};
        };

        std::optional<std::vector<size_t>> parseNumbers(std::string_view text, const char separator)
        {
            std::vector<size_t> numbers;
            for(const auto & piece : splitLine(text, separator))
            {
                const auto plain{trimmed(piece)};
                size_t value{};
                const auto [ptr, ec]{std::from_chars(plain.data(), plain.data() + plain.size(), value)};
                if(ec != std::errc{} || ptr != plain.data() + plain.size())
                {
                    return std::nullopt;
                }
                numbers.push_back(value);
            }
            return numbers;
        }

        //! Date order follows the separator: 2026-01-31 is year first (ISO), 1/31/2026 is
        //! month first (US), 31.1.2026 is day first (European). The year is dropped either way.
        std::optional<Moment> parseDate(std::string_view text)
        {
            using Layout = std::tuple<char, size_t, size_t>;
            for(const auto & [separator, monthAt, dayAt] :
                std::initializer_list<Layout>{{'-', 1U, 2U}, {'/', 0U, 1U}, {'.', 1U, 0U}})
            {
                if(text.find(separator) == std::string_view::npos)
                {
                    continue;
                }
                const auto numbers{parseNumbers(text, separator)};
                if(!numbers.has_value() || numbers->size() != 3U)
                {
                    return std::nullopt;
                }
                const Moment moment{.month = (*numbers)[monthAt], .day = (*numbers)[dayAt]};
                const bool valid{moment.month >= 1U && moment.month <= 12U && moment.day >= 1U
                                 && moment.day <= 31U};
                return valid ? std::optional{moment} : std::nullopt;
            }
            return std::nullopt;
        }

        std::optional<Moment> withClock(const Moment & date, std::string_view text)
        {
            const auto numbers{parseNumbers(text, ':')};
            if(!numbers.has_value() || numbers->size() < 2U || numbers->size() > 3U)
            {
                return std::nullopt;
            }
            Moment moment{date};
            moment.hour = (*numbers)[0];
            moment.minute = (*numbers)[1];
            moment.second = numbers->size() == 3U ? (*numbers)[2] : 0U;
            const bool valid{moment.hour <= 23U && moment.minute <= 59U && moment.second <= 59U};
            return valid ? std::optional{moment} : std::nullopt;
        }

        //! "2026-01-31 06:30", "2026-01-31T06:30:00", "1/31/2026 6:30", or a bare date.
        std::optional<Moment> parseMoment(std::string_view cell)
        {
            const auto plain{trimmed(cell)};
            const std::string_view view{plain};
            const auto split{view.find_first_of(" T")};
            const auto date{parseDate(view.substr(0, split))};
            if(!date.has_value() || split == std::string_view::npos)
            {
                return date;
            }
            return withClock(date.value(), view.substr(split + 1));
        }

        lbnl::ExpectedExt<std::vector<Moment>, std::string>
          readMoments(const std::vector<std::vector<std::string>> & rows,
                      const size_t timeIndex,
                      const std::string & datasetName)
        {
            std::vector<Moment> moments;
            for(size_t row = 1; row < rows.size(); ++row)
            {
                const auto cell{cellAt(rows[row], timeIndex)};
                const auto moment{parseMoment(cell)};
                if(!moment.has_value())
                {
                    return lbnl::Unexpected<std::string>{std::format(
                      "{} data row {}: cannot read the time '{}'", datasetName, row, trimmed(cell))};
                }
                moments.push_back(moment.value());
            }
            return moments;
        }

        //! Seconds from the first row to each row. Years are dropped, so a run that crosses
        //! New Year continues into the next nominal year instead of jumping back.
        std::vector<double> offsetsFrom(const std::vector<Moment> & moments)
        {
            std::vector<double> offsets;
            double wraps{0.0};
            for(const auto & moment : moments)
            {
                const double inYear{
                  secondsIntoYear(moment.month, moment.day, moment.hour, moment.minute, moment.second)};
                if(!offsets.empty() && inYear + wraps < offsets.back())
                {
                    wraps += secondsPerNominalYear();
                }
                offsets.push_back(inYear + wraps);
            }
            return offsets;
        }

        ///////////////////////////////////////////////////////////////////////////////////
        // Columns -> the clock
        ///////////////////////////////////////////////////////////////////////////////////

        using Failure = lbnl::Unexpected<std::string>;

        //! Where the file's columns are: the clock columns, if present, and every role
        //! column with its index. A header that is none of these is reported, not read.
        struct Layout
        {
            std::optional<size_t> time;
            std::optional<size_t> timestep;
            std::vector<std::pair<SeriesRole, size_t>> roles;
            std::vector<std::string> ignored;
        };

        Layout layoutOf(const std::vector<std::string> & headers)
        {
            Layout layout;
            for(size_t index = 0; index < headers.size(); ++index)
            {
                const auto header{trimmed(headers[index])};
                if(header.empty())
                {
                    continue;
                }
                if(!layout.time.has_value() && isTimeHeader(header))
                {
                    layout.time = index;
                }
                else if(!layout.timestep.has_value() && isTimestepHeader(header))
                {
                    layout.timestep = index;
                }
                else if(const auto role{roleForHeader(header)}; role.has_value())
                {
                    layout.roles.emplace_back(role.value(), index);
                }
                else
                {
                    layout.ignored.push_back(header);
                }
            }
            return layout;
        }

        //! The clock a file states: its axis, and where the first step's length came from.
        struct Clock
        {
            TimeAxis axis;
            FirstStep firstStep{FirstStep::FromColumn};
        };

        using ClockResult = lbnl::ExpectedExt<Clock, std::string>;

        double secondsOf(const Moment & moment)
        {
            return secondsIntoYear(moment.month, moment.day, moment.hour, moment.minute, moment.second);
        }

        //! The start of the run as the caller states it, in the Time column's own format.
        lbnl::ExpectedExt<Moment, std::string> parseStart(const std::string & start, const std::string & datasetName)
        {
            const auto moment{parseMoment(start)};
            if(!moment.has_value())
            {
                return lbnl::Unexpected<std::string>{
                  std::format("{}: cannot read the start '{}'", datasetName, trimmed(start))};
            }
            return moment.value();
        }

        //! A stamp's clock hour, 0..23, is the axis's hour 1..24 less one.
        TimeAxis startAxis(const Moment & moment)
        {
            return TimeAxis{.month = moment.month, .day = moment.day, .hour = moment.hour + 1U, .minute = moment.minute};
        }

        TimeAxis axisWith(const TimeAxis & start, const std::vector<double> & lengths)
        {
            TimeAxis axis{start};
            axis.stepSeconds = stepSecondsOf(lengths);
            return axis;
        }

        //! The gaps between consecutive times, each of which must be positive.
        lbnl::ExpectedExt<std::vector<double>, std::string> gapsBetween(const std::vector<double> & offsets,
                                                                       const std::string & datasetName)
        {
            std::vector<double> gaps;
            for(size_t index = 1; index < offsets.size(); ++index)
            {
                const double gap{offsets[index] - offsets[index - 1]};
                if(gap <= 0.0)
                {
                    return Failure{
                      std::format("{} data row {} does not come after data row {}", datasetName, index + 1, index)};
                }
                gaps.push_back(gap);
            }
            return gaps;
        }

        //! One positive length in seconds per row.
        lbnl::ExpectedExt<std::vector<double>, std::string>
          readTimesteps(const std::vector<std::vector<std::string>> & rows,
                        const size_t timestepIndex,
                        const std::string & datasetName)
        {
            std::vector<double> lengths;
            for(size_t row = 1; row < rows.size(); ++row)
            {
                const auto cell{cellAt(rows[row], timestepIndex)};
                const auto length{toDouble(cell)};
                if(!length.has_value())
                {
                    return Failure{
                      std::format("{} data row {}: cannot read the timestep '{}'", datasetName, row, trimmed(cell))};
                }
                if(length.value() <= 0.0)
                {
                    return Failure{std::format(
                      "{} data row {}: the timestep must be positive, got {:g}", datasetName, row, length.value())};
                }
                lengths.push_back(length.value());
            }
            return lengths;
        }

        //! The first step's length and where it came from.
        struct FirstLength
        {
            double seconds{0.0};
            FirstStep source{FirstStep::Assumed};
        };

        using FirstLengthResult = lbnl::ExpectedExt<FirstLength, std::string>;

        //! From the stated start to the first stamp. Years are dropped, so a start later in
        //! the year than the first stamp is the year before, as the stamps themselves wrap
        //! in offsetsFrom; only a start at the first stamp itself leaves nothing for a step.
        FirstLengthResult firstLengthFromStart(const std::string & start,
                                               const Moment & firstStamp,
                                               const std::string & firstStampText,
                                               const std::string & datasetName)
        {
            return parseStart(start, datasetName).and_then([&](const Moment & begin) -> FirstLengthResult {
                const double before{secondsOf(begin)};
                const double stamp{secondsOf(firstStamp)};
                if(before == stamp)
                {
                    return Failure{std::format("{}: the start '{}' is the first time '{}', which leaves no first step",
                                               datasetName,
                                               trimmed(start),
                                               trimmed(firstStampText))};
                }
                const double seconds{before < stamp ? stamp - before : stamp + secondsPerNominalYear() - before};
                return FirstLength{.seconds = seconds, .source = FirstStep::Stated};
            });
        }

        //! Whether a Timestep column says the same as the stamps wherever both speak: every
        //! length after the first equals the gap ending at its row, within half a second.
        bool agreesWithGaps(const std::vector<double> & lengths, const std::vector<double> & gaps)
        {
            return lengths.size() == gaps.size() + 1U
                   && std::ranges::equal(lengths | std::views::drop(1), gaps, [](const double length, const double gap) {
                          return std::abs(length - gap) <= 0.5;
                      });
        }

        //! A Timestep column beside the stamps supplies the first length when it agrees with
        //! them everywhere else. One that contradicts the stamps, or cannot be read, is not
        //! trusted: the caller chose the stamps.
        std::optional<double> firstLengthFromColumn(const std::vector<std::vector<std::string>> & rows,
                                                    const Layout & layout,
                                                    const std::vector<double> & gaps)
        {
            if(!layout.timestep.has_value())
            {
                return std::nullopt;
            }
            const auto lengths{readTimesteps(rows, layout.timestep.value(), "")};
            if(!lengths.has_value() || !agreesWithGaps(lengths.value(), gaps))
            {
                return std::nullopt;
            }
            return lengths->front();
        }

        //! The stamps give every length but the first. That one is the stated start's
        //! distance to the first stamp, else the first entry of a Timestep column that
        //! agrees with the stamps, else, assumed, the gap to the second row.
        FirstLengthResult firstLengthFor(const std::vector<std::vector<std::string>> & rows,
                                         const Layout & layout,
                                         const std::optional<std::string> & start,
                                         const std::vector<Moment> & moments,
                                         const std::vector<double> & gaps,
                                         const std::string & datasetName)
        {
            if(start.has_value())
            {
                return firstLengthFromStart(start.value(), moments.front(), cellAt(rows[1], layout.time.value()), datasetName);
            }
            if(const auto fromColumn{firstLengthFromColumn(rows, layout, gaps)}; fromColumn.has_value())
            {
                return FirstLength{.seconds = fromColumn.value(), .source = FirstStep::FromColumn};
            }
            if(gaps.empty())
            {
                return Failure{datasetName
                               + " has one row with a time and no start: how long its step lasts cannot be known"};
            }
            return FirstLength{.seconds = gaps.front(), .source = FirstStep::Assumed};
        }

        std::vector<double> withFirst(const double first, const std::vector<double> & gaps)
        {
            std::vector<double> lengths{first};
            lengths.insert(lengths.end(), gaps.begin(), gaps.end());
            return lengths;
        }

        //! The Time rule: every row is a step ending at its stamp, as EnergyPlus reads a
        //! weather file - n rows, n steps, the run starting one first length before the
        //! first stamp.
        ClockResult clockFromTimes(const std::vector<std::vector<std::string>> & rows,
                                   const Layout & layout,
                                   const std::optional<std::string> & start,
                                   const std::string & datasetName)
        {
            return readMoments(rows, layout.time.value(), datasetName)
              .and_then([&](const std::vector<Moment> & moments) -> ClockResult {
                  return gapsBetween(offsetsFrom(moments), datasetName)
                    .and_then([&](const std::vector<double> & gaps) -> ClockResult {
                        return firstLengthFor(rows, layout, start, moments, gaps, datasetName)
                          .transform([&](const FirstLength & first) {
                              const auto begins{axisAt(secondsOf(moments.front()) - first.seconds)};
                              return Clock{.axis = axisWith(begins, withFirst(first.seconds, gaps)),
                                           .firstStep = first.source};
                          });
                    });
              });
        }

        //! Without a Time column the run starts where the caller says, else on 1 January 00:00.
        lbnl::ExpectedExt<TimeAxis, std::string> statedOrDefaultStart(const std::optional<std::string> & start,
                                                                      const std::string & datasetName)
        {
            if(!start.has_value())
            {
                return TimeAxis{};
            }
            return parseStart(start.value(), datasetName).transform([](const Moment & begin) { return startAxis(begin); });
        }

        //! A stamp is the end of its row's step under every rule, so beside a Timestep
        //! column, or under a fixed step, a Time column places the start one step before
        //! the first stamp and a stated start is not read.
        lbnl::ExpectedExt<TimeAxis, std::string> startBefore(const std::vector<std::vector<std::string>> & rows,
                                                             const std::optional<size_t> timeIndex,
                                                             const double firstLength,
                                                             const std::optional<std::string> & start,
                                                             const std::string & datasetName)
        {
            if(!timeIndex.has_value())
            {
                return statedOrDefaultStart(start, datasetName);
            }
            const auto cell{cellAt(rows[1], timeIndex.value())};
            const auto moment{parseMoment(cell)};
            if(!moment.has_value())
            {
                return Failure{std::format("{} data row 1: cannot read the time '{}'", datasetName, trimmed(cell))};
            }
            return axisAt(secondsOf(moment.value()) - firstLength);
        }

        //! The Timestep rule: row i is step i, of the length it states.
        ClockResult clockFromTimesteps(const std::vector<std::vector<std::string>> & rows,
                                       const Layout & layout,
                                       const std::optional<std::string> & start,
                                       const std::string & datasetName)
        {
            if(!layout.timestep.has_value())
            {
                return Failure{datasetName + " has no Timestep column (Timestep, Step, Step Seconds or dt)"};
            }
            return readTimesteps(rows, layout.timestep.value(), datasetName)
              .and_then([&](const std::vector<double> & lengths) {
                  return startBefore(rows, layout.time, lengths.front(), start, datasetName)
                    .transform([&lengths](const TimeAxis & begins) {
                        return Clock{.axis = axisWith(begins, lengths), .firstStep = FirstStep::FromColumn};
                    });
              });
        }

        //! A fixed step: every row is one step of the stated length.
        ClockResult clockFixed(const std::vector<std::vector<std::string>> & rows,
                               const Layout & layout,
                               const FixedStep & fixed,
                               const std::optional<std::string> & start,
                               const std::string & datasetName)
        {
            if(fixed.seconds <= 0.0)
            {
                return Failure{
                  std::format("{}: the fixed step must be positive, got {:g}", datasetName, fixed.seconds)};
            }
            return startBefore(rows, layout.time, fixed.seconds, start, datasetName)
              .transform([&fixed](const TimeAxis & begins) {
                  TimeAxis axis{begins};
                  axis.stepSeconds = fixed.seconds;
                  return Clock{.axis = axis, .firstStep = FirstStep::Stated};
              });
        }

        //! AutoStep made definite by the columns the file has; any other choice as stated.
        StepChoice resolved(const StepChoice & stepChoice, const Layout & layout)
        {
            if(!std::holds_alternative<AutoStep>(stepChoice))
            {
                return stepChoice;
            }
            if(layout.time.has_value())
            {
                return StepColumn::Time;
            }
            return layout.timestep.has_value() ? StepChoice{StepColumn::Timestep} : StepChoice{FixedStep{}};
        }

        ClockResult clockFor(const std::vector<std::vector<std::string>> & rows,
                             const Layout & layout,
                             const StepChoice & stated,
                             const std::optional<std::string> & start,
                             const std::string & datasetName)
        {
            const auto stepChoice{resolved(stated, layout)};
            if(const auto * fixed{std::get_if<FixedStep>(&stepChoice)}; fixed != nullptr)
            {
                return clockFixed(rows, layout, *fixed, start, datasetName);
            }
            if(std::get<StepColumn>(stepChoice) == StepColumn::Timestep)
            {
                return clockFromTimesteps(rows, layout, start, datasetName);
            }
            if(!layout.time.has_value())
            {
                return Failure{datasetName + " has no Time column (Time, Date, Datetime or Timestamp)"};
            }
            return clockFromTimes(rows, layout, start, datasetName);
        }

        //! Every role column, every data row; of two columns naming one role the last wins.
        std::vector<Series> seriesFrom(const std::vector<std::vector<std::string>> & rows, const Layout & layout)
        {
            TimeSeriesData data;
            for(const auto & [role, index] : layout.roles)
            {
                storeSeries(data, role, fillGaps(readColumn(rows, index, 1U)));
            }
            return data.series;
        }

        //! Exported stamps need a year to be well-formed dates. It is a fixed non-leap year
        //! that import drops again; the axis carries no year of its own.
        constexpr int nominalYear{2026};

        std::chrono::sys_seconds exportStart(const TimeAxis & axis)
        {
            const std::chrono::year_month_day date{std::chrono::year{nominalYear},
                                                   std::chrono::month{static_cast<unsigned>(axis.month)},
                                                   std::chrono::day{static_cast<unsigned>(axis.day)}};
            return std::chrono::sys_days{date} + std::chrono::hours{static_cast<long long>(clockHour(axis))}
                   + std::chrono::minutes{static_cast<long long>(axis.minute)};
        }

        std::string stampText(const std::chrono::sys_seconds moment, const bool wholeMinutes)
        {
            return wholeMinutes ? std::format("{:%Y-%m-%d %H:%M}", moment)
                                : std::format("{:%Y-%m-%d %H:%M:%S}", moment);
        }
    }   // namespace

    std::string displayHeaderForRole(const SeriesRole role)
    {
        const auto stored{seriesRoleToString(role)};
        std::string spelled;
        for(const char letter : stored)
        {
            const bool boundary{!spelled.empty() && std::isupper(static_cast<unsigned char>(letter))
                                && std::islower(static_cast<unsigned char>(spelled.back()))};
            if(boundary)
            {
                spelled.push_back(' ');
            }
            spelled.push_back(letter);
        }
        return spelled;
    }

    std::string describeRoles()
    {
        const auto names{seriesRoleStrings()};
        const auto widest{std::ranges::max(names | std::views::transform([](const std::string & name) { return name.size(); }))};
        const auto row{[width = widest + 2U](const std::string & left, const std::string & right) {
            return std::format("{:<{}}{}", left, width, right);
        }};
        return std::accumulate(names.begin(), names.end(), row("Role", "CSV header"), [&row](std::string table, const std::string & name) {
            return std::move(table) + "\n" + row(name, displayHeaderForRole(seriesRoleFromString(name)));
        });
    }

    std::string describeFirstStep(const ImportResult & result)
    {
        const auto length{stepLabel(TimeAxis{.stepSeconds = stepLengths(result.data.axis, 1U).front()})};
        switch(result.firstStep)
        {
            case FirstStep::Stated:
                return std::format("first step {}, as stated", length);
            case FirstStep::FromColumn:
                return std::format("first step {}, from the Timestep column", length);
            case FirstStep::Assumed:
                return std::format("first step {}, assumed from the gap to the second row", length);
        }
        return {};
    }

    lbnl::ExpectedExt<ImportResult, std::string> readFromString(const std::string & content,
                                                                const std::string & datasetName,
                                                                const StepChoice & stepChoice,
                                                                const std::optional<std::string> & start)
    {
        const auto rows{parseRows(content)};
        if(rows.size() < 2)
        {
            return Failure{datasetName + " has no data rows"};
        }
        const auto layout{layoutOf(rows.front())};
        if(layout.roles.empty())
        {
            return Failure{datasetName + " has no column named after a time series quantity"};
        }
        const auto clock{clockFor(rows, layout, stepChoice, start, datasetName)};
        if(!clock.has_value())
        {
            return Failure{clock.error()};
        }
        ImportResult result{.ignoredHeaders = layout.ignored,
                            .hadTimeColumn = layout.time.has_value(),
                            .hadTimestepColumn = layout.timestep.has_value(),
                            .firstStep = clock->firstStep};
        result.data.Name = datasetName;
        result.data.Source = "Imported";
        result.data.axis = clock->axis;
        result.data.series = seriesFrom(rows, layout);
        result.data.UUID = contentUuid(result.data);
        return result;
    }

    lbnl::ExpectedExt<ImportResult, std::string>
      read(const std::string & fileName, const StepChoice & stepChoice, const std::optional<std::string> & start)
    {
        if(!std::filesystem::exists(fileName))
        {
            return Failure{std::filesystem::path{fileName}.filename().string() + " does not exist"};
        }
        const auto datasetName{std::filesystem::path{fileName}.stem().string()};
        return readFromString(File::loadToString(fileName), datasetName, stepChoice, start);
    }

    std::string writeToString(const TimeSeriesData & data)
    {
        std::string content{"Time,Timestep"};
        for(const auto & series : data.series)
        {
            content += "," + displayHeaderForRole(series.role);
        }
        content += "\n";

        const auto start{exportStart(data.axis)};
        const auto lengths{stepLengths(data.axis, steps(data))};
        const bool wholeMinutes{std::ranges::all_of(
          lengths, [](const double length) { return std::abs(std::remainder(length, 60.0)) < 1e-6; })};
        double elapsed{0.0};
        for(size_t index = 0; index < lengths.size(); ++index)
        {
            // The stamp is the end of the step, as the readers take it.
            elapsed += lengths[index];
            content += stampText(start + std::chrono::seconds{std::llround(elapsed)}, wholeMinutes);
            // Shortest text that reads back as the same double: the round trip is exact.
            content += std::format(",{}", lengths[index]);
            for(const auto & series : data.series)
            {
                content += std::format(",{}", series.values[index]);
            }
            content += "\n";
        }
        return content;
    }

    void writeToFile(const TimeSeriesData & data, const std::string & fileName)
    {
        File::createFileFromString(fileName, writeToString(data));
    }
}   // namespace TimeSeriesLibrary::Csv
