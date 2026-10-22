#include <gtest/gtest.h>

#include "TimeSeriesData/ContentHash.hxx"
#include "TimeSeriesData/CsvSeries.hxx"

using TimeSeriesLibrary::Series;
using TimeSeriesLibrary::SeriesRole;
using TimeSeriesLibrary::TimeAxis;
using TimeSeriesLibrary::TimeSeriesData;

namespace Csv = TimeSeriesLibrary::Csv;

namespace
{
    constexpr Csv::StepColumn byTime{Csv::StepColumn::Time};
    constexpr Csv::StepColumn byTimestep{Csv::StepColumn::Timestep};
    constexpr Csv::FixedStep hourly{.seconds = 3600.0};

    std::vector<double> temperatures(const Csv::ImportResult & result)
    {
        return TimeSeriesLibrary::valuesForRole(result.data, SeriesRole::AirTemperature).value();
    }
}   // namespace

TEST(TestCsvSeries, ImportAcceptsBothRoleSpellings)
{
    const std::string content{"Time,Air Temperature,RelativeHumidity,Comfort Index\n"
                              "2026-01-01 00:00,21.3,0.52,5\n"
                              "2026-01-01 01:00,21.1,0.55,6\n"};

    const auto result{Csv::readFromString(content, "sample", hourly)};
    ASSERT_TRUE(result.has_value());

    const auto & imported{result->data};
    EXPECT_EQ(imported.Name, "sample");
    EXPECT_TRUE(result->hadTimeColumn);
    EXPECT_FALSE(result->hadTimestepColumn);
    EXPECT_FALSE(result->firstRowWasStart);
    ASSERT_EQ(result->ignoredHeaders.size(), 1U);
    EXPECT_EQ(result->ignoredHeaders.front(), "Comfort Index");

    EXPECT_EQ(TimeSeriesLibrary::steps(imported), 2U);
    EXPECT_NEAR(temperatures(result.value()).at(0), 21.3, 1e-9);
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

    const auto result{Csv::readFromString(content, "european", hourly)};
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->hadTimeColumn);
    EXPECT_EQ(result->data.axis, TimeAxis{});

    EXPECT_NEAR(temperatures(result.value()).at(0), 21.5, 1e-9);
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

    const auto result{Csv::readFromString(content, "gaps", hourly)};
    ASSERT_TRUE(result.has_value());

    const auto values{temperatures(result.value())};
    ASSERT_EQ(values.size(), 4U);
    EXPECT_NEAR(values.at(0), 20.0, 1e-9);   // leading gap takes the first good reading
    EXPECT_NEAR(values.at(2), 20.0, 1e-9);   // mid-run gap carries the previous reading
    EXPECT_NEAR(values.at(3), 22.0, 1e-9);
}

TEST(TestCsvSeries, ImportKeepsLastColumnOnDuplicateRole)
{
    const std::string content{"Air Temperature,AirTemperature\n"
                              "20,25\n"};

    const auto result{Csv::readFromString(content, "duplicate", hourly)};
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->data.series.size(), 1U);
    EXPECT_NEAR(result->data.series.front().values.front(), 25.0, 1e-9);
}

TEST(TestCsvSeries, ImportRejectsHeaderOnlyContent)
{
    const auto result{Csv::readFromString("Air Temperature\n", "empty", hourly)};
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "empty has no data rows");
}

TEST(TestCsvSeries, ImportRejectsContentWithoutRoleColumns)
{
    const std::string content{"Time,Comfort Index\n"
                              "2026-01-01 00:00,5\n"};

    const auto result{Csv::readFromString(content, "unrelated", byTime)};
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "unrelated has no column named after a time series quantity");
}

