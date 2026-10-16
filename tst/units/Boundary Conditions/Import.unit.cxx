#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "THMZ/ZipModule/ZipModule.hxx"
#include "TimeSeriesData/Import.hxx"

using TimeSeriesLibrary::SeriesRole;

namespace Import = TimeSeriesLibrary::Import;

namespace
{
    //! One EPW data record: 35 comma-separated fields, zeros unless set.
    std::string epwRecord(const std::string & dryBulb)
    {
        std::vector<std::string> fields(35U, "0");
        fields[0] = "2026";
        fields[1] = "1";
        fields[2] = "1";
        fields[3] = "1";
        fields[6] = dryBulb;
        fields[8] = "50";
        fields[9] = "101325";
        fields[12] = "349.28";
        fields[20] = "180";
        fields[21] = "3";
        std::string record;
        for(const auto & field : fields)
        {
            record += (record.empty() ? "" : ",") + field;
        }
        return record + "\n";
    }

    std::string epwContent()
    {
        std::string content{"LOCATION,Sydney,NSW,AUS,TMY,947670,-33.95,151.18,10.0,3.0\n"};
        for(size_t index = 0; index < 7U; ++index)
        {
            content += "HEADER LINE\n";
        }
        return content + epwRecord("21.3") + epwRecord("21.1");
    }

    const std::string csvContent{"Time,Air Temperature,RelativeHumidity,Comfort Index\n"
                                 "2026-01-01 00:00,21.3,0.52,5\n"
                                 "2026-01-01 01:00,21.1,0.55,6\n"};

    std::filesystem::path scratch(const std::string & name)
    {
        return std::filesystem::temp_directory_path() / name;
    }

    std::filesystem::path written(const std::string & name, const std::string & content)
    {
        const auto path{scratch(name)};
        std::ofstream file{path, std::ios::binary};
        file << content;
        return path;
    }
}   // namespace

TEST(TestImport, DispatchesByExtensionWhateverItsCase)
{
    const auto csv{Import::importDatasets(written("import-sample.CSV", csvContent))};
    ASSERT_TRUE(csv.has_value()) << csv.error();
    ASSERT_EQ(csv->datasets.size(), 1U);
    EXPECT_EQ(csv->datasets[0].data.Name, "import-sample");
    EXPECT_EQ(csv->datasets[0].skippedColumns, (std::vector<std::string>{"Comfort Index"}));
    EXPECT_TRUE(TimeSeriesLibrary::hasRole(csv->datasets[0].data, SeriesRole::RelativeHumidity));

    const auto weather{Import::importDatasets(written("import-sample.epw", epwContent()))};
    ASSERT_TRUE(weather.has_value()) << weather.error();
    ASSERT_EQ(weather->datasets.size(), 1U);
    EXPECT_TRUE(weather->datasets[0].skippedColumns.empty());
    EXPECT_EQ(TimeSeriesLibrary::steps(weather->datasets[0].data), 2U);
    EXPECT_TRUE(TimeSeriesLibrary::hasRole(weather->datasets[0].data, SeriesRole::RadiantTemperature));
    EXPECT_TRUE(weather->failedEntries.empty());
}

TEST(TestImport, UnsupportedExtensionIsAnError)
{
    const auto result{Import::importDatasets(written("import-sample.txt", csvContent))};
    ASSERT_FALSE(result.has_value());
    EXPECT_NE(result.error().find("Unsupported file type"), std::string::npos);
    EXPECT_EQ(Import::supportedExtensions(), (std::vector<std::string>{".epw", ".csv", ".zip"}));
}

TEST(TestImport, ZipYieldsEveryWeatherAndCsvEntry)
{
    const auto path{scratch("import-sample.zip").string()};
    ThermZip::zipFiles({{"city.epw", epwContent()}, {"indoor.csv", csvContent}, {"city.stat", "not a dataset"}}, path);
    const auto result{Import::importDatasets(path)};
    ASSERT_TRUE(result.has_value()) << result.error();
    ASSERT_EQ(result->datasets.size(), 2U);
    std::vector<std::string> names;
    for(const auto & imported : result->datasets)
    {
        names.push_back(imported.data.Name);
    }
    EXPECT_EQ(names, (std::vector<std::string>{"city", "indoor"}));
    EXPECT_TRUE(result->failedEntries.empty());
}

TEST(TestImport, ReadWeatherTakesTheOneWeatherFileOfAZip)
{
    const auto path{scratch("import-weather.zip").string()};
    ThermZip::zipFiles({{"city.epw", epwContent()}, {"city.ddy", "design days"}, {"city.stat", "statistics"}}, path);
    const auto weather{Import::readWeather(path)};
    ASSERT_TRUE(weather.has_value()) << weather.error();
    EXPECT_EQ(weather->data.Name, "city");
    EXPECT_EQ(weather->location.city, "Sydney");
    // Named after the entry rather than the zip, otherwise the same file read.
    const auto plain{Import::readWeather(written("import-plain.epw", epwContent()))};
    ASSERT_TRUE(plain.has_value());
    EXPECT_EQ(weather->data.series, plain->data.series);
    EXPECT_EQ(weather->data.UUID, plain->data.UUID);

    const auto two{scratch("import-two-weather.zip").string()};
    ThermZip::zipFiles({{"a.epw", epwContent()}, {"b.epw", epwContent()}}, two);
    const auto refused{Import::readWeather(two)};
    ASSERT_FALSE(refused.has_value());
    EXPECT_NE(refused.error().find("more than one"), std::string::npos);
    EXPECT_FALSE(Import::readWeather(written("import-sample.csv", csvContent)).has_value());
}

TEST(TestImport, FailedZipEntriesAreListedBesideTheGoodOnes)
{
    const std::map<std::string, std::string> entries{{"indoor.csv", csvContent}, {"broken.csv", "nothing,here\n1,2\n"}};
    const auto result{Import::importDatasetsFromEntries(entries)};
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_EQ(result->datasets.size(), 1U);
    ASSERT_EQ(result->failedEntries.size(), 1U);
    EXPECT_EQ(result->failedEntries[0].rfind("broken.csv: ", 0U), 0U);

    const auto nothing{Import::importDatasetsFromEntries({{"city.stat", "not a dataset"}})};
    ASSERT_FALSE(nothing.has_value());
    EXPECT_NE(nothing.error().find("no EnergyPlus weather"), std::string::npos);
}
