#include <algorithm>
#include <charconv>
#include <cmath>
#include <filesystem>
#include <map>
#include <optional>
#include <sstream>

#include "Epw.hxx"

#include "ContentHash.hxx"

#include "LibraryUtilities/FileManipulation.hxx"

namespace TimeSeriesLibrary::Epw
{
    namespace
    {
        constexpr double stefanBoltzmann{5.670374419e-8};
        constexpr size_t headerLines{8U};
        //! A record with fewer fields does not reach the wind columns and is skipped.
        constexpr size_t minimumFields{22U};

        struct FieldSpec
        {
            size_t index;
            std::string label;
            std::string unit;
            double sentinel;
        };

        //! The fields that map straight onto a role, with the factor taking the
        //! file's unit to ours. Zero-based EPW record positions.
        struct DirectSeries
        {
            FieldSpec field;
            SeriesRole role;
            double scale;
        };

        const std::vector<DirectSeries> & directSeriesList()
        {
            static const std::vector<DirectSeries> series{
              {FieldSpec{6U, "Dry bulb temperature", "degC", 99.9},
               SeriesRole::AirTemperature,
               1.0},
              {FieldSpec{8U, "Relative humidity", "%", 999.0},
               SeriesRole::RelativeHumidity,
               0.01},
              {FieldSpec{21U, "Wind speed", "m/s", 999.0}, SeriesRole::WindSpeed, 1.0},
              {FieldSpec{20U, "Wind direction", "deg", 999.0}, SeriesRole::WindDirection, 1.0},
            };
            return series;
        }

        const FieldSpec & horizontalInfraredField()
        {
            static const FieldSpec field{12U, "Horizontal infrared radiation intensity", "Wh/m2", 9999.0};
            return field;
        }

        //! Parsed for presence only: no role in the present vocabulary can express
        //! them, and the import reports them so the omission is visible.
        const std::vector<FieldSpec> & unmappedFields()
        {
            static const std::vector<FieldSpec> fields{
              {9U, "Atmospheric station pressure", "Pa", 999999.0},
              {13U, "Global horizontal radiation", "Wh/m2", 9999.0},
              {14U, "Direct normal radiation", "Wh/m2", 9999.0},
              {15U, "Diffuse horizontal radiation", "Wh/m2", 9999.0},
              {22U, "Total sky cover", "tenths", 99.0},
              {23U, "Opaque sky cover", "tenths", 99.0},
              {30U, "Snow depth", "cm", 999.0},
              {32U, "Albedo", "", 999.0},
              {33U, "Liquid precipitation depth", "mm", 999.0},
              {34U, "Liquid precipitation quantity", "h", 99.0},
            };
            return fields;
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

        std::optional<double> toDouble(std::string_view cell)
        {
            const auto plain{trimmed(cell)};
            double value{};
            const auto [ptr, ec]{std::from_chars(plain.data(), plain.data() + plain.size(), value)};
            const bool complete{ec == std::errc{} && ptr == plain.data() + plain.size()};
            return complete ? std::optional{value} : std::nullopt;
        }

        std::vector<std::string> splitCommas(std::string_view line)
        {
            std::vector<std::string> cells{std::string{}};
            for(const char letter : line)
            {
                if(letter == ',')
                {
                    cells.emplace_back();
                }
                else if(letter != '\r')
                {
                    cells.back().push_back(letter);
                }
            }
            return cells;
        }

        std::optional<double> readField(const std::vector<std::string> & record,
                                        const FieldSpec & field)
        {
            if(field.index >= record.size())
            {
                return std::nullopt;
            }
            const auto value{toDouble(record[field.index])};
            const bool missing{!value.has_value() || value.value() >= field.sentinel};
            return missing ? std::nullopt : value;
        }

        //! Replace missing readings by the last good one, or by the first good one
        //! at the head. A column with no good reading at all yields nothing.
        std::optional<std::vector<double>> filled(const std::vector<std::optional<double>> & values)
        {
            const auto firstGood{std::ranges::find_if(
              values, [](const std::optional<double> & value) { return value.has_value(); })};
            if(firstGood == values.end())
            {
                return std::nullopt;
            }

            double previous{firstGood->value()};
            std::vector<double> result;
            result.reserve(values.size());
            for(const auto & value : values)
            {
                previous = value.value_or(previous);
                result.push_back(previous);
            }
            return result;
        }

        std::vector<std::optional<double>> column(const std::vector<std::vector<std::string>> & records,
                                                  const FieldSpec & field)
        {
            std::vector<std::optional<double>> values;
            values.reserve(records.size());
            std::ranges::transform(records, std::back_inserter(values), [&field](const auto & record) {
                return readField(record, field);
            });
            return values;
        }

        Location parseLocation(const std::vector<std::string> & parts)
        {
            const auto text{[&parts](const size_t index) {
                return index < parts.size() ? trimmed(parts[index]) : std::string{};
            }};
            const auto number{[&text](const size_t index) {
                return toDouble(text(index)).value_or(0.0);
            }};
            return Location{.city = text(1U),
                            .region = text(2U),
                            .country = text(3U),
                            .wmo = text(5U),
                            .latitude = number(6U),
                            .longitude = number(7U),
                            .timeZone = number(8U),
                            .elevation = number(9U)};
        }

        std::vector<std::vector<std::string>> dataRecords(const std::string & content)
        {
            std::vector<std::vector<std::string>> records;
            std::istringstream stream{content};
            std::string line;
            size_t lineNumber{0U};
            while(std::getline(stream, line))
            {
                if(lineNumber++ < headerLines)
                {
                    continue;
                }
                auto fields{splitCommas(line)};
                if(fields.size() >= minimumFields)
                {
                    records.emplace_back(std::move(fields));
                }
            }
            return records;
        }
    }   // namespace

