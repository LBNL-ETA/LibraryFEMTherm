#include <gtest/gtest.h>

#include "TimeSeriesData/ContentHash.hxx"
#include "TimeSeriesData/Window.hxx"

using TimeSeriesLibrary::Series;
using TimeSeriesLibrary::SeriesRole;
using TimeSeriesLibrary::TimeAxis;
using TimeSeriesLibrary::TimeSeriesData;

namespace
{
    std::vector<double> counting(const std::size_t count)
    {
        std::vector<double> values(count);
        for(std::size_t index = 0U; index < count; ++index)
        {
            values[index] = static_cast<double>(index);
        }
        return values;
    }

    TimeSeriesData hourlyFromNewYear(const std::size_t rows)
    {
        TimeSeriesData data{.UUID = "original", .Name = "sample", .series = {{.role = SeriesRole::AirTemperature, .values = counting(rows)}}};
        return data;
    }
}   // namespace

TEST(TestWindow, AxisAfterMovesThroughTheYear)
{
    const TimeAxis hourly{};
    EXPECT_EQ(TimeSeriesLibrary::axisAfter(hourly, 0U), hourly);
    EXPECT_EQ(TimeSeriesLibrary::axisAfter(hourly, 24U), (TimeAxis{.month = 1U, .day = 2U, .hour = 0U, .minute = 0U}));
    EXPECT_EQ(TimeSeriesLibrary::axisAfter(hourly, 31U * 24U + 5U), (TimeAxis{.month = 2U, .day = 1U, .hour = 5U, .minute = 0U}));

    const TimeAxis halfHourly{.month = 12U, .day = 31U, .hour = 23U, .minute = 0U, .stepSeconds = 1800.0};
    EXPECT_EQ(TimeSeriesLibrary::axisAfter(halfHourly, 1U), (TimeAxis{.month = 12U, .day = 31U, .hour = 23U, .minute = 30U, .stepSeconds = 1800.0}));
    // The nominal year wraps: two half hours past 23:00 on 31 December is the next 1 January.
    EXPECT_EQ(TimeSeriesLibrary::axisAfter(halfHourly, 2U), (TimeAxis{.month = 1U, .day = 1U, .hour = 0U, .minute = 0U, .stepSeconds = 1800.0}));
}

TEST(TestWindow, WindowKeepsTheRowsAskedForAndShiftsTheAxis)
{
    const auto result{TimeSeriesLibrary::window(hourlyFromNewYear(72U), 24U, 24U)};
    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_EQ(result->axis, (TimeAxis{.month = 1U, .day = 2U}));
    EXPECT_EQ(TimeSeriesLibrary::steps(result.value()), 24U);
    const auto values{TimeSeriesLibrary::valuesForRole(result.value(), SeriesRole::AirTemperature)};
    ASSERT_TRUE(values.has_value());
    EXPECT_NEAR(values->front(), 24.0, 1e-12);
    EXPECT_NEAR(values->back(), 47.0, 1e-12);
    EXPECT_EQ(result->Name, "sample");
}

TEST(TestWindow, WindowUuidIsItsContent)
{
    const auto result{TimeSeriesLibrary::window(hourlyFromNewYear(72U), 24U, 24U)};
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->UUID, TimeSeriesLibrary::contentUuid(result.value()));
    EXPECT_NE(result->UUID, "original");
    // The whole dataset is its own window; only the UUID is recomputed.
    const auto whole{TimeSeriesLibrary::window(hourlyFromNewYear(72U), 0U, 72U)};
    ASSERT_TRUE(whole.has_value());
    EXPECT_EQ(whole->series, hourlyFromNewYear(72U).series);
    EXPECT_EQ(whole->axis, TimeAxis{});
}

TEST(TestWindow, WindowPastTheEndOrEmptyIsAnError)
{
    EXPECT_FALSE(TimeSeriesLibrary::window(hourlyFromNewYear(72U), 60U, 24U).has_value());
    EXPECT_FALSE(TimeSeriesLibrary::window(hourlyFromNewYear(72U), 0U, 0U).has_value());
    const auto tooLong{TimeSeriesLibrary::window(hourlyFromNewYear(72U), 60U, 24U)};
    EXPECT_NE(tooLong.error().find("72 rows"), std::string::npos);
}
