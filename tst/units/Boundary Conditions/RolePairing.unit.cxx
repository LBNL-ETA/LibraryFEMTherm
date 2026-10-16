#include <gtest/gtest.h>

#include "BoundaryConditions/BoundaryConditions.hxx"

using BCLibrary::BoundaryCondition;
using TimeSeriesLibrary::Series;
using TimeSeriesLibrary::SeriesRole;
using TimeSeriesLibrary::TimeSeriesData;

namespace
{
    BoundaryCondition convectiveReading(const std::vector<SeriesRole> & roles)
    {
        BCLibrary::Convection convection;
        convection.airTemperature = BCLibrary::Constant{20.0};
        convection.filmCoefficient = BCLibrary::Constant{8.0};
        BCLibrary::SurfaceExchange exchange{.relativeHumidity = BCLibrary::Constant{0.5}, .convection = convection};
        for(const auto role : roles)
        {
            switch(role)
            {
                case SeriesRole::AirTemperature:
                    exchange.convection->airTemperature = BCLibrary::FromTimeSeries{role};
                    break;
                case SeriesRole::ConvectiveCoefficient:
                    exchange.convection->filmCoefficient = BCLibrary::FromTimeSeries{role};
                    break;
                case SeriesRole::RelativeHumidity:
                    exchange.relativeHumidity = BCLibrary::FromTimeSeries{role};
                    break;
                case SeriesRole::RadiantTemperature:
                    exchange.radiation = BCLibrary::FixedCoefficientRadiation{.temperature = BCLibrary::FromTimeSeries{role},
                                                                              .coefficient = BCLibrary::Constant{4.5}};
                    break;
                default:
                    break;
            }
        }
        return BoundaryCondition{.Name = "probe", .data = exchange};
    }

    TimeSeriesData providing(const std::vector<SeriesRole> & roles)
    {
        TimeSeriesData data{.Name = "weather"};
        for(const auto role : roles)
        {
            data.series.push_back(Series{.role = role, .values = {1.0, 2.0}});
        }
        return data;
    }
}   // namespace

TEST(TestRolePairing, ARecordOfConstantsMissesNothingEvenWithoutADataset)
{
    EXPECT_TRUE(BCLibrary::missingRoles(convectiveReading({}), std::nullopt).empty());
    EXPECT_TRUE(BCLibrary::missingRoles(convectiveReading({}), providing({})).empty());
}

TEST(TestRolePairing, WithoutADatasetEveryRoleReadIsMissing)
{
    const auto record{convectiveReading({SeriesRole::AirTemperature, SeriesRole::ConvectiveCoefficient})};
    EXPECT_EQ(BCLibrary::missingRoles(record, std::nullopt), BCLibrary::requiredRoles(record));
}

TEST(TestRolePairing, ProvidedRolesAreNotMissing)
{
    const auto record{convectiveReading({SeriesRole::RelativeHumidity, SeriesRole::AirTemperature, SeriesRole::ConvectiveCoefficient})};
    const auto missing{BCLibrary::missingRoles(record, providing({SeriesRole::AirTemperature, SeriesRole::RelativeHumidity}))};
    EXPECT_EQ(missing, (std::vector<SeriesRole>{SeriesRole::ConvectiveCoefficient}));
    EXPECT_TRUE(BCLibrary::missingRoles(record, providing({SeriesRole::RelativeHumidity, SeriesRole::AirTemperature, SeriesRole::ConvectiveCoefficient})).empty());
}

TEST(TestRolePairing, AMissingRadiantTemperatureIsTakenAsTheAirTemperature)
{
    const auto record{convectiveReading({SeriesRole::RadiantTemperature})};
    EXPECT_TRUE(BCLibrary::missingRoles(record, providing({SeriesRole::AirTemperature})).empty());
    EXPECT_EQ(BCLibrary::missingRoles(record, providing({SeriesRole::RelativeHumidity})),
              (std::vector<SeriesRole>{SeriesRole::RadiantTemperature}));
}