    std::vector<double> skyTemperature(const std::vector<double> & horizontalInfrared)
    {
        std::vector<double> temperatures;
        temperatures.reserve(horizontalInfrared.size());
        std::ranges::transform(
          horizontalInfrared, std::back_inserter(temperatures), [](const double intensity) {
              return std::pow((std::max)(intensity, 1.0) / stefanBoltzmann, 0.25) - 273.15;
          });
        return temperatures;
    }

    lbnl::ExpectedExt<ImportResult, std::string>
      readFromString(const std::string & content, const std::string & datasetName)
    {
        using Failure = lbnl::Unexpected<std::string>;

        const auto firstLine{content.substr(0, content.find('\n'))};
        const auto locationParts{splitCommas(firstLine)};
        if(locationParts.empty() || trimmed(locationParts.front()) != "LOCATION")
        {
            return Failure{datasetName + " has no LOCATION header"};
        }

        const auto records{dataRecords(content)};
        if(records.empty())
        {
            return Failure{datasetName + " has no data records"};
        }

        ImportResult result;
        result.location = parseLocation(locationParts);
        result.data.Name = datasetName;
        result.data.Source = "Imported";

        for(const auto & direct : directSeriesList())
        {
            if(const auto values{filled(column(records, direct.field))}; values.has_value())
            {
                std::vector<double> scaled;
                scaled.reserve(values->size());
                std::ranges::transform(values.value(),
                                       std::back_inserter(scaled),
                                       [&direct](const double value) { return value * direct.scale; });
                result.data.series.push_back(Series{.role = direct.role, .values = std::move(scaled)});
            }
        }
        if(const auto infrared{filled(column(records, horizontalInfraredField()))}; infrared.has_value())
        {
            result.data.series.push_back(Series{.role = SeriesRole::RadiantTemperature,
                                                   .values = skyTemperature(infrared.value())});
        }

        for(const auto & field : unmappedFields())
        {
            if(filled(column(records, field)).has_value())
            {
                const auto suffix{field.unit.empty() ? std::string{} : " [" + field.unit + "]"};
                result.unmappedColumns.push_back(field.label + suffix);
            }
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
}   // namespace TimeSeriesLibrary::Epw
