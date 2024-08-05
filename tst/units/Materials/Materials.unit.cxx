#include "gtest/gtest.h"

#include <variant>

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

    ASSERT_TRUE(std::holds_alternative<MaterialsLibrary::Solid>(material.data));
    const auto & solid{std::get<MaterialsLibrary::Solid>(material.data)};
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
    Helper::assertVectorOfPoints(
      {{10, 0.18}, {15, 0.23}}, hygroThermal.ThermalConductivityTemperatureDependent, tolerance);

    ASSERT_TRUE(solid.optical.has_value());
    ASSERT_TRUE(solid.optical.value().integrated.has_value());
    const auto & integrated{solid.optical->integrated.value()};
    Helper::assertInfraredSection({std::nullopt, 0.8, 0.7}, integrated.IR, tolerance);

    Helper::assertOpticalRange({{0.75, 0.14, 0.7, 0.28}, {0.55, 0.33, 0.82, 0.15}}, integrated.Solar, tolerance);
    Helper::assertOpticalRange({{0.61, 0.47, 0.91, 0.35}, {0.61, 0.28, 0.85, 0.24}}, integrated.Visible, tolerance);
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
    solid.hygroThermal =
      MaterialsLibrary::HygroThermal{0.1,
                                     "Some material information",
                                     800.0,
                                     0.35,
                                     840.0,
                                     0.2,
                                     MaterialsLibrary::MaterialRoughness::MediumRough,
                                     std::nullopt,
                                     {{{0.05, 12}, {0.1, 37}}},
                                     {{{35, 3.1e-10}, {100, 3.1e-09}}},
                                     std::nullopt,
                                     {{{0.05, 0.1}, {0.1, 0.2}}},
                                     {{{10, 0.18}, {15, 0.23}}}};

    // Set up the optical properties
    solid.optical = MaterialsLibrary::Optical{MaterialsLibrary::Integrated{
      {std::nullopt, 0.8, 0.7},
      {{0.75, 0.14, 0.7, 0.28}, {0.55, 0.33, 0.82, 0.15}},
      {{0.61, 0.47, 0.91, 0.35}, {0.61, 0.28, 0.85, 0.24}}   // Use generated values
    }};

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

TEST_F(TestMaterials, RadiationEnclosureDeserialization)
{
        // Mock data setup
        auto mockData = []() {
                Helper::MockNode node{"Materials"};

                auto & materialNode{addChildNode(node, "Material")};
                addChildNode(materialNode, "UUID", "RadiationEnclosureUUID");
                addChildNode(materialNode, "Name", "SomeRadiationEnclosure");
                addChildNode(materialNode, "Protected", "true");
                addChildNode(materialNode, "Color", "0x8080FF");

                auto & radiationEnclosureNode{addChildNode(materialNode, "RadiationEnclosure")};
                addChildNode(radiationEnclosureNode, "EmissivityDefault", "0.8");

                return node;
        };

        auto elementNode(mockData());
        const Helper::MockNodeAdapter adapter{&elementNode};

        MaterialsLibrary::Material material;
        adapter >> FileParse::Child{"Material", material};

        constexpr auto tolerance{1e-6};
        EXPECT_EQ("SomeRadiationEnclosure", material.Name);
        EXPECT_EQ("RadiationEnclosureUUID", material.UUID);
        EXPECT_EQ(true, material.Protected);
        EXPECT_EQ("0x8080FF", material.Color);

        ASSERT_TRUE(std::holds_alternative<MaterialsLibrary::RadiationEnclosure>(material.data));
        const auto & radiationEnclosure{std::get<MaterialsLibrary::RadiationEnclosure>(material.data)};
        Helper::assertOptionalNear(0.8, radiationEnclosure.emissivityDefault, tolerance);
}

TEST_F(TestMaterials, RadiationEnclosureSerialization)
{
        // Set up the material object with all properties
        MaterialsLibrary::Material material;
        material.Name = "SomeRadiationEnclosure";
        material.UUID = "RadiationEnclosureUUID";
        material.Protected = true;
        material.Color = "8080FF";

        // Set up the radiation enclosure properties
        MaterialsLibrary::RadiationEnclosure radiationEnclosure;
        radiationEnclosure.emissivityDefault = 0.8;

        material.data = radiationEnclosure;

        // Serialize
        Helper::MockNode elementNode("Materials");
        const Helper::MockNodeAdapter adapter{&elementNode};

        adapter << FileParse::Child{"Material", material};

        // Expected mock node structure setup
        auto expectedMockData = []() {
                Helper::MockNode node{"Materials"};

                auto & materialNode{addChildNode(node, "Material")};
                addChildNode(materialNode, "UUID", "RadiationEnclosureUUID");
                addChildNode(materialNode, "Name", "SomeRadiationEnclosure");
                addChildNode(materialNode, "Protected", "true");
                addChildNode(materialNode, "Color", "8080FF");

                auto & radiationEnclosureNode{addChildNode(materialNode, "RadiationEnclosure")};
                addChildNode(radiationEnclosureNode, "EmissivityDefault", "0.8");

                return node;
        };

        auto expected{expectedMockData()};
        EXPECT_TRUE(Helper::compareNodes(elementNode, expected));
}

