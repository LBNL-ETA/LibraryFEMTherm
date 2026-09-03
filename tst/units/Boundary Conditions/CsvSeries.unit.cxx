#include <gtest/gtest.h>

#include "TimeSeriesData/ContentHash.hxx"
#include "TimeSeriesData/CsvSeries.hxx"

using TimeSeriesLibrary::Series;
using TimeSeriesLibrary::SeriesRole;
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
      TimeSeriesLibrary::valuesForRole(imported, SeriesRole::AirTemperature)};
    ASSERT_TRUE(temperature.has_value());
    EXPECT_NEAR(temperature->at(0), 21.3, 1e-9);
    const auto humidity{
      TimeSeriesLibrary::valuesForRole(imported, SeriesRole::RelativeHumidity)};
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
      TimeSeriesLibrary::valuesForRole(result->data, SeriesRole::AirTemperature)};
    ASSERT_TRUE(temperature.has_value());
    EXPECT_NEAR(temperature->at(0), 21.5, 1e-9);
    const auto wind{TimeSeriesLibrary::valuesForRole(result->data, SeriesRole::WindSpeed)};
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
      TimeSeriesLibrary::valuesForRole(result->data, SeriesRole::AirTemperature)};
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
    ASSERT_EQ(result->data.series.size(), 1U);
    EXPECT_NEAR(result->data.series.front().values.front(), 25.0, 1e-9);
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
    data.series = {Series{SeriesRole::AirTemperature, {21.3, 21.1, 20.8}},
                     Series{SeriesRole::PrescribedHumidity, {0.52, 0.55, 0.57}}};

    const auto content{Csv::writeToString(data)};
    EXPECT_NE(content.find("Time,Air Temperature,Prescribed Humidity"), std::string::npos);

    const auto result{Csv::readFromString(content, "round trip")};
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->hadTimeColumn);
    ASSERT_EQ(result->data.series.size(), 2U);
    EXPECT_EQ(result->data.series[0].role, SeriesRole::AirTemperature);
    EXPECT_EQ(result->data.series[1].role, SeriesRole::PrescribedHumidity);
    for(size_t index = 0; index < 3U; ++index)
    {
        EXPECT_NEAR(result->data.series[0].values[index],
                    data.series[0].values[index],
                    1e-9);
        EXPECT_NEAR(result->data.series[1].values[index],
                    data.series[1].values[index],
                    1e-9);
    }
}

TEST(TestCsvSeries, ImportReadsTimeAxisFromIsoTimeColumn)
{
    const std::string content{"Time,Air Temperature\n"
                              "2026-03-15 06:30,1\n"
                              "2026-03-15 06:40,2\n"
                              "2026-03-15T06:50:00,3\n"};

    const auto result{Csv::readFromString(content, "ten minutes")};
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->hadTimeColumn);
    const TimeSeriesLibrary::TimeAxis expected{
      .month = 3U, .day = 15U, .hour = 6U, .minute = 30U, .stepSeconds = 600.0};
    EXPECT_EQ(result->data.axis, expected);
}

TEST(TestCsvSeries, ImportReadsUsAndEuropeanDateOrders)
{
    const std::string american{"Date,Air Temperature\n"
                               "1/31/2026 23:00,1\n"
                               "2/1/2026 0:00,2\n"};
    const auto usResult{Csv::readFromString(american, "us")};
    ASSERT_TRUE(usResult.has_value());
    EXPECT_EQ(usResult->data.axis,
              (TimeSeriesLibrary::TimeAxis{.month = 1U, .day = 31U, .hour = 23U}));

    const std::string european{"Timestamp;Air Temperature\n"
                               "31.1.2026 23:00;1\n"
                               "1.2.2026 00:00;2\n"};
    const auto euResult{Csv::readFromString(european, "eu")};
    ASSERT_TRUE(euResult.has_value());
    EXPECT_EQ(euResult->data.axis, usResult->data.axis);
}

TEST(TestCsvSeries, ImportKeepsSpacingAcrossNewYear)
{
    // Years are dropped; the run must still read as one continuous hourly sequence.
    const std::string content{"Time,Air Temperature\n"
                              "2025-12-31 23:00,1\n"
                              "2026-01-01 00:00,2\n"
                              "2026-01-01 01:00,3\n"};

    const auto result{Csv::readFromString(content, "new year")};
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->data.axis,
              (TimeSeriesLibrary::TimeAxis{.month = 12U, .day = 31U, .hour = 23U}));
}

TEST(TestCsvSeries, ImportRejectsUnevenSpacing)
{
    const std::string content{"Time,Air Temperature\n"
                              "2026-01-01 00:00,1\n"
                              "2026-01-01 01:00,2\n"
                              "2026-01-01 03:00,3\n"};

    const auto result{Csv::readFromString(content, "uneven")};
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(),
              "uneven rows are not evenly spaced: data row 3 is 7200 s after the previous "
              "one, expected 3600 s");
}

TEST(TestCsvSeries, ImportRejectsUnreadableTime)
{
    const std::string content{"Time,Air Temperature\n"
                              "2026-01-01 00:00,1\n"
                              "yesterday,2\n"};

    const auto result{Csv::readFromString(content, "vague")};
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "vague data row 2: cannot read the time 'yesterday'");
}

TEST(TestCsvSeries, ExportRoundTripsAxis)
{
    TimeSeriesData data;
    data.Name = "summer";
    data.axis = TimeSeriesLibrary::TimeAxis{
      .month = 7U, .day = 4U, .hour = 12U, .minute = 0U, .stepSeconds = 1800.0};
    data.series = {Series{SeriesRole::AirTemperature, {30.0, 31.0, 32.0}}};

    const auto content{Csv::writeToString(data)};
    EXPECT_NE(content.find("\n2026-07-04 12:00,30"), std::string::npos);
    EXPECT_NE(content.find("\n2026-07-04 12:30,31"), std::string::npos);

    const auto result{Csv::readFromString(content, "summer")};
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->data.axis, data.axis);
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
      TimeSeriesLibrary::valuesForRole(result->data, SeriesRole::AirTemperature)};
    ASSERT_TRUE(values.has_value());
    EXPECT_NEAR(values->front(), 21.3, 1e-9);
}
