#include <gtest/gtest.h>

#include "TimeSeriesData/ContentHash.hxx"
#include "TimeSeriesData/CsvSeries.hxx"

using TimeSeriesLibrary::Channel;
using TimeSeriesLibrary::ChannelRole;
using TimeSeriesLibrary::TimeSeriesData;

namespace Csv = TimeSeriesLibrary::Csv;

TEST(TestCsvSeries, ImportAcceptsBothRoleSpellings)
{
    const std::string content{"Time,Air Temperature,RelativeHumidity,Comfort Index\n"
                              "2026-01-01 00:00,21.3,0.52,5\n"
                              "2026-01-01 01:00,21.1,0.55,6\n"};

    const auto result{Csv::readFromString(content, "sample")};
    ASSERT_TRUE(result.has_value());

    const auto & imported{result->data};
    EXPECT_EQ(imported.Name, "sample");
    EXPECT_TRUE(result->hadTimeColumn);
    ASSERT_EQ(result->ignoredHeaders.size(), 1U);
    EXPECT_EQ(result->ignoredHeaders.front(), "Comfort Index");

    EXPECT_EQ(TimeSeriesLibrary::steps(imported), 2U);
    const auto temperature{
      TimeSeriesLibrary::valuesForRole(imported, ChannelRole::AirTemperature)};
    ASSERT_TRUE(temperature.has_value());
    EXPECT_NEAR(temperature->at(0), 21.3, 1e-9);
    const auto humidity{
      TimeSeriesLibrary::valuesForRole(imported, ChannelRole::RelativeHumidity)};
    ASSERT_TRUE(humidity.has_value());
    EXPECT_NEAR(humidity->at(1), 0.55, 1e-9);

    EXPECT_EQ(imported.UUID, TimeSeriesLibrary::contentUuid(imported));
}

TEST(TestCsvSeries, ImportReadsSemicolonSeparatedDecimalCommas)
{
    const std::string content{"Air Temperature;Wind Speed\n"
                              "21,5;1,4\n"
                              "20,8;2,1\n"};

    const auto result{Csv::readFromString(content, "european")};
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->hadTimeColumn);

    const auto temperature{
      TimeSeriesLibrary::valuesForRole(result->data, ChannelRole::AirTemperature)};
    ASSERT_TRUE(temperature.has_value());
    EXPECT_NEAR(temperature->at(0), 21.5, 1e-9);
    const auto wind{TimeSeriesLibrary::valuesForRole(result->data, ChannelRole::WindSpeed)};
    ASSERT_TRUE(wind.has_value());
    EXPECT_NEAR(wind->at(1), 2.1, 1e-9);
}

TEST(TestCsvSeries, ImportCarriesLastReadingAcrossGaps)
{
    const std::string content{"Air Temperature,Wind Speed\n"
                              ",1\n"
                              "20,2\n"
                              ",3\n"
                              "22,4\n"};

    const auto result{Csv::readFromString(content, "gaps")};
    ASSERT_TRUE(result.has_value());

    const auto values{
      TimeSeriesLibrary::valuesForRole(result->data, ChannelRole::AirTemperature)};
    ASSERT_TRUE(values.has_value());
    ASSERT_EQ(values->size(), 4U);
    EXPECT_NEAR(values->at(0), 20.0, 1e-9);   // leading gap takes the first good reading
    EXPECT_NEAR(values->at(2), 20.0, 1e-9);   // mid-run gap carries the previous reading
    EXPECT_NEAR(values->at(3), 22.0, 1e-9);
}

TEST(TestCsvSeries, ImportKeepsLastColumnOnDuplicateRole)
{
    const std::string content{"Air Temperature,AirTemperature\n"
                              "20,25\n"};

    const auto result{Csv::readFromString(content, "duplicate")};
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->data.channels.size(), 1U);
    EXPECT_NEAR(result->data.channels.front().values.front(), 25.0, 1e-9);
}

TEST(TestCsvSeries, ImportRejectsHeaderOnlyContent)
{
    const auto result{Csv::readFromString("Air Temperature\n", "empty")};
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "empty has no data rows");
}

TEST(TestCsvSeries, ImportRejectsContentWithoutRoleColumns)
{
    const std::string content{"Time,Comfort Index\n"
                              "2026-01-01 00:00,5\n"};

    const auto result{Csv::readFromString(content, "unrelated")};
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "unrelated has no column named after a time series quantity");
}

TEST(TestCsvSeries, ExportRoundTripsThroughImport)
{
    TimeSeriesData data;
    data.Name = "round trip";
    data.channels = {Channel{ChannelRole::AirTemperature, {21.3, 21.1, 20.8}},
                     Channel{ChannelRole::PrescribedHumidity, {0.52, 0.55, 0.57}}};

    const auto content{Csv::writeToString(data)};
    EXPECT_NE(content.find("Time,Air Temperature,Prescribed Humidity"), std::string::npos);

    const auto result{Csv::readFromString(content, "round trip")};
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->hadTimeColumn);
    ASSERT_EQ(result->data.channels.size(), 2U);
    EXPECT_EQ(result->data.channels[0].role, ChannelRole::AirTemperature);
    EXPECT_EQ(result->data.channels[1].role, ChannelRole::PrescribedHumidity);
    for(size_t index = 0; index < 3U; ++index)
    {
        EXPECT_NEAR(result->data.channels[0].values[index],
                    data.channels[0].values[index],
                    1e-9);
        EXPECT_NEAR(result->data.channels[1].values[index],
                    data.channels[1].values[index],
                    1e-9);
    }
}

TEST(TestCsvSeries, ImportHandlesQuotedCellsWithSeparators)
{
    const std::string content{"\"Air Temperature\",\"Ignore, me\"\n"
                              "21.3,\"1,000\"\n"};

    const auto result{Csv::readFromString(content, "quoted")};
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->ignoredHeaders.size(), 1U);
    EXPECT_EQ(result->ignoredHeaders.front(), "Ignore, me");

    const auto values{
      TimeSeriesLibrary::valuesForRole(result->data, ChannelRole::AirTemperature)};
    ASSERT_TRUE(values.has_value());
    EXPECT_NEAR(values->front(), 21.3, 1e-9);
}
