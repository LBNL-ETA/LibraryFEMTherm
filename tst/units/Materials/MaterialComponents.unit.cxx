#include <gtest/gtest.h>

#include "Materials/Materials.hxx"
#include "Materials/Serializers.hxx"

#include "MockNodeAdapter.hxx"
#include "MaterialsHelper.hxx"

class TestMaterialComponents : public testing::Test
{};

TEST_F(TestMaterialComponents, DeserializePoint)
{
    // Mock data setup
    auto mockData = []() {
        Helper::MockNode node{"Point"};

        addChildNode(node, "x", "3.14");
        addChildNode(node, "y", "1.59");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    MaterialsLibrary::point point;
    adapter >> point;

    MaterialsLibrary::point expectedPoint{3.14, 1.59};

    constexpr auto tolerance{1e-6};
    EXPECT_NEAR(expectedPoint.x, point.x, tolerance);
    EXPECT_NEAR(expectedPoint.y, point.y, tolerance);
}

TEST_F(TestMaterialComponents, SerializePoint)
{
    // Create a point object
    MaterialsLibrary::point point{3.14, 1.59};

    // Serialize
    Helper::MockNode elementNode("Point");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << point;

    // Expected mock data setup
    auto correctNodes = []() {
        Helper::MockNode node{"Point"};

        addChildNode(node, "x", "3.14");
        addChildNode(node, "y", "1.59");

        return node;
    };

    // Assertions
    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestMaterialComponents, DeserializeHygroThermal)
{
    // Mock data setup for HygroThermal
    auto mockData = []() {
        Helper::MockNode node{"HygroThermal"};

        addChildNode(node, "DefaultThickness", "0.1");
        addChildNode(node, "MaterialInformation", "Some material information");
        addChildNode(node, "BulkDensity", "800");
        addChildNode(node, "Porosity", "0.35");
        addChildNode(node, "SpecificHeatCapacityDry", "840");
        addChildNode(node, "ThermalConductivityDry", "0.2");
        addChildNode(node, "Roughness", "Medium Rough");

        fillNodesWithTableValues(node, "MoistureStorageFunction", "TableValue", {{"0.05", "12"}, {"0.1", "37"}});
        fillNodesWithTableValues(
          node, "LiquidTransportationCoefficientSuction", "TableValue", {{"35", "3.1e-10"}, {"100", "3.1e-09"}});
        fillNodesWithTableValues(
          node, "ThermalConductivityMoistureDependent", "TableValue", {{"0.05", "0.1"}, {"0.1", "0.2"}});
        fillNodesWithTableValues(
          node, "ThermalConductivityTemperatureDependent", "TableValue", {{"10", "0.18"}, {"15", "0.23"}});

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    // Deserialize
    MaterialsLibrary::HygroThermal material;
    adapter >> material;

    // Expected values
    constexpr double tolerance = 1e-6;

    // Assertions for scalar properties
    ASSERT_TRUE(material.DefaultThickness.has_value());
    EXPECT_NEAR(0.1, material.DefaultThickness.value(), tolerance);
    ASSERT_TRUE(material.BulkDensity.has_value());
    EXPECT_NEAR(800, material.BulkDensity.value(), tolerance);
    ASSERT_TRUE(material.Porosity.has_value());
    EXPECT_NEAR(0.35, material.Porosity.value(), tolerance);
    ASSERT_TRUE(material.SpecificHeatCapacityDry.has_value());
    EXPECT_NEAR(840, material.SpecificHeatCapacityDry.value(), tolerance);
    ASSERT_TRUE(material.ThermalConductivityDry.has_value());
    EXPECT_NEAR(0.2, material.ThermalConductivityDry.value(), tolerance);
    ASSERT_TRUE(material.Roughness.has_value());
    EXPECT_EQ(static_cast<int>(MaterialsLibrary::MaterialRoughness::MediumRough),
              static_cast<int>(material.Roughness.value()));

    Helper::assertVectorOfPoints({{0.05, 12}, {0.1, 37}}, material.MoistureStorageFunction, tolerance);
    Helper::assertVectorOfPoints(
      {{35, 3.1e-10}, {100, 3.1e-9}}, material.LiquidTransportationCoefficientSuction, tolerance);
    Helper::assertVectorOfPoints({{0.05, 0.1}, {0.1, 0.2}}, material.ThermalConductivityMoistureDependent, tolerance);
    Helper::assertVectorOfPoints({{10, 0.18}, {15, 0.23}}, material.ThermalConductivityTemperatureDependent, tolerance);
}

TEST_F(TestMaterialComponents, SerializeHygroThermal)
{
    // Prepare a HygroThermal object with known values
    MaterialsLibrary::HygroThermal material;
    material.DefaultThickness = 0.1;
    material.MaterialInformation = "Some material information";
    material.BulkDensity = 800;
    material.Porosity = 0.35;
    material.SpecificHeatCapacityDry = 840;
    material.ThermalConductivityDry = 0.2;
    material.Roughness = MaterialsLibrary::MaterialRoughness::MediumRough;
    material.MoistureStorageFunction = {{0.05, 12}, {0.1, 37}};
    material.LiquidTransportationCoefficientSuction = {{35, 3.1e-10}, {100, 3.1e-9}};
    material.ThermalConductivityMoistureDependent = {{0.05, 0.1}, {0.1, 0.2}};
    material.ThermalConductivityTemperatureDependent = {{10, 0.18}, {15, 0.23}};

    // Serialize
    Helper::MockNode elementNode("HygroThermal");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << material;

    // Create the expected mock node structure
    auto expectedMockData = []() {
        Helper::MockNode node{"HygroThermal"};

        addChildNode(node, "DefaultThickness", "0.1");
        addChildNode(node, "MaterialInformation", "Some material information");
        addChildNode(node, "BulkDensity", "800");
        addChildNode(node, "Porosity", "0.35");
        addChildNode(node, "SpecificHeatCapacityDry", "840");
        addChildNode(node, "ThermalConductivityDry", "0.2");
        addChildNode(node, "Roughness", "Medium Rough");

        fillNodesWithTableValues(node, "MoistureStorageFunction", "TableValue", {{"0.05", "12"}, {"0.1", "37"}});
        fillNodesWithTableValues(
          node, "LiquidTransportationCoefficientSuction", "TableValue", {{"35", "3.1e-10"}, {"100", "3.1e-09"}});
        fillNodesWithTableValues(
          node, "ThermalConductivityMoistureDependent", "TableValue", {{"0.05", "0.1"}, {"0.1", "0.2"}});
        fillNodesWithTableValues(
          node, "ThermalConductivityTemperatureDependent", "TableValue", {{"10", "0.18"}, {"15", "0.23"}});

        return node;
    };

    auto expected{expectedMockData()};
    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), expectedMockData()));
}

