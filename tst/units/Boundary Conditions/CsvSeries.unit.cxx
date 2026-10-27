#include <algorithm>

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
    EXPECT_EQ(result->firstStep, Csv::FirstStep::Stated);
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

TEST(TestCsvSeries, ExportReadsBackTheSameByTime)
{
    TimeSeriesData data;
    data.series = {Series{SeriesRole::AirTemperature, {21.3, 21.1, 20.8}}};

    // The exported Timestep column agrees with the stamps, so it supplies the first length.
    const auto result{Csv::readFromString(Csv::writeToString(data), "same", byTime)};
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_EQ(result->firstStep, Csv::FirstStep::FromColumn);
    EXPECT_EQ(temperatures(result.value()), (std::vector<double>{21.3, 21.1, 20.8}));
    EXPECT_EQ(result->data.axis, data.axis);
}

TEST(TestCsvSeries, WithoutAChoiceTheColumnsDecide)
{
    const std::string both{"Time,Timestep,Air Temperature\n"
                           "2026-01-01 00:15,900,1\n"
                           "2026-01-01 00:30,900,2\n"
                           "2026-01-01 01:30,3600,3\n"};
    const auto byDefault{Csv::readFromString(both, "both")};
    const auto byTime{Csv::readFromString(both, "both", Csv::StepColumn::Time)};
    ASSERT_TRUE(byDefault.has_value()) << byDefault.error();
    ASSERT_TRUE(byTime.has_value()) << byTime.error();
    EXPECT_EQ(byDefault->data.axis, byTime->data.axis);
    EXPECT_EQ(byDefault->firstStep, Csv::FirstStep::FromColumn);
    // The column agrees with the stamps, so the three readings are one dataset.
    const auto byColumn{Csv::readFromString(both, "both", byTimestep)};
    ASSERT_TRUE(byColumn.has_value()) << byColumn.error();
    EXPECT_EQ(byTime->data, byColumn->data);

    const auto timestepOnly{Csv::readFromString("Timestep,Air Temperature\n900,1\n1800,2\n", "timestep")};
    ASSERT_TRUE(timestepOnly.has_value()) << timestepOnly.error();
    EXPECT_EQ(timestepOnly->data.axis.stepSeconds, (TimeSeriesLibrary::StepSeconds{std::vector<double>{900.0, 1800.0}}));

    const auto neither{Csv::readFromString("Air Temperature\n1\n2\n3\n", "neither")};
    ASSERT_TRUE(neither.has_value()) << neither.error();
    EXPECT_EQ(neither->data.axis, TimeAxis{});
    EXPECT_EQ(temperatures(neither.value()), (std::vector<double>{1.0, 2.0, 3.0}));
}

TEST(TestCsvSeries, DescribeRolesIsAnAlignedTable)
{
    const auto table{Csv::describeRoles()};
    // The widest role name, ConvectiveCoefficient, has 21 letters; the columns sit two apart.
    EXPECT_TRUE(table.starts_with("Role                   CSV header\n"
                                  "AirTemperature         Air Temperature\n"));
    EXPECT_EQ(std::ranges::count(table, '\n'), 12);
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
    // Stamps alone: the first step is taken as long as the gap to the second row, so the
    // run starts at 06:20, hour 7 of the day.
    EXPECT_EQ(result->firstStep, Csv::FirstStep::Assumed);
    const TimeAxis expected{.month = 3U, .day = 15U, .hour = 7U, .minute = 20U, .stepSeconds = 600.0};
    EXPECT_EQ(result->data.axis, expected);
    EXPECT_EQ(temperatures(result.value()), (std::vector<double>{1.0, 2.0, 3.0}));
    EXPECT_EQ(Csv::describeFirstStep(result.value()), "first step 10 min, assumed from the gap to the second row");
}

TEST(TestCsvSeries, ImportReadsUsAndEuropeanDateOrders)
{
    const std::string american{"Date,Air Temperature\n"
                               "1/31/2026 23:00,1\n"
                               "2/1/2026 0:00,2\n"};
    const auto usResult{Csv::readFromString(american, "us", byTime)};
    ASSERT_TRUE(usResult.has_value());
    // Two hourly steps; the run starts an hour before the first stamp, at 22:00.
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
    EXPECT_EQ(temperatures(result.value()), (std::vector<double>{1.0, 2.0, 3.0}));
}

