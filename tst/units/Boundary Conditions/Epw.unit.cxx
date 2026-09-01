#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "TimeSeriesData/ContentHash.hxx"
#include "TimeSeriesData/Epw.hxx"

using TimeSeriesLibrary::SeriesRole;

namespace Epw = TimeSeriesLibrary::Epw;

namespace
{
    //! One EPW data record: 35 comma-separated fields, zeros unless set.
    std::string makeRecord(const std::string & dryBulb,
                           const std::string & humidity,
                           const std::string & infrared,
                           const std::string & windDirection,
                           const std::string & windSpeed)
    {
        std::vector<std::string> fields(35U, "0");
        fields[0] = "2026";
        fields[1] = "1";
        fields[2] = "1";
        fields[3] = "1";
        fields[6] = dryBulb;
        fields[8] = humidity;
        fields[9] = "101325";
        fields[12] = infrared;
        fields[20] = windDirection;
        fields[21] = windSpeed;
        fields[30] = "999";   // snow depth at its sentinel: the column is missing
        std::string record;
        for(const auto & field : fields)
        {
            if(!record.empty())
            {
                record += ",";
            }
            record += field;
        }
        return record + "\n";
    }

    std::string makeHeader()
    {
        std::string header{"LOCATION,Sydney,NSW,AUS,TMY,947670,-33.95,151.18,10.0,3.0\n"};
        for(size_t index = 0; index < 7U; ++index)
        {
            header += "HEADER LINE\n";
        }
        return header;
    }

    // sigma * 280.15^4 = 349.28 W/m2, so the derived sky temperature is 7.00 degC.
    constexpr auto infraredForSevenDegrees{"349.28"};
}   // namespace

TEST(TestEpw, ImportReadsSeriesAndLocation)
{
    const std::string content{makeHeader()
                              + makeRecord("10", "50", infraredForSevenDegrees, "180", "5")
                              + makeRecord("11", "999", infraredForSevenDegrees, "190", "6")
                              + makeRecord("12", "60", infraredForSevenDegrees, "200", "7")};

    const auto result{Epw::readFromString(content, "sydney")};
    ASSERT_TRUE(result.has_value());

    const auto & imported{result->data};
    EXPECT_EQ(imported.Name, "sydney");
    EXPECT_EQ(TimeSeriesLibrary::steps(imported), 3U);
    EXPECT_EQ(imported.UUID, TimeSeriesLibrary::contentUuid(imported));

    EXPECT_EQ(result->location.city, "Sydney");
    EXPECT_EQ(result->location.country, "AUS");
    EXPECT_NEAR(result->location.latitude, -33.95, 1e-9);
    EXPECT_NEAR(result->location.elevation, 3.0, 1e-9);

    const auto temperature{
      TimeSeriesLibrary::valuesForRole(imported, SeriesRole::AirTemperature)};
    ASSERT_TRUE(temperature.has_value());
    EXPECT_NEAR(temperature->at(2), 12.0, 1e-9);

    // Per cent in the file, a fraction here; the sentinel row carries the last reading.
    const auto humidity{
      TimeSeriesLibrary::valuesForRole(imported, SeriesRole::RelativeHumidity)};
    ASSERT_TRUE(humidity.has_value());
    EXPECT_NEAR(humidity->at(0), 0.5, 1e-9);
    EXPECT_NEAR(humidity->at(1), 0.5, 1e-9);
    EXPECT_NEAR(humidity->at(2), 0.6, 1e-9);

    const auto wind{TimeSeriesLibrary::valuesForRole(imported, SeriesRole::WindSpeed)};
    ASSERT_TRUE(wind.has_value());
    EXPECT_NEAR(wind->at(1), 6.0, 1e-9);
    const auto direction{TimeSeriesLibrary::valuesForRole(imported, SeriesRole::WindDirection)};
    ASSERT_TRUE(direction.has_value());
    EXPECT_NEAR(direction->at(0), 180.0, 1e-9);
}

TEST(TestEpw, ImportDerivesRadiantTemperatureFromInfrared)
{
    const std::string content{makeHeader()
                              + makeRecord("10", "50", infraredForSevenDegrees, "180", "5")};

    const auto result{Epw::readFromString(content, "sky")};
    ASSERT_TRUE(result.has_value());

    const auto radiant{
      TimeSeriesLibrary::valuesForRole(result->data, SeriesRole::RadiantTemperature)};
    ASSERT_TRUE(radiant.has_value());
    EXPECT_NEAR(radiant->front(), 7.0, 0.01);
}

TEST(TestEpw, ImportReportsUnmappedColumnsAndDropsAllMissingOnes)
{
    const std::string content{makeHeader()
                              + makeRecord("10", "50", infraredForSevenDegrees, "180", "5")};

    const auto result{Epw::readFromString(content, "columns")};
    ASSERT_TRUE(result.has_value());

    const auto & unmapped{result->unmappedColumns};
    const auto contains{[&unmapped](const std::string & label) {
        return std::ranges::find(unmapped, label) != unmapped.end();
    }};
    EXPECT_TRUE(contains("Atmospheric station pressure [Pa]"));
    EXPECT_FALSE(contains("Snow depth [cm]"));   // sentinel in every record
}

TEST(TestEpw, ImportRejectsContentWithoutLocationHeader)
{
    const auto result{Epw::readFromString("NOT A WEATHER FILE\n", "broken")};
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "broken has no LOCATION header");
}

TEST(TestEpw, ImportRejectsContentWithoutRecords)
{
    const auto result{Epw::readFromString(makeHeader(), "headerOnly")};
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "headerOnly has no data records");
}

TEST(TestEpw, SkyTemperatureClampsToOneWattMinimum)
{
    const auto temperatures{Epw::skyTemperature({0.0})};
    ASSERT_EQ(temperatures.size(), 1U);
    // max(0, 1) W/m2 -> (1/sigma)^0.25 - 273.15 = 64.8 K - 273.15
    EXPECT_NEAR(temperatures.front(), -208.35, 0.05);
}
