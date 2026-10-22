#include <algorithm>
#include <cctype>
#include <charconv>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <format>
#include <initializer_list>
#include <map>
#include <optional>
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

        //! The clock a file states: its axis and the first data row that is a step (row 1,
        //! or row 2 under the Time rule, where row 1 is only the start).
        struct Clock
        {
            TimeAxis axis;
            size_t firstDataRow{1U};
        };

        using ClockResult = lbnl::ExpectedExt<Clock, std::string>;

        TimeAxis startAxis(const Moment & moment)
        {
            return TimeAxis{.month = moment.month, .day = moment.day, .hour = moment.hour, .minute = moment.minute};
        }

        //! Even steps stay one stepSeconds; uneven ones are stated per step, the first of
        //! them doubling as stepSeconds. Lengths within half a second count as equal.
        TimeAxis axisWith(const TimeAxis & start, const std::vector<double> & lengths)
        {
            TimeAxis axis{start};
            axis.stepSeconds = lengths.front();
            const bool even{std::ranges::all_of(
              lengths, [&lengths](const double length) { return std::abs(length - lengths.front()) <= 0.5; })};
            if(!even)
            {
                axis.stepLengths = lengths;
            }
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

        //! The Time rule, EnergyPlus-like for now: a row's value belongs to the step that
        //! ENDS at its time, so the first row is the start of the run and every later row
        //! is one step - n rows, n-1 steps. Which row drives a step is still under
        //! discussion; when that is settled, this function changes and nothing else does.
        ClockResult clockFromTimes(const std::vector<std::vector<std::string>> & rows,
                                   const size_t timeIndex,
                                   const std::string & datasetName)
        {
            return readMoments(rows, timeIndex, datasetName)
              .and_then([&datasetName](const std::vector<Moment> & moments) -> ClockResult {
                  if(moments.size() < 2U)
                  {
                      return Failure{datasetName
                                     + " needs at least two rows with times: the first row is the start "
                                       "of the run, each later row ends a step"};
                  }
                  return gapsBetween(offsetsFrom(moments), datasetName)
                    .transform([&moments](const std::vector<double> & gaps) {
                        return Clock{.axis = axisWith(startAxis(moments.front()), gaps), .firstDataRow = 2U};
                    });
              });
        }

        //! A stamp is the end of its row's step under every rule, so beside a Timestep
        //! column, or under a fixed step, a Time column places the start one step before
        //! the first stamp. Without a Time column the run starts on 1 January 00:00.
        lbnl::ExpectedExt<TimeAxis, std::string> startBefore(const std::vector<std::vector<std::string>> & rows,
                                                             const std::optional<size_t> timeIndex,
                                                             const double firstLength,
                                                             const std::string & datasetName)
        {
            if(!timeIndex.has_value())
            {
                return TimeAxis{};
            }
            const auto cell{cellAt(rows[1], timeIndex.value())};
            const auto moment{parseMoment(cell)};
            if(!moment.has_value())
            {
                return Failure{std::format("{} data row 1: cannot read the time '{}'", datasetName, trimmed(cell))};
            }
            const double stampSeconds{
              secondsIntoYear(moment->month, moment->day, moment->hour, moment->minute, moment->second)};
            return axisAt(stampSeconds - firstLength);
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

        //! The Timestep rule: row i is step i, of the length it states.
        ClockResult clockFromTimesteps(const std::vector<std::vector<std::string>> & rows,
                                       const Layout & layout,
                                       const std::string & datasetName)
        {
            if(!layout.timestep.has_value())
            {
                return Failure{datasetName + " has no Timestep column (Timestep, Step, Step Seconds or dt)"};
            }
            return readTimesteps(rows, layout.timestep.value(), datasetName)
              .and_then([&rows, &layout, &datasetName](const std::vector<double> & lengths) {
                  return startBefore(rows, layout.time, lengths.front(), datasetName)
                    .transform([&lengths](const TimeAxis & start) {
                        return Clock{.axis = axisWith(start, lengths), .firstDataRow = 1U};
                    });
              });
        }

        //! A fixed step: every row is one step of the stated length.
        ClockResult clockFixed(const std::vector<std::vector<std::string>> & rows,
                               const Layout & layout,
                               const FixedStep & fixed,
                               const std::string & datasetName)
        {
            if(fixed.seconds <= 0.0)
            {
                return Failure{
                  std::format("{}: the fixed step must be positive, got {:g}", datasetName, fixed.seconds)};
            }
            return startBefore(rows, layout.time, fixed.seconds, datasetName).transform([&fixed](const TimeAxis & start) {
                TimeAxis axis{start};
                axis.stepSeconds = fixed.seconds;
                return Clock{.axis = axis, .firstDataRow = 1U};
            });
        }

        ClockResult clockFor(const std::vector<std::vector<std::string>> & rows,
                             const Layout & layout,
                             const StepChoice & stepChoice,
                             const std::string & datasetName)
        {
            if(const auto * fixed{std::get_if<FixedStep>(&stepChoice)}; fixed != nullptr)
            {
                return clockFixed(rows, layout, *fixed, datasetName);
            }
            if(std::get<StepColumn>(stepChoice) == StepColumn::Timestep)
            {
                return clockFromTimesteps(rows, layout, datasetName);
            }
            if(!layout.time.has_value())
            {
                return Failure{datasetName + " has no Time column (Time, Date, Datetime or Timestamp)"};
            }
            return clockFromTimes(rows, layout.time.value(), datasetName);
        }

        //! Every role column from the first data row on; of two columns naming one role
        //! the last wins.
        std::vector<Series> seriesFrom(const std::vector<std::vector<std::string>> & rows,
                                       const Layout & layout,
                                       const size_t firstDataRow)
        {
            TimeSeriesData data;
            for(const auto & [role, index] : layout.roles)
            {
                storeSeries(data, role, fillGaps(readColumn(rows, index, firstDataRow)));
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
            return std::chrono::sys_days{date} + std::chrono::hours{static_cast<long long>(axis.hour)}
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

    lbnl::ExpectedExt<ImportResult, std::string>
      readFromString(const std::string & content, const std::string & datasetName, const StepChoice & stepChoice)
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
        const auto clock{clockFor(rows, layout, stepChoice, datasetName)};
        if(!clock.has_value())
        {
            return Failure{clock.error()};
        }
        ImportResult result{.ignoredHeaders = layout.ignored,
                            .hadTimeColumn = layout.time.has_value(),
                            .hadTimestepColumn = layout.timestep.has_value(),
                            .firstRowWasStart = clock->firstDataRow > 1U};
        result.data.Name = datasetName;
        result.data.Source = "Imported";
        result.data.axis = clock->axis;
        result.data.series = seriesFrom(rows, layout, clock->firstDataRow);
        result.data.UUID = contentUuid(result.data);
        return result;
    }

    lbnl::ExpectedExt<ImportResult, std::string> read(const std::string & fileName, const StepChoice & stepChoice)
    {
        if(!std::filesystem::exists(fileName))
        {
            return Failure{std::filesystem::path{fileName}.filename().string() + " does not exist"};
        }
        const auto datasetName{std::filesystem::path{fileName}.stem().string()};
        return readFromString(File::loadToString(fileName), datasetName, stepChoice);
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
            content += std::format(",{:g}", lengths[index]);
            for(const auto & series : data.series)
            {
                content += std::format(",{:.4g}", series.values[index]);
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