TEST(TestCsvSeries, ExportRoundTripsThroughImportByTimestep)
{
    TimeSeriesData data;
    data.Name = "round trip";
    data.series = {Series{SeriesRole::AirTemperature, {21.3, 21.1, 20.8}},
                     Series{SeriesRole::PrescribedHumidity, {0.52, 0.55, 0.57}}};

    const auto content{Csv::writeToString(data)};
    EXPECT_NE(content.find("Time,Timestep,Air Temperature,Prescribed Humidity"), std::string::npos);

    const auto result{Csv::readFromString(content, "round trip", byTimestep)};
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(result->hadTimeColumn);
    EXPECT_TRUE(result->hadTimestepColumn);
    EXPECT_EQ(result->data.axis, data.axis);
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

TEST(TestCsvSeries, ExportReadBackByTimeIsOneStepShorter)
{
    TimeSeriesData data;
    data.series = {Series{SeriesRole::AirTemperature, {21.3, 21.1, 20.8}}};

    const auto result{Csv::readFromString(Csv::writeToString(data), "shorter", byTime)};
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(result->firstRowWasStart);
    EXPECT_EQ(temperatures(result.value()), (std::vector<double>{21.1, 20.8}));
    // The first stamp, the end of the first step, becomes the start.
    EXPECT_EQ(result->data.axis, (TimeAxis{.month = 1U, .day = 1U, .hour = 1U}));
}

TEST(TestCsvSeries, TimeRuleEndsAStepAtEveryStamp)
{
    const std::string content{"Time,Air Temperature\n"
                              "2026-03-15 06:30,1\n"
                              "2026-03-15 06:40,2\n"
                              "2026-03-15T06:50:00,3\n"};

    const auto result{Csv::readFromString(content, "ten minutes", byTime)};
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(result->hadTimeColumn);
    EXPECT_TRUE(result->firstRowWasStart);
    const TimeAxis expected{.month = 3U, .day = 15U, .hour = 6U, .minute = 30U, .stepSeconds = 600.0};
    EXPECT_EQ(result->data.axis, expected);
    EXPECT_EQ(temperatures(result.value()), (std::vector<double>{2.0, 3.0}));
}

TEST(TestCsvSeries, ImportReadsUsAndEuropeanDateOrders)
{
    const std::string american{"Date,Air Temperature\n"
                               "1/31/2026 23:00,1\n"
                               "2/1/2026 0:00,2\n"};
    const auto usResult{Csv::readFromString(american, "us", byTime)};
    ASSERT_TRUE(usResult.has_value());
    EXPECT_EQ(usResult->data.axis, (TimeAxis{.month = 1U, .day = 31U, .hour = 23U}));

    const std::string european{"Timestamp;Air Temperature\n"
                               "31.1.2026 23:00;1\n"
                               "1.2.2026 00:00;2\n"};
    const auto euResult{Csv::readFromString(european, "eu", byTime)};
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

    const auto result{Csv::readFromString(content, "new year", byTime)};
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->data.axis, (TimeAxis{.month = 12U, .day = 31U, .hour = 23U}));
}

TEST(TestCsvSeries, UnevenTimesBecomeStepLengths)
{
    const std::string content{"Time,Air Temperature\n"
                              "2026-01-01 00:00,1\n"
                              "2026-01-01 01:00,2\n"
                              "2026-01-01 03:00,3\n"};

    const auto result{Csv::readFromString(content, "uneven", byTime)};
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_EQ(result->data.axis, (TimeAxis{.stepSeconds = 3600.0, .stepLengths = {3600.0, 7200.0}}));
    EXPECT_EQ(temperatures(result.value()), (std::vector<double>{2.0, 3.0}));
    EXPECT_EQ(TimeSeriesLibrary::describeAxis(result->data), "1 Jan 00:00, steps 1 h to 2 h, 2 rows");
}

TEST(TestCsvSeries, TimeRuleRejectsTimesOutOfOrderOrTooFew)
{
    const std::string repeated{"Time,Air Temperature\n"
                               "2026-01-01 00:00,1\n"
                               "2026-01-01 01:00,2\n"
                               "2026-01-01 01:00,3\n"};
    const auto duplicate{Csv::readFromString(repeated, "dup", byTime)};
    ASSERT_FALSE(duplicate.has_value());
    EXPECT_EQ(duplicate.error(), "dup data row 3 does not come after data row 2");

    const auto single{Csv::readFromString("Time,Air Temperature\n2026-01-01 00:00,1\n", "one", byTime)};
    ASSERT_FALSE(single.has_value());
    EXPECT_NE(single.error().find("needs at least two rows with times"), std::string::npos);
}

TEST(TestCsvSeries, ImportRejectsUnreadableTime)
{
    const std::string content{"Time,Air Temperature\n"
                              "2026-01-01 00:00,1\n"
                              "yesterday,2\n"};

    const auto result{Csv::readFromString(content, "vague", byTime)};
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "vague data row 2: cannot read the time 'yesterday'");
}

TEST(TestCsvSeries, TimestepColumnStatesEveryStep)
{
    const std::string content{"Timestep,Air Temperature\n"
                              "900,1\n"
                              "900,2\n"
                              "1800,3\n"};

    const auto result{Csv::readFromString(content, "logger", byTimestep)};
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(result->hadTimestepColumn);
    EXPECT_FALSE(result->hadTimeColumn);
    EXPECT_FALSE(result->firstRowWasStart);
    EXPECT_EQ(result->data.axis, (TimeAxis{.stepSeconds = 900.0, .stepLengths = {900.0, 900.0, 1800.0}}));
    EXPECT_EQ(temperatures(result.value()), (std::vector<double>{1.0, 2.0, 3.0}));
    EXPECT_EQ(TimeSeriesLibrary::stepLabel(result->data.axis), "15 min to 30 min");
}

TEST(TestCsvSeries, TimestepColumnWithTimesStartsOneStepBeforeTheFirstStamp)
{
    const std::string content{"Time,dt,Air Temperature\n"
                              "2026-01-01 01:00,3600,1\n"
                              "2026-01-01 02:00,3600,2\n"};

    const auto result{Csv::readFromString(content, "stamped", byTimestep)};
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_EQ(result->data.axis, TimeAxis{});
    EXPECT_EQ(TimeSeriesLibrary::steps(result->data), 2U);
}