TEST(TestCsvSeries, UnevenTimesBecomeStepLengths)
{
    const std::string content{"Time,Air Temperature\n"
                              "2026-01-01 00:00,1\n"
                              "2026-01-01 01:00,2\n"
                              "2026-01-01 03:00,3\n"};

    const auto result{Csv::readFromString(content, "uneven", byTime)};
    ASSERT_TRUE(result.has_value()) << result.error();
    // The assumed first step is an hour, the gap to the second row; the run starts at 23:00 the day before.
    EXPECT_EQ(result->data.axis,
              (TimeAxis{.month = 12U, .day = 31U, .hour = 24U, .stepSeconds = std::vector<double>{3600.0, 3600.0, 7200.0}}));
    EXPECT_EQ(temperatures(result.value()), (std::vector<double>{1.0, 2.0, 3.0}));
    EXPECT_EQ(TimeSeriesLibrary::describeAxis(result->data), "31 Dec 23:00, steps 1 h to 2 h, 3 rows");
}

TEST(TestCsvSeries, AStatedStartSetsTheFirstStep)
{
    const std::string content{"Time,Air Temperature\n"
                              "2026-01-01 00:30,1\n"
                              "2026-01-01 01:00,2\n"};

    const auto stated{Csv::readFromString(content, "stated", byTime, "2026-01-01 00:00")};
    ASSERT_TRUE(stated.has_value()) << stated.error();
    EXPECT_EQ(stated->firstStep, Csv::FirstStep::Stated);
    EXPECT_EQ(stated->data.axis, (TimeAxis{.stepSeconds = 1800.0}));
    EXPECT_EQ(Csv::describeFirstStep(stated.value()), "first step 30 min, as stated");

    // Without it the first step is assumed as long as the second; with it, the gap is exact.
    const auto assumed{Csv::readFromString(content, "assumed", byTime)};
    ASSERT_TRUE(assumed.has_value()) << assumed.error();
    EXPECT_EQ(assumed->data.axis, stated->data.axis);
    EXPECT_EQ(assumed->firstStep, Csv::FirstStep::Assumed);
    const auto shorter{Csv::readFromString(content, "shorter", byTime, "2026-01-01 00:10")};
    ASSERT_TRUE(shorter.has_value()) << shorter.error();
    EXPECT_EQ(shorter->data.axis,
              (TimeAxis{.month = 1U, .day = 1U, .hour = 1U, .minute = 10U, .stepSeconds = std::vector<double>{1200.0, 1800.0}}));

    // The year is dropped, so a start later in the year than the first stamp is the year before.
    const auto wrapped{Csv::readFromString(content, "wrapped", byTime, "2025-12-31 23:00")};
    ASSERT_TRUE(wrapped.has_value()) << wrapped.error();
    EXPECT_EQ(wrapped->data.axis,
              (TimeAxis{.month = 12U, .day = 31U, .hour = 24U, .stepSeconds = std::vector<double>{5400.0, 1800.0}}));

    const auto empty{Csv::readFromString(content, "empty", byTime, "2026-01-01 00:30")};
    ASSERT_FALSE(empty.has_value());
    EXPECT_EQ(empty.error(), "empty: the start '2026-01-01 00:30' is the first time '2026-01-01 00:30', which leaves no first step");
    const auto vague{Csv::readFromString(content, "vague", byTime, "dawn")};
    ASSERT_FALSE(vague.has_value());
    EXPECT_EQ(vague.error(), "vague: cannot read the start 'dawn'");
}

TEST(TestCsvSeries, ATimestepColumnThatContradictsTheStampsIsNotTrusted)
{
    const std::string content{"Time,Timestep,Air Temperature\n"
                              "2026-01-01 00:15,3600,1\n"
                              "2026-01-01 00:30,3600,2\n"
                              "2026-01-01 00:45,3600,3\n"};

    const auto byStamps{Csv::readFromString(content, "stamps", byTime)};
    ASSERT_TRUE(byStamps.has_value()) << byStamps.error();
    EXPECT_EQ(byStamps->firstStep, Csv::FirstStep::Assumed);
    EXPECT_EQ(byStamps->data.axis, (TimeAxis{.stepSeconds = 900.0}));

    const auto byColumn{Csv::readFromString(content, "column", byTimestep)};
    ASSERT_TRUE(byColumn.has_value()) << byColumn.error();
    EXPECT_EQ(byColumn->firstStep, Csv::FirstStep::FromColumn);
    EXPECT_EQ(byColumn->data.axis, (TimeAxis{.month = 12U, .day = 31U, .hour = 24U, .minute = 15U, .stepSeconds = 3600.0}));
}

