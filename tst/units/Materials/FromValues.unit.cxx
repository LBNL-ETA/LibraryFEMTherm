#include <gtest/gtest.h>

#include <cmath>
#include <optional>
#include <string>
#include <vector>

#include "Materials/FromValues.hxx"

using namespace MaterialsLibrary;

namespace
{
    constexpr double tolerance{1e-12};

    const Curve cottaerIsotherm{{0.0, 0.0}, {0.5, 10.0}, {0.8, 27.0}, {0.98, 63.0}, {1.0, 180.0}};
    const Curve cottaerLiquid{{0.0, 0.0}, {27.0, 1e-9}, {180.0, 1e-7}};

    MaterialsLibrary::Material cottaerSandstone(const std::optional<std::string> & color = std::nullopt)
    {
        return fromValues(
          "Cottaer Sandstone", 15.0, cottaerIsotherm, cottaerLiquid, {}, 2050.0, 850.0, 1.8, 0.0, 0.0, 0.22, color);
    }

    //! k(T) = 1 + T: a unit conductivity with a unit temperature coefficient.
    MaterialsLibrary::Material unitThermal()
    {
        return fromValues("unit-kT", 1.0, {{0.0, 0.0}, {1.0, 1.0}}, {}, {}, 1.0, 1.0, 1.0, 1.0);
    }

    //! A humidity-dependent resistance factor and a moisture-dependent conductivity.
    const Curve annexIsotherm{{0.0, 0.0}, {0.5, 20.0}, {0.9, 60.0}, {1.0, 146.0}};
    const Curve annexMu{{0.5, 200.0}, {0.9, 50.0}, {1.0, 20.0}};
    constexpr double annexConductivity{1.5};
    constexpr double annexSlope{0.0158 / 146.0};

    MaterialsLibrary::Material annexMaterial()
    {
        return fromValues(
          "Annex material", 200.0, annexIsotherm, {}, annexMu, 2146.0, 850.0, annexConductivity, 0.0, annexSlope, 0.146);
    }

    MaterialsLibrary::Material linearSorption()
    {
        return fromValues("linear-sorption", 10.0, {{0.0, 0.0}, {1.0, 100.0}});
    }

    std::vector<std::pair<double, double>> pairs(const std::optional<std::vector<MaterialsLibrary::point>> & table)
    {
        std::vector<std::pair<double, double>> result;
        for(const auto & sample : table.value())
        {
            result.emplace_back(sample.x, sample.y);
        }
        return result;
    }

    //! By value: callers pass the temporary fromValues(...) returns.
    MaterialsLibrary::HygroThermal hygro(const MaterialsLibrary::Material & record)
    {
        return record.data.hygroThermal.value();
    }
}   // namespace

TEST(MaterialsFromValues, UuidIsStableAndNameBound)
{
    EXPECT_EQ(materialUuid("Stucco"), materialUuid("Stucco"));
    EXPECT_NE(materialUuid("Stucco"), materialUuid("stucco"));
}

TEST(MaterialsFromValues, RecordCarriesNameAndUuid)
{
    const auto record{cottaerSandstone()};
    EXPECT_EQ(record.Name, "Cottaer Sandstone");
    EXPECT_EQ(record.UUID, materialUuid("Cottaer Sandstone"));
    EXPECT_FALSE(record.Protected);
}

TEST(MaterialsFromValues, AStatedColourReplacesTheDerivedOne)
{
    EXPECT_EQ(cottaerSandstone().Color, materialColor("Cottaer Sandstone"));
    EXPECT_EQ(cottaerSandstone("0xB03A2E").Color, "0xB03A2E");
}

TEST(MaterialsFromValues, ColorIsStableAndNeitherWhiteNorBlack)
{
    const auto first{cottaerSandstone().Color};
    EXPECT_EQ(first, materialColor("Cottaer Sandstone"));
    ASSERT_EQ(first.size(), 8U);
    EXPECT_EQ(first.substr(0, 2), "0x");
    EXPECT_NE(first, "0xFFFFFF");
    EXPECT_NE(first, "0x000000");
    EXPECT_NE(materialColor("Stucco"), first);

    // Each channel is held to [64, 224): off both extremes.
    for(std::size_t offset = 2U; offset < 8U; offset += 2U)
    {
        const auto channel{std::stoul(first.substr(offset, 2), nullptr, 16)};
        EXPECT_GE(channel, 64UL);
        EXPECT_LT(channel, 224UL);
    }
}