TEST(TestCsvSeries, TimestepColumnRejectsNonPositiveOrUnreadableLengths)
{
    const auto zero{Csv::readFromString("Step,Air Temperature\n0,1\n", "zero", byTimestep)};
    ASSERT_FALSE(zero.has_value());
    EXPECT_EQ(zero.error(), "zero data row 1: the timestep must be positive, got 0");

    const auto words{Csv::readFromString("Step Seconds,Air Temperature\n900,1\nlater,2\n", "words", byTimestep)};
    ASSERT_FALSE(words.has_value());
    EXPECT_EQ(words.error(), "words data row 2: cannot read the timestep 'later'");
}

TEST(TestCsvSeries, AColumnTheChoiceAsksForMustExist)
{
    const std::string content{"Air Temperature\n1\n2\n"};

    const auto noTime{Csv::readFromString(content, "bare", byTime)};
    ASSERT_FALSE(noTime.has_value());
    EXPECT_EQ(noTime.error(), "bare has no Time column (Time, Date, Datetime or Timestamp)");

    const auto noStep{Csv::readFromString(content, "bare", byTimestep)};
    ASSERT_FALSE(noStep.has_value());
    EXPECT_EQ(noStep.error(), "bare has no Timestep column (Timestep, Step, Step Seconds or dt)");
}

TEST(TestCsvSeries, FixedStepMakesEveryRowAStep)
{
    const std::string content{"Time,Air Temperature\n"
                              "2026-07-04 12:30,1\n"
                              "2026-07-04 13:00,2\n"};

    const auto quarterly{Csv::readFromString(content, "fixed", Csv::FixedStep{.seconds = 900.0})};
    ASSERT_TRUE(quarterly.has_value()) << quarterly.error();
    // The Time column only places the start, one step before the first stamp.
    EXPECT_EQ(quarterly->data.axis, (TimeAxis{.month = 7U, .day = 4U, .hour = 12U, .minute = 15U, .stepSeconds = 900.0}));
    EXPECT_EQ(temperatures(quarterly.value()), (std::vector<double>{1.0, 2.0}));

    const auto refused{Csv::readFromString(content, "fixed", Csv::FixedStep{.seconds = 0.0})};
    ASSERT_FALSE(refused.has_value());
    EXPECT_EQ(refused.error(), "fixed: the fixed step must be positive, got 0");
}

TEST(TestCsvSeries, ExportStampsTheEndOfEachStep)
{
    TimeSeriesData data;
    data.Name = "summer";
    data.axis = TimeAxis{.month = 7U, .day = 4U, .hour = 12U, .minute = 0U, .stepSeconds = 1800.0};
    data.series = {Series{SeriesRole::AirTemperature, {30.0, 31.0, 32.0}}};

    const auto content{Csv::writeToString(data)};
    EXPECT_NE(content.find("\n2026-07-04 12:30,1800,30"), std::string::npos);
    EXPECT_NE(content.find("\n2026-07-04 13:00,1800,31"), std::string::npos);

    const auto result{Csv::readFromString(content, "summer", byTimestep)};
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_EQ(result->data.axis, data.axis);
}

TEST(TestCsvSeries, ExportWritesUnevenLengthsAndReadsThemBack)
{
    TimeSeriesData data;
    data.axis = TimeAxis{.month = 7U, .day = 4U, .hour = 12U, .stepSeconds = 900.0, .stepLengths = {900.0, 1800.0, 3600.0}};
    data.series = {Series{SeriesRole::AirTemperature, {30.0, 31.0, 32.0}}};

    const auto content{Csv::writeToString(data)};
    EXPECT_NE(content.find("\n2026-07-04 12:15,900,30"), std::string::npos);
    EXPECT_NE(content.find("\n2026-07-04 12:45,1800,31"), std::string::npos);
    EXPECT_NE(content.find("\n2026-07-04 13:45,3600,32"), std::string::npos);

    const auto exact{Csv::readFromString(content, "uneven", byTimestep)};
    ASSERT_TRUE(exact.has_value()) << exact.error();
    EXPECT_EQ(exact->data.axis, data.axis);
    EXPECT_EQ(exact->data.series, data.series);

    const auto shorter{Csv::readFromString(content, "uneven", byTime)};
    ASSERT_TRUE(shorter.has_value()) << shorter.error();
    EXPECT_EQ(shorter->data.axis,
              (TimeAxis{.month = 7U, .day = 4U, .hour = 12U, .minute = 15U, .stepSeconds = 1800.0, .stepLengths = {1800.0, 3600.0}}));
    EXPECT_EQ(temperatures(shorter.value()), (std::vector<double>{31.0, 32.0}));
}

TEST(TestCsvSeries, ImportHandlesQuotedCellsWithSeparators)
{
    const std::string content{"\"Air Temperature\",\"Ignore, me\"\n"
                              "21.3,\"1,000\"\n"};

    const auto result{Csv::readFromString(content, "quoted", hourly)};
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->ignoredHeaders.size(), 1U);
    EXPECT_EQ(result->ignoredHeaders.front(), "Ignore, me");
    EXPECT_NEAR(temperatures(result.value()).front(), 21.3, 1e-9);
}
