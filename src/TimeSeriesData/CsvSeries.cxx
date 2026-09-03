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

        std::vector<std::optional<double>>
          readColumn(const std::vector<std::vector<std::string>> & rows, const size_t index)
        {
            std::vector<std::optional<double>> column;
            column.reserve(rows.size() - 1);
            std::transform(rows.begin() + 1,
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

        //! The axis is the first row's position plus the spacing, which every row must keep.
        lbnl::ExpectedExt<TimeAxis, std::string> axisFromMoments(const std::vector<Moment> & moments,
                                                                 const std::string & datasetName)
        {
            using Failure = lbnl::Unexpected<std::string>;
            const auto & first{moments.front()};
            TimeAxis axis{.month = first.month, .day = first.day, .hour = first.hour, .minute = first.minute};
            if(moments.size() < 2U)
            {
                return axis;
            }
            const auto offsets{offsetsFrom(moments)};
            axis.stepSeconds = offsets[1] - offsets[0];
            if(axis.stepSeconds <= 0.0)
            {
                return Failure{datasetName + " data row 2 does not come after data row 1"};
            }
            for(size_t index = 2; index < offsets.size(); ++index)
            {
                const double gap{offsets[index] - offsets[index - 1]};
                if(std::abs(gap - axis.stepSeconds) > 0.5)
                {
                    return Failure{std::format("{} rows are not evenly spaced: data row {} is {:g} s "
                                               "after the previous one, expected {:g} s",
                                               datasetName,
                                               index + 1,
                                               gap,
                                               axis.stepSeconds)};
                }
            }
            return axis;
        }

        //! A file without a time column keeps the default axis: 1 January 00:00, hourly.
        lbnl::ExpectedExt<TimeAxis, std::string>
          timeAxisOf(const std::vector<std::vector<std::string>> & rows,
                     const std::optional<size_t> timeIndex,
                     const std::string & datasetName)
        {
            if(!timeIndex.has_value())
            {
                return TimeAxis{};
            }
            return readMoments(rows, timeIndex.value(), datasetName)
              .and_then([&datasetName](const std::vector<Moment> & moments) {
                  return axisFromMoments(moments, datasetName);
              });
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
      readFromString(const std::string & content, const std::string & datasetName)
    {
        using Failure = lbnl::Unexpected<std::string>;

        const auto rows{parseRows(content)};
        if(rows.size() < 2)
        {
            return Failure{datasetName + " has no data rows"};
        }

        ImportResult result;
        result.data.Name = datasetName;
        result.data.Source = "Imported";

        std::optional<size_t> timeIndex;
        const auto & headers{rows.front()};
        for(size_t index = 0; index < headers.size(); ++index)
        {
            const auto header{trimmed(headers[index])};
            if(header.empty())
            {
                continue;
            }
            if(!timeIndex.has_value() && isTimeHeader(header))
            {
                timeIndex = index;
                continue;
            }
            const auto role{roleForHeader(header)};
            if(!role.has_value())
            {
                result.ignoredHeaders.push_back(header);
                continue;
            }
            storeSeries(result.data, role.value(), fillGaps(readColumn(rows, index)));
        }

        if(result.data.series.empty())
        {
            return Failure{datasetName + " has no column named after a time series quantity"};
        }

        const auto axis{timeAxisOf(rows, timeIndex, datasetName)};
        if(!axis.has_value())
        {
            return Failure{axis.error()};
        }
        result.hadTimeColumn = timeIndex.has_value();
        result.data.axis = axis.value();
        result.data.UUID = contentUuid(result.data);
        return result;
    }

    lbnl::ExpectedExt<ImportResult, std::string> read(const std::string & fileName)
    {
        if(!std::filesystem::exists(fileName))
        {
            return lbnl::Unexpected<std::string>{
              std::filesystem::path{fileName}.filename().string() + " does not exist"};
        }
        const auto datasetName{std::filesystem::path{fileName}.stem().string()};
        return readFromString(File::loadToString(fileName), datasetName);
    }

    std::string writeToString(const TimeSeriesData & data)
    {
        std::string content{"Time"};
        for(const auto & series : data.series)
        {
            content += "," + displayHeaderForRole(series.role);
        }
        content += "\n";

        const auto start{exportStart(data.axis)};
        const bool wholeMinutes{std::abs(std::remainder(data.axis.stepSeconds, 60.0)) < 1e-6};
        for(size_t index = 0; index < steps(data); ++index)
        {
            const auto elapsed{std::chrono::seconds{
              std::llround(static_cast<double>(index) * data.axis.stepSeconds)}};
            content += stampText(start + elapsed, wholeMinutes);
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
