#include <gtest/gtest.h>

#include <cmath>
#include <string>
#include <vector>

#include "Authoring/Materials.hxx"

using namespace ThermFile::Authoring;

namespace
{
    constexpr double tolerance{1e-12};

    Material cottaerSandstone()
    {
        return Material{.name = "Cottaer Sandstone",
                        .diffusionResistanceFactor = 15.0,
                        .sorptionCurve = {{0.0, 0.0}, {0.5, 10.0}, {0.8, 27.0}, {0.98, 63.0}, {1.0, 180.0}},
                        .liquidTransportCurve = {{0.0, 0.0}, {27.0, 1e-9}, {180.0, 1e-7}},
                        .density = 2050.0,
                        .heatCapacity = 850.0,
                        .thermalConductivity = 1.8,
                        .porosity = 0.22};
    }

    //! k(T) = 1 + T: a unit conductivity with a unit temperature coefficient.
    Material unitThermal()
    {
        return Material{.name = "unit-kT",
                        .diffusionResistanceFactor = 1.0,
                        .sorptionCurve = {{0.0, 0.0}, {1.0, 1.0}},
                        .density = 1.0,
                        .heatCapacity = 1.0,
                        .thermalConductivity = 1.0,
                        .thermalConductivityBeta = 1.0};
    }

    //! A humidity-dependent resistance factor and a moisture-dependent conductivity.
    Material annexMaterial()
    {
        return Material{.name = "Annex material",
                        .diffusionResistanceFactor = 200.0,
                        .sorptionCurve = {{0.0, 0.0}, {0.5, 20.0}, {0.9, 60.0}, {1.0, 146.0}},
                        .muCurve = {{0.5, 200.0}, {0.9, 50.0}, {1.0, 20.0}},
                        .density = 2146.0,
                        .heatCapacity = 850.0,
                        .thermalConductivity = 1.5,
                        .thermalConductivityMoistureSlope = 0.0158 / 146.0,
                        .porosity = 0.146};
    }