TEST(MaterialsFromValues, ScalarsAndIsotherm)
{
    const auto & record{hygro(cottaerSandstone())};
    EXPECT_NEAR(record.BulkDensity.value(), 2050.0, tolerance);
    EXPECT_NEAR(record.Porosity.value(), 0.22, tolerance);
    EXPECT_NEAR(record.SpecificHeatCapacityDry.value(), 850.0, tolerance);
    EXPECT_NEAR(record.ThermalConductivityDry.value(), 1.8, tolerance);
    EXPECT_NEAR(record.WaterVaporDiffusionResistanceFactor.value(), 15.0, tolerance);
    EXPECT_EQ(record.MaterialInformation.value(), "NA");
    EXPECT_EQ(pairs(record.MoistureStorageFunction), cottaerIsotherm);
    EXPECT_EQ(pairs(record.LiquidTransportationCoefficientSuction), cottaerLiquid);
    EXPECT_EQ(pairs(record.LiquidTransportationCoefficientRedistribution), cottaerLiquid);
}

TEST(MaterialsFromValues, ConstantConductivityIsASinglePoint)
{
    const auto & record{hygro(cottaerSandstone())};
    EXPECT_EQ(pairs(record.ThermalConductivityMoistureDependent), (Curve{{0.0, 1.8}}));
    EXPECT_EQ(pairs(record.ThermalConductivityTemperatureDependent), (Curve{{10.0, 1.8}}));
    EXPECT_FALSE(record.WaterVaporDiffusionResistanceFactorMoistureDependent.has_value());
}

TEST(MaterialsFromValues, LinearConductivityInTemperatureIsATwoPointTable)
{
    const auto table{pairs(hygro(unitThermal()).ThermalConductivityTemperatureDependent)};
    ASSERT_EQ(table.size(), 2U);
    for(const auto & [temperature, conductivity] : table)
    {
        EXPECT_NEAR(conductivity, 1.0 + temperature, tolerance);
    }
    EXPECT_NEAR(table.front().first, -50.0, tolerance);
    EXPECT_NEAR(table.back().first, 150.0, tolerance);
}

TEST(MaterialsFromValues, EmptyLiquidCurveBecomesAZeroTable)
{
    const auto & record{hygro(linearSorption())};
    EXPECT_EQ(pairs(record.LiquidTransportationCoefficientSuction), (Curve{{0.0, 0.0}}));
}

TEST(MaterialsFromValues, WaterContentInterpolatesAndClamps)
{
    EXPECT_NEAR(waterContent(cottaerIsotherm, 0.5), 10.0, tolerance);
    EXPECT_NEAR(waterContent(cottaerIsotherm, 0.65), 18.5, tolerance);
    EXPECT_NEAR(waterContent(cottaerIsotherm, -1.0), 0.0, tolerance);
    EXPECT_NEAR(waterContent(cottaerIsotherm, 2.0), 180.0, tolerance);
    EXPECT_NEAR(maxWaterContent(cottaerIsotherm), 180.0, tolerance);
}

TEST(MaterialsFromValues, ResistanceFactorIsRekeyedByWaterContent)
{
    const auto table{resistanceFactorByWaterContent(annexMu, annexIsotherm)};
    ASSERT_EQ(table.size(), annexMu.size());
    for(std::size_t index = 0U; index < table.size(); ++index)
    {
        EXPECT_NEAR(table[index].first, waterContent(annexIsotherm, annexMu[index].first), tolerance);
        EXPECT_NEAR(table[index].second, annexMu[index].second, tolerance);
    }
    const auto & record{hygro(annexMaterial())};
    ASSERT_TRUE(record.WaterVaporDiffusionResistanceFactorMoistureDependent.has_value());
    EXPECT_EQ(record.WaterVaporDiffusionResistanceFactorMoistureDependent->size(), table.size());
}

TEST(MaterialsFromValues, LinearConductivityInWaterSpansTheIsotherm)
{
    const auto table{pairs(hygro(annexMaterial()).ThermalConductivityMoistureDependent)};
    ASSERT_EQ(table.size(), 2U);
    const double wMax{maxWaterContent(annexIsotherm)};
    EXPECT_NEAR(table[1].first, wMax, tolerance);
    EXPECT_NEAR(table[1].second, annexConductivity + annexSlope * wMax, tolerance);
}

TEST(MaterialsFromValues, DatabaseRoundTripsThroughXml)
{
    auto database{materialsDatabase({cottaerSandstone(), cottaerSandstone(), annexMaterial()})};
    EXPECT_EQ(database.getNames(), (std::vector<std::string>{"Cottaer Sandstone", "Annex material"}));

    MaterialsLibrary::DB loaded;
    loaded.loadFromString(database.saveToString());
    const auto record{loaded.getByUUID(materialUuid("Cottaer Sandstone"))};
    ASSERT_TRUE(record.has_value());
    EXPECT_NEAR(hygro(*record).BulkDensity.value(), 2050.0, tolerance);
    EXPECT_EQ(pairs(hygro(*record).MoistureStorageFunction), cottaerIsotherm);
}