TEST(TestCsvSeries, AStartPlacesARunWithoutStamps)
{
    const auto placed{Csv::readFromString("Timestep,Air Temperature\n900,1\n900,2\n", "placed", byTimestep, "2026-02-01 06:00")};
    ASSERT_TRUE(placed.has_value()) << placed.error();
    EXPECT_EQ(placed->data.axis, (TimeAxis{.month = 2U, .day = 1U, .hour = 7U, .stepSeconds = 900.0}));
    EXPECT_EQ(placed->firstStep, Csv::FirstStep::FromColumn);

    const auto fixed{Csv::readFromString("Air Temperature\n1\n2\n", "fixed", hourly, "2026-02-01 06:00")};
    ASSERT_TRUE(fixed.has_value()) << fixed.error();
    EXPECT_EQ(fixed->data.axis, (TimeAxis{.month = 2U, .day = 1U, .hour = 7U}));

    // Beside a Time column these rules place the start from the first stamp and do not read it.
    const std::string stamped{"Time,dt,Air Temperature\n2026-01-01 01:00,3600,1\n2026-01-01 02:00,3600,2\n"};
    const auto ignored{Csv::readFromString(stamped, "stamped", byTimestep, "2026-02-01 06:00")};
    ASSERT_TRUE(ignored.has_value()) << ignored.error();
    EXPECT_EQ(ignored->data.axis, Csv::readFromString(stamped, "stamped", byTimestep)->data.axis);
}

TEST(TestCsvSeries, TimeRuleRejectsTimesOutOfOrderOrASingleRowWithoutAStart)
{
    const std::string repeated{"Time,Air Temperature\n"
                               "2026-01-01 00:00,1\n"
                               "2026-01-01 01:00,2\n"
                               "2026-01-01 01:00,3\n"};
    const auto duplicate{Csv::readFromString(repeated, "dup", byTime)};
    ASSERT_FALSE(duplicate.has_value());
    EXPECT_EQ(duplicate.error(), "dup data row 3 does not come after data row 2");

    const std::string oneRow{"Time,Air Temperature\n2026-01-01 01:00,1\n"};
    const auto single{Csv::readFromString(oneRow, "one", byTime)};
    ASSERT_FALSE(single.has_value());
    EXPECT_EQ(single.error(), "one has one row with a time and no start: how long its step lasts cannot be known");
    const auto placed{Csv::readFromString(oneRow, "one", byTime, "2026-01-01 00:00")};
    ASSERT_TRUE(placed.has_value()) << placed.error();
    EXPECT_EQ(placed->data.axis, TimeAxis{});
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
    EXPECT_EQ(result->firstStep, Csv::FirstStep::FromColumn);
    EXPECT_EQ(Csv::describeFirstStep(result.value()), "first step 15 min, from the Timestep column");
    EXPECT_EQ(result->data.axis, (TimeAxis{.stepSeconds = std::vector<double>{900.0, 900.0, 1800.0}}));
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
    EXPECT_EQ(quarterly->data.axis, (TimeAxis{.month = 7U, .day = 4U, .hour = 13U, .minute = 15U, .stepSeconds = 900.0}));
    EXPECT_EQ(temperatures(quarterly.value()), (std::vector<double>{1.0, 2.0}));

    const auto refused{Csv::readFromString(content, "fixed", Csv::FixedStep{.seconds = 0.0})};
    ASSERT_FALSE(refused.has_value());
    EXPECT_EQ(refused.error(), "fixed: the fixed step must be positive, got 0");
}

TEST(TestCsvSeries, ExportStampsTheEndOfEachStep)
{
    TimeSeriesData data;
    data.Name = "summer";
    data.axis = TimeAxis{.month = 7U, .day = 4U, .hour = 13U, .minute = 0U, .stepSeconds = 1800.0};
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
    data.axis = TimeAxis{.month = 7U, .day = 4U, .hour = 13U, .stepSeconds = std::vector<double>{900.0, 1800.0, 3600.0}};
    data.series = {Series{SeriesRole::AirTemperature, {30.0, 31.0, 32.0}}};

    const auto content{Csv::writeToString(data)};
    EXPECT_NE(content.find("\n2026-07-04 12:15,900,30"), std::string::npos);
    EXPECT_NE(content.find("\n2026-07-04 12:45,1800,31"), std::string::npos);
    EXPECT_NE(content.find("\n2026-07-04 13:45,3600,32"), std::string::npos);

    const auto exact{Csv::readFromString(content, "uneven", byTimestep)};
    ASSERT_TRUE(exact.has_value()) << exact.error();
    EXPECT_EQ(exact->data.axis, data.axis);
    EXPECT_EQ(exact->data.series, data.series);

    const auto byStamps{Csv::readFromString(content, "uneven", byTime)};
    ASSERT_TRUE(byStamps.has_value()) << byStamps.error();
    EXPECT_EQ(byStamps->firstStep, Csv::FirstStep::FromColumn);
    EXPECT_EQ(byStamps->data.axis, data.axis);
    EXPECT_EQ(byStamps->data.series, data.series);
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
