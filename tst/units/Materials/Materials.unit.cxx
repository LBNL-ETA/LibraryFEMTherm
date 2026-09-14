#include "gtest/gtest.h"

#include "Materials/Materials.hxx"
#include "Materials/Serializers.hxx"

#include "MockNodeAdapter.hxx"
#include "MaterialsHelper.hxx"
#include "HelperUtility.hxx"

class TestMaterials : public testing::Test
{};

TEST_F(TestMaterials, SolidMaterial1Deserialization)
{
    // Mock data setup
    auto mockData = []() {
        Helper::MockNode node{"Materials"};

        Helper::fillSolidMaterialTest1(node);

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    MaterialsLibrary::Material material;
    adapter >> FileParse::Child{"Material", material};

    constexpr auto tolerance{1e-6};
    EXPECT_EQ("SomeMaterial", material.Name);
    EXPECT_EQ("SomeUUID", material.UUID);
    EXPECT_EQ(true, material.Protected);
    EXPECT_EQ("0xFF8080", material.Color);

    const auto & solid{material.data};
    ASSERT_TRUE(solid.hygroThermal.has_value());
    const auto & hygroThermal{solid.hygroThermal.value()};
    Helper::assertOptionalNear(0.1, hygroThermal.DefaultThickness, tolerance);
    ASSERT_TRUE(hygroThermal.MaterialInformation.has_value());
    EXPECT_EQ("Some material information", hygroThermal.MaterialInformation.value());
    Helper::assertOptionalNear(800.0, hygroThermal.BulkDensity, tolerance);
    Helper::assertOptionalNear(0.35, hygroThermal.Porosity, tolerance);
    Helper::assertOptionalNear(840.0, hygroThermal.SpecificHeatCapacityDry, tolerance);
    Helper::assertOptionalNear(0.2, hygroThermal.ThermalConductivityDry, tolerance);
    ASSERT_TRUE(hygroThermal.Roughness.has_value());
    EXPECT_EQ(static_cast<int>(MaterialsLibrary::MaterialRoughness::MediumRough),
              static_cast<int>(hygroThermal.Roughness.value()));

    Helper::assertVectorOfPoints({{0.05, 12}, {0.1, 37}}, hygroThermal.MoistureStorageFunction, tolerance);
    Helper::assertVectorOfPoints(
      {{35, 3.1e-10}, {100, 3.1e-09}}, hygroThermal.LiquidTransportationCoefficientSuction, tolerance);
    Helper::assertVectorOfPoints(
      {{0.05, 0.1}, {0.1, 0.2}}, hygroThermal.ThermalConductivityMoistureDependent, tolerance);
    Helper::assertVectorOfPoints({{12.0, 220.0}, {37.0, 640.0}},
                                 hygroThermal.WaterVaporDiffusionResistanceFactorMoistureDependent,
                                 tolerance);
    Helper::assertVectorOfPoints(
      {{10, 0.18}, {15, 0.23}}, hygroThermal.ThermalConductivityTemperatureDependent, tolerance);

    ASSERT_TRUE(solid.optical.has_value());
    ASSERT_TRUE(solid.optical.value().integrated.has_value());
    const auto & integrated{solid.optical->integrated.value()};
    Helper::assertInfraredSection({std::nullopt, 0.8, 0.7}, integrated.IR, tolerance);

    Helper::assertOpticalRange({{{0.75, 0.14}, {0.7, 0.28}}, {{0.55, 0.33}, {0.82, 0.15}}}, integrated.Solar, tolerance);
    Helper::assertOpticalRange({{{0.61, 0.47}, {0.91, 0.35}}, {{0.61, 0.28}, {0.85, 0.24}}}, integrated.Visible, tolerance);
}

TEST_F(TestMaterials, SolidMaterial1Serialization)
{
    // Set up the material object with all properties
    MaterialsLibrary::Material material;
    material.Name = "SomeMaterial";
    material.UUID = "SomeUUID";
    material.Protected = true;
    material.Color = "0xFF8080";

    // Set up the solid material properties
    MaterialsLibrary::Solid solid;
    // Designated initializers on purpose: this struct gains fields in the middle over
    // time, and a positional list silently shifts every value after the insertion point.
    solid.hygroThermal = MaterialsLibrary::HygroThermal{
      .DefaultThickness = 0.1,
      .MaterialInformation = "Some material information",
      .BulkDensity = 800.0,
      .Porosity = 0.35,
      .SpecificHeatCapacityDry = 840.0,
      .ThermalConductivityDry = 0.2,
      .Roughness = MaterialsLibrary::MaterialRoughness::MediumRough,
      .WaterVaporDiffusionResistanceFactor = std::nullopt,
      .MoistureStorageFunction = {{{0.05, 12}, {0.1, 37}}},
      .LiquidTransportationCoefficientSuction = {{{35, 3.1e-10}, {100, 3.1e-09}}},
      .LiquidTransportationCoefficientRedistribution = std::nullopt,
      .ThermalConductivityMoistureDependent = {{{0.05, 0.1}, {0.1, 0.2}}},
      .WaterVaporDiffusionResistanceFactorMoistureDependent = {{{12.0, 220.0}, {37.0, 640.0}}},
      .ThermalConductivityTemperatureDependent = {{{10, 0.18}, {15, 0.23}}}};

    // clang-format off
    MaterialsLibrary::Optical optical{
        .integrated = MaterialsLibrary::Integrated
        {
            .IR = {.Transmittance = std::nullopt, .EmissivityFront = 0.8, .EmissivityBack = 0.7},
            .Solar = MaterialsLibrary::OpticalType{
                .Direct =
                {
                    .Front = {.Transmittance = 0.75, .Reflectance = 0.14},
                    .Back = {.Transmittance = 0.7, .Reflectance = 0.28}
                },
                .Diffuse =
                {
                    .Front = {.Transmittance = 0.55, .Reflectance = 0.33},
                    .Back = {.Transmittance = 0.82, .Reflectance = 0.15}
                }
            },
            .Visible = MaterialsLibrary::OpticalType{
                .Direct =
                {
                    .Front = {.Transmittance = 0.61, .Reflectance = 0.47},
                    .Back = {.Transmittance = 0.91, .Reflectance = 0.35}
                },
                .Diffuse =
                {
                    .Front = {.Transmittance = 0.61, .Reflectance = 0.28},
                    .Back = {.Transmittance = 0.85, .Reflectance = 0.24}
                }
            }
        }
        // clang-format on
    };

    solid.optical = optical;

    material.data = solid;

    // Serialize
    Helper::MockNode elementNode("Materials");
    const Helper::MockNodeAdapter adapter{&elementNode};

    adapter << FileParse::Child{"Material", material};

    // Expected mock node structure setup
    auto expectedMockData = []() {
        Helper::MockNode node{"Materials"};

        // The same function is used in deserialization test
        Helper::fillSolidMaterialTest1(node);

        return node;
    };

    auto expected{expectedMockData()};
    EXPECT_TRUE(Helper::compareNodes(elementNode, expected));
}