TEST_F(TestMaterials, FrameCavityDeserialization)
{
        // Mock data setup
        auto mockData = []() {
                Helper::MockNode node{"Materials"};

                auto & materialNode{addChildNode(node, "Material")};
                addChildNode(materialNode, "UUID", "FrameCavityUUID");
                addChildNode(materialNode, "Name", "SomeFrameCavity");
                addChildNode(materialNode, "Protected", "true");
                addChildNode(materialNode, "Color", "0x80FF80");

                auto & frameCavityNode{addChildNode(materialNode, "Cavity")};
                addChildNode(frameCavityNode, "CavityStandard", "ISO15099");
                addChildNode(frameCavityNode, "Gas", "Air");
                addChildNode(frameCavityNode, "EmissivitySide1", "0.74");
                addChildNode(frameCavityNode, "EmissivitySide2", "0.85");

                return node;
        };

        auto elementNode(mockData());
        const Helper::MockNodeAdapter adapter{&elementNode};

        MaterialsLibrary::Material material;
        adapter >> FileParse::Child{"Material", material};

        constexpr auto tolerance{1e-6};
        EXPECT_EQ("SomeFrameCavity", material.Name);
        EXPECT_EQ("FrameCavityUUID", material.UUID);
        EXPECT_EQ(true, material.Protected);
        EXPECT_EQ("0x80FF80", material.Color);

        ASSERT_TRUE(std::holds_alternative<MaterialsLibrary::Cavity>(material.data));
        const auto & frameCavity{std::get<MaterialsLibrary::Cavity>(material.data)};
        EXPECT_EQ(static_cast<int>(MaterialsLibrary::CavityStandard::ISO15099), static_cast<int>(frameCavity.cavityStandard));
        EXPECT_EQ("Air", frameCavity.GasName);
        Helper::assertOptionalNear(0.74, frameCavity.EmissivitySide1, tolerance);
        Helper::assertOptionalNear(0.85, frameCavity.EmissivitySide2, tolerance);
}

TEST_F(TestMaterials, FrameCavitySerialization)
{
        // Set up the material object with all properties
        MaterialsLibrary::Material material;
        material.Name = "SomeFrameCavity";
        material.UUID = "FrameCavityUUID";
        material.Protected = true;
        material.Color = "80FF80";

        // Set up the frame cavity properties
        MaterialsLibrary::Cavity frameCavity;
        frameCavity.cavityStandard = MaterialsLibrary::CavityStandard::ISO15099;
        frameCavity.GasName = "Air";
        frameCavity.EmissivitySide1 = 0.74;
        frameCavity.EmissivitySide2 = 0.85;

        material.data = frameCavity;

        // Serialize
        Helper::MockNode elementNode("Materials");
        const Helper::MockNodeAdapter adapter{&elementNode};

        adapter << FileParse::Child{"Material", material};

        // Expected mock node structure setup
        auto expectedMockData = []() {
                Helper::MockNode node{"Materials"};

                auto & materialNode{addChildNode(node, "Material")};
                addChildNode(materialNode, "UUID", "FrameCavityUUID");
                addChildNode(materialNode, "Name", "SomeFrameCavity");
                addChildNode(materialNode, "Protected", "true");
                addChildNode(materialNode, "Color", "80FF80");

                auto & frameCavityNode{addChildNode(materialNode, "Cavity")};
                addChildNode(frameCavityNode, "CavityStandard", "ISO15099");
                addChildNode(frameCavityNode, "Gas", "Air");
                addChildNode(frameCavityNode, "EmissivitySide1", "0.74");
                addChildNode(frameCavityNode, "EmissivitySide2", "0.85");

                return node;
        };

        auto expected{expectedMockData()};
        EXPECT_TRUE(Helper::compareNodes(elementNode, expected));
}