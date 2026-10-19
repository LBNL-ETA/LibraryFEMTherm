#include <gtest/gtest.h>

#include <string>
#include <variant>

#include "BoundaryConditions/Operators.hxx"

using namespace BCLibrary;

namespace
{
    BoundaryCondition exterior(const double filmCoefficient)
    {
        return {.UUID = "1810f37a-0000-0000-0000-000000000000",
                .Name = "Exterior",
                .data = SurfaceExchange{.convection = Convection{.airTemperature = Constant{-18.0},
                                                                 .filmCoefficient = Constant{filmCoefficient}},
                                        .radiation = BlackBodyRadiation{.temperature = Constant{-18.0}}}};
    }

    std::string text(const LibraryCommon::PropertyValue & value)
    {
        if(const auto * label{std::get_if<std::string>(&value)})
        {
            return *label;
        }
        if(const auto * number{std::get_if<double>(&value)})
        {
            return std::to_string(*number);
        }
        return "N/A";
    }
}   // namespace

TEST(UnifiedBCOperators, IdentityAndNameAreNotPhysics)
{
    auto other{exterior(26.0)};
    other.UUID = "another";
    other.Name = "Renamed";
    other.Color = "0xFF0000";
    other.ProjectName = "sample-sill";

    EXPECT_TRUE(arePhysicalPropertiesEqual(exterior(26.0), other));
    EXPECT_TRUE(physicalDifferences(exterior(26.0), other).empty());
}

TEST(UnifiedBCOperators, ValuesReadBackFromTextAreEqualWithinTolerance)
{
    EXPECT_TRUE(arePhysicalPropertiesEqual(exterior(26.0), exterior(26.0 + 1e-9)));
}

TEST(UnifiedBCOperators, DifferingFilmIsOneRowWithItsPath)
{
    const auto diffs{physicalDifferences(exterior(26.0), exterior(30.0))};

    ASSERT_EQ(diffs.size(), 1U);
    EXPECT_EQ(diffs[0].name, "Convection-Film coefficient");
    EXPECT_EQ(std::get<double>(diffs[0].lhs), 26.0);
    EXPECT_EQ(std::get<double>(diffs[0].rhs), 30.0);
    EXPECT_FALSE(arePhysicalPropertiesEqual(exterior(26.0), exterior(30.0)));
}

TEST(UnifiedBCOperators, ConstantAgainstSeriesNamesTheRole)
{
    auto series{exterior(26.0)};
    std::get<SurfaceExchange>(series.data).convection->airTemperature =
      FromTimeSeries{TimeSeriesLibrary::SeriesRole::AirTemperature};

    const auto diffs{physicalDifferences(exterior(26.0), series)};

    ASSERT_EQ(diffs.size(), 1U);
    EXPECT_EQ(diffs[0].name, "Convection-Air temperature");
    EXPECT_EQ(text(diffs[0].rhs).rfind("series: ", 0), 0U) << text(diffs[0].rhs);
}

TEST(UnifiedBCOperators, DifferentRadiationTypeIsOneRow)
{
    auto enclosure{exterior(26.0)};
    std::get<SurfaceExchange>(enclosure.data).radiation = AutomaticEnclosure{};

    const auto diffs{physicalDifferences(exterior(26.0), enclosure)};

    ASSERT_EQ(diffs.size(), 1U);
    EXPECT_EQ(diffs[0].name, "Radiation-type");
}

TEST(UnifiedBCOperators, AdiabaticAgainstExchangeIsATypeDifference)
{
    const BoundaryCondition adiabatic{.Name = "Adiabatic", .data = NoExchange{}};

    const auto diffs{physicalDifferences(adiabatic, exterior(26.0))};

    ASSERT_EQ(diffs.size(), 1U);
    EXPECT_EQ(diffs[0].name, "Type");
    EXPECT_TRUE(arePhysicalPropertiesEqual(adiabatic, BoundaryCondition{.Name = "Other", .data = NoExchange{}}));
}

TEST(UnifiedBCOperators, MissingConvectionListsTheOtherSidesFields)
{
    auto radiationOnly{exterior(26.0)};
    std::get<SurfaceExchange>(radiationOnly.data).convection.reset();

    const auto diffs{physicalDifferences(exterior(26.0), radiationOnly)};

    EXPECT_FALSE(diffs.empty());
    for(const auto & diff : diffs)
    {
        EXPECT_EQ(diff.name.rfind("Convection-", 0), 0U) << diff.name;
        EXPECT_TRUE(std::holds_alternative<std::monostate>(diff.rhs)) << diff.name;
    }
}
