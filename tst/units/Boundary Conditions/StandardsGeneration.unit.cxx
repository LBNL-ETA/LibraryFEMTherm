#include <gtest/gtest.h>

#include "TimeSeriesData/Standards.hxx"

using TimeSeriesLibrary::Series;
using TimeSeriesLibrary::SeriesRole;
using TimeSeriesLibrary::TimeSeriesData;
namespace Standards = TimeSeriesLibrary::Standards;

namespace
{
    TimeSeriesData makeExterior(std::vector<double> temperatures, std::vector<double> humidities)
    {
        TimeSeriesData exterior;
        exterior.Name = "Golden exterior";
        exterior.series = {Series{SeriesRole::AirTemperature, std::move(temperatures)},
                             Series{SeriesRole::RelativeHumidity, std::move(humidities)}};
        return exterior;
    }
}   // namespace

TEST(TestStandardsGeneration, EN15026RampAndHighLoadOffset)
{
    // Constant days pin the daily mean to the hourly value: cold (below the low
    // breakpoint), mid-ramp (15), and warm (above the high breakpoint).
    const auto exterior{makeExterior({-5.0, 15.0, 25.0}, {0.8, 0.8, 0.8})};

    const auto normal{Standards::generateInterior(
      exterior, Standards::Standard::EN_15026_DIN_4108, Standards::MoistureLoad::Normal, 1U)};
    ASSERT_TRUE(normal.has_value());

    const auto temperature{TimeSeriesLibrary::valuesForRole(normal.value(), SeriesRole::AirTemperature)};
    const auto humidity{TimeSeriesLibrary::valuesForRole(normal.value(), SeriesRole::RelativeHumidity)};
    ASSERT_TRUE(temperature.has_value());
    ASSERT_TRUE(humidity.has_value());

    EXPECT_NEAR(temperature->at(0), 20.0, 1e-9);
    EXPECT_NEAR(temperature->at(1), 21.0, 1e-9);
    EXPECT_NEAR(temperature->at(2), 22.0, 1e-9);
    EXPECT_NEAR(humidity->at(0), 0.35, 1e-9);   // ramp at -5 over [-10, 20] -> 0.30..0.60
    EXPECT_NEAR(humidity->at(2), 0.60, 1e-9);

    const auto high{Standards::generateInterior(
      exterior, Standards::Standard::EN_15026_DIN_4108, Standards::MoistureLoad::High, 1U)};
    ASSERT_TRUE(high.has_value());
    const auto highHumidity{TimeSeriesLibrary::valuesForRole(high.value(), SeriesRole::RelativeHumidity)};
    ASSERT_TRUE(highHumidity.has_value());
    EXPECT_NEAR(highHumidity->at(0), 0.45, 1e-9);
}

TEST(TestStandardsGeneration, ISO13788WarmOutdoorCarriesVapourAcross)
{
    // At or above the excess ramp's end the class excess is zero, and with the indoor
    // setpoint equal to the outdoor temperature the vapour density carries across
    // unchanged - indoor relative humidity equals outdoor.
    const auto exterior{makeExterior({20.0, 20.0}, {0.5, 0.5})};

    const auto result{Standards::generateInterior(
      exterior, Standards::Standard::ISO_13788, Standards::MoistureLoad::Normal, 1U)};
    ASSERT_TRUE(result.has_value());

    const auto temperature{TimeSeriesLibrary::valuesForRole(result.value(), SeriesRole::AirTemperature)};
    const auto humidity{TimeSeriesLibrary::valuesForRole(result.value(), SeriesRole::RelativeHumidity)};
    ASSERT_TRUE(temperature.has_value());
    ASSERT_TRUE(humidity.has_value());
    EXPECT_NEAR(temperature->at(0), 20.0, 1e-9);
    EXPECT_NEAR(humidity->at(0), 0.5, 1e-9);

    // A cold outdoor day with the same vapour content raises indoor humidity above the
    // carried-across value through the class excess.
    const auto cold{makeExterior({-5.0, -5.0}, {0.8, 0.8})};
    const auto coldResult{Standards::generateInterior(
      cold, Standards::Standard::ISO_13788, Standards::MoistureLoad::High, 1U)};
    ASSERT_TRUE(coldResult.has_value());
    const auto coldHumidity{TimeSeriesLibrary::valuesForRole(coldResult.value(), SeriesRole::RelativeHumidity)};
    ASSERT_TRUE(coldHumidity.has_value());
    const auto normalResult{Standards::generateInterior(
      cold, Standards::Standard::ISO_13788, Standards::MoistureLoad::Normal, 1U)};
    const auto normalHumidity{
      TimeSeriesLibrary::valuesForRole(normalResult.value(), SeriesRole::RelativeHumidity)};
    EXPECT_GT(coldHumidity->at(0), normalHumidity->at(0));
}

