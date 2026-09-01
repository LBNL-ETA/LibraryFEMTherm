#include <algorithm>
#include <cctype>
#include <charconv>
#include <chrono>
#include <filesystem>
#include <format>
#include <map>
#include <optional>
#include <sstream>

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

        const auto & headers{rows.front()};
        for(size_t index = 0; index < headers.size(); ++index)
        {
            const auto header{trimmed(headers[index])};
            if(header.empty())
            {
                continue;
            }
            if(!result.hadTimeColumn && isTimeHeader(header))
            {
                result.hadTimeColumn = true;
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

        const std::chrono::sys_seconds start{
          std::chrono::sys_days{std::chrono::year{2026} / 1 / 1}};
        for(size_t index = 0; index < steps(data); ++index)
        {
            content += std::format("{:%Y-%m-%d %H:%M}",
                                   start + std::chrono::hours{static_cast<int>(index)});
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