TEST_F(TestMaterialComponents, DeserializeOpticalProperties)
{
    // Mock data setup for OpticalProperties
    auto mockData = []() {
        Helper::MockNode node{"OpticalProperties"};

        addChildNode(node, "Transmittance", "0.5");
        addChildNode(node, "Reflectance", "0.3");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    // Deserialize
    MaterialsLibrary::OpticalProperties opticalProperties;
    adapter >> opticalProperties;

    constexpr double tolerance = 1e-6;
    EXPECT_NEAR(0.5, opticalProperties.Transmittance, tolerance);
    EXPECT_NEAR(0.3, opticalProperties.Reflectance, tolerance);
}

TEST_F(TestMaterialComponents, SerializeOpticalProperties)
{
    // Prepare an OpticalProperties object with known values
    MaterialsLibrary::OpticalProperties opticalProperties;
    opticalProperties.Transmittance = 0.5;
    opticalProperties.Reflectance = 0.3;

    // Serialize
    Helper::MockNode elementNode("OpticalProperties");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << opticalProperties;

    // Create the expected mock node structure
    auto expectedMockData = []() {
        Helper::MockNode node{"OpticalProperties"};

        addChildNode(node, "Transmittance", "0.5");
        addChildNode(node, "Reflectance", "0.3");

        return node;
    };

    auto expected{expectedMockData()};
    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), expectedMockData()));
}

TEST_F(TestMaterialComponents, DeserializeOpticalSide)
{
    // Mock data setup for OpticalSide
    auto mockData = []() {
        Helper::MockNode node{"OpticalSide"};

        auto & frontSide = addChildNode(node, "Front");
        addChildNode(frontSide, "Transmittance", "0.5");
        addChildNode(frontSide, "Reflectance", "0.3");

        auto & backSide = addChildNode(node, "Back", "OpticalProperties");
        addChildNode(backSide, "Transmittance", "0.7");
        addChildNode(backSide, "Reflectance", "0.1");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    // Deserialize
    MaterialsLibrary::OpticalSide opticalSide;
    adapter >> opticalSide;

    constexpr double tolerance = 1e-6;
    EXPECT_NEAR(0.5, opticalSide.Front.Transmittance, tolerance);
    EXPECT_NEAR(0.3, opticalSide.Front.Reflectance, tolerance);
    EXPECT_NEAR(0.7, opticalSide.Back.Transmittance, tolerance);
    EXPECT_NEAR(0.1, opticalSide.Back.Reflectance, tolerance);
}

TEST_F(TestMaterialComponents, SerializeOpticalSide)
{
    MaterialsLibrary::OpticalSide opticalSide{{0.5, 0.3}, {0.7, 0.1}};

    // Serialize
    Helper::MockNode elementNode("OpticalSide");
    Helper::MockNodeAdapter adapter{&elementNode};

    adapter << opticalSide;

    // Create the expected mock node structure
    auto expectedMockData = []() {
        Helper::MockNode node{"OpticalSide"};

        fillOpticalPropertiesNode(node, "Front", "0.5", "0.3");
        fillOpticalPropertiesNode(node, "Back", "0.7", "0.1");

        return node;
    };

    auto expected{expectedMockData()};
    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), expectedMockData()));
}