TEST(TestStandardsGeneration, ASHRAE160Band)
{
    const auto exterior{makeExterior({10.0, 25.0}, {0.6, 0.6})};

    const auto result{Standards::generateInterior(
      exterior, Standards::Standard::ASHRAE_160, Standards::MoistureLoad::Normal, 1U)};
    ASSERT_TRUE(result.has_value());

    const auto temperature{TimeSeriesLibrary::valuesForRole(result.value(), SeriesRole::AirTemperature)};
    ASSERT_TRUE(temperature.has_value());
    EXPECT_NEAR(temperature->at(0), 21.1, 1e-9);   // below the heating setpoint band
    EXPECT_NEAR(temperature->at(1), 23.9, 1e-9);   // above the cooling setpoint band
}

TEST(TestStandardsGeneration, DailyMeanHoldsAcrossTheDay)
{
    // Two 4-step days: a cold day and a warm day. Every step of a day gets the same
    // interior value - the interior responds to the day, not to the hour.
    const auto exterior{
      makeExterior({0.0, 0.0, 0.0, 0.0, 25.0, 25.0, 25.0, 25.0}, std::vector<double>(8, 0.7))};

    const auto result{Standards::generateInterior(
      exterior, Standards::Standard::EN_15026_DIN_4108, Standards::MoistureLoad::Normal, 4U)};
    ASSERT_TRUE(result.has_value());

    const auto temperature{TimeSeriesLibrary::valuesForRole(result.value(), SeriesRole::AirTemperature)};
    ASSERT_TRUE(temperature.has_value());
    ASSERT_EQ(temperature->size(), 8U);
    for(size_t index = 0; index < 4U; ++index)
    {
        EXPECT_NEAR(temperature->at(index), 20.0, 1e-9);
        EXPECT_NEAR(temperature->at(index + 4U), 22.0, 1e-9);
    }
}

TEST(TestStandardsGeneration, NamingStepsAndIdentity)
{
    const auto exterior{makeExterior(std::vector<double>(48, 12.0), std::vector<double>(48, 0.7))};

    const auto result{Standards::generateInterior(
      exterior, Standards::Standard::EN_15026_DIN_4108, Standards::MoistureLoad::Normal)};
    ASSERT_TRUE(result.has_value());

    EXPECT_EQ(result->Name, "Golden interior (EN 15026 / DIN 4108, normal)");
    EXPECT_EQ(TimeSeriesLibrary::steps(result.value()), 48U);
    EXPECT_FALSE(result->UUID.empty());

    // Same content, same identity: regeneration converges instead of duplicating.
    const auto again{Standards::generateInterior(
      exterior, Standards::Standard::EN_15026_DIN_4108, Standards::MoistureLoad::Normal)};
    EXPECT_EQ(result->UUID, again->UUID);
}

TEST(TestStandardsGeneration, MissingAirTemperatureIsAnError)
{
    TimeSeriesData exterior;
    exterior.Name = "No temperature";
    exterior.series = {Series{SeriesRole::WindSpeed, {1.0, 2.0}}};

    const auto result{Standards::generateInterior(
      exterior, Standards::Standard::ISO_13788, Standards::MoistureLoad::Normal)};
    EXPECT_FALSE(result.has_value());
}