    Material linearSorption()
    {
        return Material{.name = "linear-sorption",
                        .diffusionResistanceFactor = 10.0,
                        .sorptionCurve = {{0.0, 0.0}, {1.0, 100.0}}};
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

    //! By value: callers pass the temporary libraryMaterial(...) returns.
    MaterialsLibrary::HygroThermal hygro(const MaterialsLibrary::Material & record)
    {
        return record.data.hygroThermal.value();
    }
}   // namespace

TEST(AuthoringMaterials, UuidIsStableAndNameBound)
{
    EXPECT_EQ(materialUuid("Stucco"), materialUuid("Stucco"));
    EXPECT_NE(materialUuid("Stucco"), materialUuid("stucco"));
}

TEST(AuthoringMaterials, RecordCarriesNameAndUuid)
{
    const auto record{libraryMaterial(cottaerSandstone())};
    EXPECT_EQ(record.Name, "Cottaer Sandstone");
    EXPECT_EQ(record.UUID, materialUuid("Cottaer Sandstone"));
    EXPECT_FALSE(record.Protected);
}

TEST(AuthoringMaterials, ColorIsStableAndNeitherWhiteNorBlack)
{
    const auto first{libraryMaterial(cottaerSandstone()).Color};
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

TEST(AuthoringMaterials, ScalarsAndIsotherm)
{
    const auto material{cottaerSandstone()};
    const auto & record{hygro(libraryMaterial(material))};
    EXPECT_NEAR(record.BulkDensity.value(), 2050.0, tolerance);
    EXPECT_NEAR(record.Porosity.value(), 0.22, tolerance);
    EXPECT_NEAR(record.SpecificHeatCapacityDry.value(), 850.0, tolerance);
    EXPECT_NEAR(record.ThermalConductivityDry.value(), 1.8, tolerance);
    EXPECT_NEAR(record.WaterVaporDiffusionResistanceFactor.value(), 15.0, tolerance);
    EXPECT_EQ(record.MaterialInformation.value(), "NA");
    EXPECT_EQ(pairs(record.MoistureStorageFunction), material.sorptionCurve);
    EXPECT_EQ(pairs(record.LiquidTransportationCoefficientSuction), material.liquidTransportCurve);
    EXPECT_EQ(pairs(record.LiquidTransportationCoefficientRedistribution), material.liquidTransportCurve);
}

TEST(AuthoringMaterials, ConstantConductivityIsASinglePoint)
{
    const auto & record{hygro(libraryMaterial(cottaerSandstone()))};
    EXPECT_EQ(pairs(record.ThermalConductivityMoistureDependent), (Curve{{0.0, 1.8}}));
    EXPECT_EQ(pairs(record.ThermalConductivityTemperatureDependent), (Curve{{10.0, 1.8}}));
    EXPECT_FALSE(record.WaterVaporDiffusionResistanceFactorMoistureDependent.has_value());
}

TEST(AuthoringMaterials, LinearConductivityInTemperatureIsATwoPointTable)
{
    const auto table{pairs(hygro(libraryMaterial(unitThermal())).ThermalConductivityTemperatureDependent)};
    ASSERT_EQ(table.size(), 2U);
    for(const auto & [temperature, conductivity] : table)
    {
        EXPECT_NEAR(conductivity, 1.0 + temperature, tolerance);
    }
    EXPECT_NEAR(table.front().first, -50.0, tolerance);
    EXPECT_NEAR(table.back().first, 150.0, tolerance);
}

TEST(AuthoringMaterials, EmptyLiquidCurveBecomesAZeroTable)
{
    const auto & record{hygro(libraryMaterial(linearSorption()))};
    EXPECT_EQ(pairs(record.LiquidTransportationCoefficientSuction), (Curve{{0.0, 0.0}}));
}

TEST(AuthoringMaterials, WaterContentInterpolatesAndClamps)
{
    const auto material{cottaerSandstone()};
    EXPECT_NEAR(waterContent(material, 0.5), 10.0, tolerance);
    EXPECT_NEAR(waterContent(material, 0.65), 18.5, tolerance);
    EXPECT_NEAR(waterContent(material, -1.0), 0.0, tolerance);
    EXPECT_NEAR(waterContent(material, 2.0), 180.0, tolerance);
    EXPECT_NEAR(maxWaterContent(material), 180.0, tolerance);
}

TEST(AuthoringMaterials, ResistanceFactorIsRekeyedByWaterContent)
{
    const auto material{annexMaterial()};
    const auto table{resistanceFactorByWaterContent(material)};
    ASSERT_EQ(table.size(), material.muCurve.size());
    for(std::size_t index = 0U; index < table.size(); ++index)
    {
        EXPECT_NEAR(table[index].first, waterContent(material, material.muCurve[index].first), tolerance);
        EXPECT_NEAR(table[index].second, material.muCurve[index].second, tolerance);
    }
    const auto & record{hygro(libraryMaterial(material))};
    ASSERT_TRUE(record.WaterVaporDiffusionResistanceFactorMoistureDependent.has_value());
    EXPECT_EQ(record.WaterVaporDiffusionResistanceFactorMoistureDependent->size(), table.size());
}

TEST(AuthoringMaterials, LinearConductivityInWaterSpansTheIsotherm)
{
    const auto material{annexMaterial()};
    const auto table{pairs(hygro(libraryMaterial(material)).ThermalConductivityMoistureDependent)};
    ASSERT_EQ(table.size(), 2U);
    const double wMax{maxWaterContent(material)};
    EXPECT_NEAR(table[1].first, wMax, tolerance);
    EXPECT_NEAR(table[1].second,
                material.thermalConductivity + material.thermalConductivityMoistureSlope * wMax,
                tolerance);
}

TEST(AuthoringMaterials, DatabaseRoundTripsThroughXml)
{
    auto database{materialsDatabase({cottaerSandstone(), cottaerSandstone(), annexMaterial()})};
    EXPECT_EQ(database.getNames(), (std::vector<std::string>{"Cottaer Sandstone", "Annex material"}));

    MaterialsLibrary::DB loaded;
    loaded.loadFromString(database.saveToString());
    const auto record{loaded.getByUUID(materialUuid("Cottaer Sandstone"))};
    ASSERT_TRUE(record.has_value());
    EXPECT_NEAR(hygro(*record).BulkDensity.value(), 2050.0, tolerance);
    EXPECT_EQ(pairs(hygro(*record).MoistureStorageFunction), cottaerSandstone().sorptionCurve);
}