TEST_F(TestMaterialComponents, DeserializeOpticalType)
{
    // Mock data setup for OpticalType
    auto mockData = []() {
        Helper::MockNode node{"OpticalType"};

        fillOpticalSide(node, "Direct", {{"Front", {"0.5", "0.3"}}, {"Back", {"0.7", "0.1"}}});
        fillOpticalSide(node, "Diffuse", {{"Front", {"0.5", "0.3"}}, {"Back", {"0.7", "0.1"}}});

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    // Deserialize
    MaterialsLibrary::OpticalType opticalType;
    adapter >> opticalType;

    // Assertions
    constexpr double tolerance = 1e-6;
    EXPECT_NEAR(0.5, opticalType.Direct.Front.Transmittance, tolerance);
    EXPECT_NEAR(0.3, opticalType.Direct.Front.Reflectance, tolerance);
    EXPECT_NEAR(0.7, opticalType.Direct.Back.Transmittance, tolerance);
    EXPECT_NEAR(0.1, opticalType.Direct.Back.Reflectance, tolerance);
}

TEST_F(TestMaterialComponents, SerializeOpticalType)
{
    MaterialsLibrary::OpticalType opticalType;
    opticalType.Direct = {{0.5, 0.3}, {0.7, 0.1}};
    opticalType.Diffuse = {{0.5, 0.3}, {0.7, 0.1}};

    // Serialize
    Helper::MockNode elementNode("OpticalType");
    Helper::MockNodeAdapter adapter{&elementNode};

    adapter << opticalType;

    // Create the expected mock node structure
    auto expectedMockData = []() {
        Helper::MockNode node{"OpticalType"};

        fillOpticalSide(node, "Direct", {{"Front", {"0.5", "0.3"}}, {"Back", {"0.7", "0.1"}}});
        fillOpticalSide(node, "Diffuse", {{"Front", {"0.5", "0.3"}}, {"Back", {"0.7", "0.1"}}});

        return node;
    };

    auto expected{expectedMockData()};
    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), expectedMockData()));
}

TEST_F(TestMaterialComponents, DeserializeInfrared)
{
    // Mock data setup for Infrared
    auto mockData = []() {
        Helper::MockNode node{"Infrared"};

        addChildNode(node, "Transmittance", "0.5");
        addChildNode(node, "Emissivity-Front", "0.3");
        addChildNode(node, "Emissivity-Back", "0.1");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    // Deserialize
    MaterialsLibrary::Infrared infrared;
    adapter >> infrared;

    constexpr double tolerance = 1e-6;
    EXPECT_NEAR(0.5, infrared.Transmittance.value(), tolerance);
    EXPECT_NEAR(0.3, infrared.EmissivityFront.value(), tolerance);
    EXPECT_NEAR(0.1, infrared.EmissivityBack.value(), tolerance);
}

TEST_F(TestMaterialComponents, SerializeInfrared)
{
    MaterialsLibrary::Infrared infrared;
    infrared.Transmittance = 0.5;
    infrared.EmissivityFront = 0.3;
    infrared.EmissivityBack = 0.1;

    // Serialize
    Helper::MockNode elementNode("Infrared");
    Helper::MockNodeAdapter adapter{&elementNode};

    adapter << infrared;

    // Create the expected mock node structure
    auto expectedMockData = []() {
        Helper::MockNode node{"Infrared"};

        addChildNode(node, "Transmittance", "0.5");
        addChildNode(node, "Emissivity-Front", "0.3");
        addChildNode(node, "Emissivity-Back", "0.1");

        return node;
    };

    auto expected{expectedMockData()};
    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), expectedMockData()));
}

TEST_F(TestMaterialComponents, DeserializeIntegrated)
{
    // Mock data setup for Integrated
    auto mockData = []() {
        Helper::MockNode node{"Integrated"};

        fillInfraredSection(node, "0.5", "0.3", "0.1");

        fillOpticalRange(node,
                         "Solar",
                         {{"Direct", {{"Front", {"0.6", "0.35"}}, {"Back", {"0.76", "0.16"}}}},
                          {"Diffuse", {{"Front", {"0.58", "0.36"}}, {"Back", {"0.78", "0.18"}}}}});

        fillOpticalRange(node,
                         "Visible",
                         {{"Direct", {{"Front", {"0.65", "0.4"}}, {"Back", {"0.8", "0.2"}}}},
                          {"Diffuse", {{"Front", {"0.6", "0.38"}}, {"Back", {"0.75", "0.25"}}}}});

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    // Deserialize
    MaterialsLibrary::Integrated integrated;
    adapter >> integrated;

    // Assertions
    constexpr double tolerance = 1e-6;
    Helper::assertInfraredSection({0.5, 0.3, 0.1}, integrated.IR, tolerance);
    Helper::assertOpticalRange({{0.6, 0.35, 0.76, 0.16}, {0.58, 0.36, 0.78, 0.18}}, integrated.Solar, tolerance);
    Helper::assertOpticalRange({{0.65, 0.4, 0.8, 0.2}, {0.6, 0.38, 0.75, 0.25}}, integrated.Visible, tolerance);
}
