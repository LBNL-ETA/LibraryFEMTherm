#include "gtest/gtest.h"

#include "THMZ/GlazingSystem/Serializers.hxx"

#include "GlazingSystemHelper.hxx"
#include "MockNodeAdapter.hxx"

class TestGlazingSystem : public testing::Test
{};

TEST_F(TestGlazingSystem, EnvironmentCoefficientsDeserializer)
{
    auto environmentCoefficientsNode = Helper::generateEnvironmentCoefficients({"3.16", "6.83", "4.101", "3.84"});
    const Helper::MockNodeAdapter adapter{&environmentCoefficientsNode};

    ThermFile::EnvironmentCoefficients coefficients;
    adapter >> coefficients;

    Helper::expect_near(ThermFile::EnvironmentCoefficients{3.16, 6.83, 4.101, 3.84}, coefficients, 1e-6);
}

TEST_F(TestGlazingSystem, EnvironmentCoefficientsSerializer)
{
    ThermFile::EnvironmentCoefficients coefficients{3.16, 6.83, 4.101, 3.84};

    Helper::MockNode elementNode("EnvironmentCoefficients");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << coefficients;

    auto correctNodes = Helper::generateEnvironmentCoefficients({"3.16", "6.83", "4.101", "3.84"});

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes));
}

TEST_F(TestGlazingSystem, GlazingSystemPropertiesDeserializer)
{
    auto glazingSystemPropertiesNode = Helper::generateGlazingSystemProperties(
      {"Winter",
       "BlackBody",
       "2.0",
       {{"Interior", {"3.16", "6.83", "4.101", "3.84"}}, {"Exterior", {"7.45", "7.43", "4.1254", "8.54"}}},
       "4.0"});
    const Helper::MockNodeAdapter adapter{&glazingSystemPropertiesNode};

    ThermFile::GlazingSystemProperties glazingSystemProperties;
    adapter >> glazingSystemProperties;

    ThermFile::GlazingSystemProperties correct{ThermFile::EnvironmentalCondition::Winter,
                                               ThermFile::BoundaryConditionStyle::BlackBody,
                                               2.0,
                                               {{ThermFile::Position::Interior, {3.16, 6.83, 4.101, 3.84}},
                                                {ThermFile::Position::Exterior, {7.45, 7.43, 4.1254, 8.54}}},
                                               4.0};

    Helper::expect_near(correct, glazingSystemProperties, 1e-6);
}

TEST_F(TestGlazingSystem, IGUSurfaceDeserializer)
{
    auto iguSurfaceNode{Helper::generateIGUSurface({"3.16", "0.83"})};
    const Helper::MockNodeAdapter adapter{&iguSurfaceNode};

    ThermFile::IGUSurface iguSurface;
    adapter >> iguSurface;

    Helper::expect_near(ThermFile::IGUSurface{3.16, 0.83}, iguSurface, 1e-6);
}

TEST_F(TestGlazingSystem, IGUSurfaceSerializer)
{
    ThermFile::IGUSurface iguSurface{3.16, 0.83};

    Helper::MockNode elementNode("Surface");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << iguSurface;

    auto correctNode{Helper::generateIGUSurface({"3.16", "0.83"})};

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNode));
}

TEST_F(TestGlazingSystem, IGULayerDeserializer)
{
    auto iguLayerNode{Helper::generateIGULayer({"1", "Name", "2", "3", {"3.16", "0.83"}, {"4.16", "0.83"}})};
    const Helper::MockNodeAdapter adapter{&iguLayerNode};

    ThermFile::IGULayer iguLayer;
    adapter >> iguLayer;

    ThermFile::IGULayer correct{1, "Name", 2.0, 3.0, {3.16, 0.83}, {4.16, 0.83}};

    Helper::expect_near(correct, iguLayer, 1e-6);
}

TEST_F(TestGlazingSystem, IGULayerSerializer)
{
    ThermFile::IGULayer iguLayer{1, "Name", 2.0, 3.0, {3.16, 0.83}, {4.16, 0.87}};

    Helper::MockNode elementNode("IGULayer");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << iguLayer;

    auto correctNode{Helper::generateIGULayer({"1", "Name", "2", "3", {"3.16", "0.83"}, {"4.16", "0.87"}})};

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNode));
}

TEST_F(TestGlazingSystem, IGUGasCoefficientsDeserializer)
{
    auto iguGasCoefficientsNode{Helper::generateIGUGasCoefficients({"0.2", "0.3", "0.4"})};
    const Helper::MockNodeAdapter adapter{&iguGasCoefficientsNode};

    ThermFile::IGUGasCoefficients iguGasCoefficients;
    adapter >> iguGasCoefficients;

    Helper::expect_near(ThermFile::IGUGasCoefficients{0.2, 0.3, 0.4}, iguGasCoefficients, 1e-6);
}

TEST_F(TestGlazingSystem, IGUGasCoefficientsSerializer)
{
    ThermFile::IGUGasCoefficients iguGasCoefficients{0.2, 0.3, 0.4};

    Helper::MockNode elementNode("IGUGasCoefficients");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << iguGasCoefficients;

    auto correctNode{Helper::generateIGUGasCoefficients({"0.2", "0.3", "0.4"})};

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNode));
}

TEST_F(TestGlazingSystem, IGUGasPropertiesDeserializer)
{
    auto iguGasPropertiesNode{Helper::generateIGUGasProperties(
      {"Name", "2", {"0.2", "0.3", "0.4"}, {"0.5", "0.6", "0.7"}, {"0.8", "0.9", "1.0"}})};
    const Helper::MockNodeAdapter adapter{&iguGasPropertiesNode};

    ThermFile::IGUGasProperties iguGasProperties;
    adapter >> iguGasProperties;

    ThermFile::IGUGasProperties correct{"Name", 2.0, {0.2, 0.3, 0.4}, {0.5, 0.6, 0.7}, {0.8, 0.9, 1.0}};

    Helper::expect_near(correct, iguGasProperties, 1e-6);
}

TEST_F(TestGlazingSystem, IGUGasPropertiesSerializer)
{
    ThermFile::IGUGasProperties iguGasProperties{"Name", 2.0, {0.2, 0.3, 0.4}, {0.5, 0.6, 0.7}, {0.8, 0.9, 1.0}};

    Helper::MockNode elementNode("IGUGasProperties");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << iguGasProperties;

    auto correctNode{Helper::generateIGUGasProperties(
      {"Name", "2", {"0.2", "0.3", "0.4"}, {"0.5", "0.6", "0.7"}, {"0.8", "0.9", "1"}})};

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNode));
}

TEST_F(TestGlazingSystem, IGUGasDeserializer)
{
    Helper::MockIGUGasProperties airProperties{"Air", "0.002873", {"1", "2", "3"}, {"4", "5", "6"}, {"7", "8", "9"}};
    Helper::MockIGUGasProperties argonProperties{
      "Argon", "0.002285", {"11", "12", "13"}, {"14", "15", "16"}, {"17", "18", "19"}};
    std::vector<Helper::MockIGUPureGas> components{{"0.2", airProperties}, {"0.8", argonProperties}};
    Helper::MockIGUGas iguMockGas{"1", "Name", "2", "3", components};
    auto iguGasNode{Helper::generateIGUGas(iguMockGas)};
    const Helper::MockNodeAdapter adapter{&iguGasNode};

    ThermFile::IGUGas iguGas;
    adapter >> iguGas;

    ThermFile::IGUGasProperties airCorrect{"Air", 0.002873, {1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    ThermFile::IGUGasProperties argonCorrect{"Argon", 0.002285, {11, 12, 13}, {14, 15, 16}, {17, 18, 19}};

    std::vector<ThermFile::IGUPureGas> correctComponents{{0.2, airCorrect}, {0.8, argonCorrect}};

    ThermFile::IGUGas correct{1, "Name", 2.0, 3.0, correctComponents};

    Helper::expect_near(correct, iguGas, 1e-6);
}

TEST_F(TestGlazingSystem, IGUGasSerializer)
{
    ThermFile::IGUGasProperties airCorrect{"Air", 0.002873, {1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    ThermFile::IGUGasProperties xenonCorrect{"Xenon", 0.0004538, {11, 12, 13}, {14, 15, 16}, {17, 18, 19}};

    std::vector<ThermFile::IGUPureGas> correctComponents{{0.2, airCorrect}, {0.8, xenonCorrect}};
    ThermFile::IGUGas iguGas{1, "Name", 2.0, 3.0, correctComponents};

    Helper::MockNode elementNode("IGUGas");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << iguGas;

    Helper::MockIGUGasProperties airProperties{"Air", "0.002873", {"1", "2", "3"}, {"4", "5", "6"}, {"7", "8", "9"}};
    Helper::MockIGUGasProperties xenonProperties{
      "Xenon", "4.538e-04", {"11", "12", "13"}, {"14", "15", "16"}, {"17", "18", "19"}};
    std::vector<Helper::MockIGUPureGas> components{{"0.2", airProperties}, {"0.8", xenonProperties}};
    auto correctNode{Helper::generateIGUGas({"1", "Name", "2", "3", components})};

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNode));
}

TEST_F(TestGlazingSystem, ShadePropertiesDeserializer)
{
    auto shadePropertiesNode{Helper::generateShadeProperties({"1", "2", "3", "4", "5"})};
    const Helper::MockNodeAdapter adapter{&shadePropertiesNode};

    ThermFile::ShadeProperties shadeProperties;
    adapter >> shadeProperties;

    ThermFile::ShadeProperties correct{1.0, 2.0, 3.0, 4.0, 5.0};

    Helper::expect_near(correct, shadeProperties, 1e-6);
}

TEST_F(TestGlazingSystem, ShadePropertiesSerializer)
{
    ThermFile::ShadeProperties shadeProperties{1.0, 2.0, 3.0, 4.0, 5.0};

    Helper::MockNode elementNode("ShadeProperties");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << shadeProperties;

    const auto correctNode{Helper::generateShadeProperties({"1", "2", "3", "4", "5"})};

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNode));
}

TEST_F(TestGlazingSystem, IGUShadeLayerDeserializer)
{
    auto iguShadeLayerNode{
      Helper::generateIGUShadeLayer({"Interior", "0.5", "0.6", "0.7", "0.8", "0.9", {"1", "2", "3", "4", "5"}})};
    const Helper::MockNodeAdapter adapter{&iguShadeLayerNode};

    ThermFile::IGUShadeLayer iguShadeLayer;
    adapter >> iguShadeLayer;

    ThermFile::IGUShadeLayer correct{ThermFile::Position::Interior, 0.5, 0.6, 0.7, 0.8, 0.9, {1.0, 2.0, 3.0, 4.0, 5.0}};

    Helper::expect_near(correct, iguShadeLayer, 1e-6);
}

TEST_F(TestGlazingSystem, IGUShadeLayerSerializer)
{
    ThermFile::IGUShadeLayer iguShadeLayer{
      ThermFile::Position::Interior, 0.5, 0.6, 0.7, 0.8, 0.9, {1.0, 2.0, 3.0, 4.0, 5.0}};

    Helper::MockNode elementNode("IGUShadeLayer");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << iguShadeLayer;

    const auto correctNode{
      Helper::generateIGUShadeLayer({"Interior", "0.5", "0.6", "0.7", "0.8", "0.9", {"1", "2", "3", "4", "5"}})};

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNode));
}

TEST_F(TestGlazingSystem, GlazingSystemSerializer)
{
    std::vector<ThermFile::GlazingSystemProperties> testGlzSysProperties{
      {ThermFile::EnvironmentalCondition::Winter,
       ThermFile::BoundaryConditionStyle::Existing,
       2.0,
       {{ThermFile::Position::Interior, {3.16, 6.83, 4.101, 3.84}},
        {ThermFile::Position::Exterior, {7.45, 7.43, 4.1254, 8.54}}},
       4.0},
      {ThermFile::EnvironmentalCondition::Summer,
       ThermFile::BoundaryConditionStyle::Split,
       3.0,
       {{ThermFile::Position::Interior, {3.26, 6.93, 4.201, 3.94}},
        {ThermFile::Position::Exterior, {7.55, 7.53, 4.2254, 8.64}}},
       5.0}};
    std::vector<ThermFile::IGULayer> testLayers{{102, "Layer1", 0.5, 1.5, {3.16, 0.83}, {4.16, 0.87}},
                                                {103, "Layer2", 0.6, 1.6, {3.26, 0.93}, {4.26, 0.97}}};
    ThermFile::IGUGasProperties airCorrect{"Air", 0.002873, {1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    std::vector<ThermFile::IGUPureGas> correctComponents{{1.0, airCorrect}};
    ThermFile::IGUGas iguGas{1, "Air", 0.2, 1.2, correctComponents};
    std::vector<ThermFile::IGUGas> testGases{{iguGas}};
    ThermFile::IGUShadeLayer testShade{
      ThermFile::Position::Interior, 0.5, 0.6, 0.7, 0.8, 0.9, {1.0, 2.0, 3.0, 4.0, 5.0}};
    ThermFile::GlazingSystem glazingSystem{1,
                                           100,
                                           "TestSystem",
                                           testGlzSysProperties,
                                           testLayers,
                                           testGases,
                                           testShade,
                                           2.5,
                                           1,
                                           true,
                                           "InteriorCondition",
                                           "ExteriorCondition",
                                           false,
                                           "Manual",
                                           1.5,
                                           0.5,
                                           0.25,
                                           false,
                                           "EdgeTag"};

    Helper::MockNode elementNode("GlazingSystem");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << glazingSystem;

    // Set up the MockGlazingSystem object with test data
    Helper::MockGlazingSystem correctGlazingSystem{
      "1",            // ID matches the actual system's ID
      "100",          // index matches the actual system's index
      "TestSystem",   // name matches the actual system's name
      // Properties need to reflect the actual GlazingSystemProperties used
      {{
         "Winter",     // EnvironmentalCondition as string
         "Existing",   // BoundaryConditionStyle as string
         "2",          // uFactor as string
         {             // Coefficients map correctly represented
          {"Exterior", {"7.45", "7.43", "4.1254", "8.54"}},
          {"Interior", {"3.16", "6.83", "4.101", "3.84"}}},
         "4"   // solar as string
       },
       {
         "Summer",   // Second property with Summer condition
         "Split",    // BoundaryConditionStyle as string
         "3",        // uFactor as string
         {           // Coefficients map correctly represented
          {"Exterior", {"7.55", "7.53", "4.2254", "8.64"}},
          {"Interior", {"3.26", "6.93", "4.201", "3.94"}}},
         "5"   // solar as string
       }},
      // Layers with differentiated properties
      {
        {"102", "Layer1", "0.5", "1.5", {"3.16", "0.83"}, {"4.16", "0.87"}},
        {"103", "Layer2", "0.6", "1.6", {"3.26", "0.93"}, {"4.26", "0.97"}}   // Varied thickness, conductivity, and
                                                                              // surface properties
      },
      // Gases matching the actual system's gases
      {{"1", "Air", "0.2", "1.2", {{"1", {"Air", "0.002873", {"1", "2", "3"}, {"4", "5", "6"}, {"7", "8", "9"}}}}}},
      // ShadeLayer matching the actual system's shade layer
      std::optional<Helper::MockIGUShadeLayer>(
        {"Interior", "0.5", "0.6", "0.7", "0.8", "0.9", {"1", "2", "3", "4", "5"}}),
      "2.5",
      "1",
      "true",
      "InteriorCondition",
      "ExteriorCondition",
      "false",
      "Manual",
      "1.5",
      "0.5",
      "0.25",
      "false",
      "EdgeTag"};


    Helper::MockNode correct{Helper::generateGlazingSystem(correctGlazingSystem)};

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correct));
}

TEST_F(TestGlazingSystem, GlazingSystemDeserializer)
{
    Helper::MockGlazingSystem glazingSystem{
      "1",            // ID matches the actual system's ID
      "100",          // index matches the actual system's index
      "TestSystem",   // name matches the actual system's name
      // Properties need to reflect the actual GlazingSystemProperties used
      {{
         "Winter",     // EnvironmentalCondition as string
         "Existing",   // BoundaryConditionStyle as string
         "2",          // uFactor as string
         {             // Coefficients map correctly represented
          {"Exterior", {"7.45", "7.43", "4.1254", "8.54"}},
          {"Interior", {"3.16", "6.83", "4.101", "3.84"}}},
         "4"   // solar as string
       },
       {
         "Summer",      // Second property with Summer condition
         "Enclosure",   // BoundaryConditionStyle as string
         "3",           // uFactor as string
         {              // Coefficients map correctly represented
          {"Exterior", {"7.55", "7.53", "4.2254", "8.64"}},
          {"Interior", {"3.26", "6.93", "4.201", "3.94"}}},
         "5"   // solar as string
       }},
      // Layers with differentiated properties
      {
        {"102", "Layer1", "0.5", "1.5", {"3.16", "0.83"}, {"4.16", "0.87"}},
        {"103", "Layer2", "0.6", "1.6", {"3.26", "0.93"}, {"4.26", "0.97"}}   // Varied thickness, conductivity, and
                                                                              // surface properties
      },
      // Gases matching the actual system's gases
      {{"1", "Air", "0.2", "1.2", {{"1.0", {"Air", "0.002873", {"1", "2", "3"}, {"4", "5", "6"}, {"7", "8", "9"}}}}}},
      // ShadeLayer matching the actual system's shade layer
      std::optional<Helper::MockIGUShadeLayer>(
        {"Interior", "0.5", "0.6", "0.7", "0.8", "0.9", {"1", "2", "3", "4", "5"}}),
      "2.5",
      "1",
      "true",
      "InteriorCondition",
      "ExteriorCondition",
      "false",
      "Manual",
      "1.5",
      "0.5",
      "0.25",
      "false",
      "EdgeTag"};

    Helper::MockNode glazingSystemNode{Helper::generateGlazingSystem(glazingSystem)};
    const Helper::MockNodeAdapter adapter{&glazingSystemNode};

    ThermFile::GlazingSystem actualGlazingSystem;
    adapter >> actualGlazingSystem;

    ThermFile::GlazingSystem correctGlazingSystem{
      1,
      100,
      "TestSystem",
      {{ThermFile::EnvironmentalCondition::Winter,
        ThermFile::BoundaryConditionStyle::Existing,
        2.0,
        {{ThermFile::Position::Interior, {3.16, 6.83, 4.101, 3.84}},
         {ThermFile::Position::Exterior, {7.45, 7.43, 4.1254, 8.54}}},
        4.0},
       {ThermFile::EnvironmentalCondition::Summer,
        ThermFile::BoundaryConditionStyle::Enclosure,
        3.0,
        {{ThermFile::Position::Interior, {3.26, 6.93, 4.201, 3.94}},
         {ThermFile::Position::Exterior, {7.55, 7.53, 4.2254, 8.64}}},
        5.0}},
      {{102, "Layer1", 0.5, 1.5, {3.16, 0.83}, {4.16, 0.87}}, {103, "Layer2", 0.6, 1.6, {3.26, 0.93}, {4.26, 0.97}}},
      {{1, "Air", 0.2, 1.2, {{1.0, {"Air", 0.002873, {1, 2, 3}, {4, 5, 6}, {7, 8, 9}}}}}},
      ThermFile::IGUShadeLayer{ThermFile::Position::Interior, 0.5, 0.6, 0.7, 0.8, 0.9, {1.0, 2.0, 3.0, 4.0, 5.0}},
      2.5,
      1,
      true,
      "InteriorCondition",
      "ExteriorCondition",
      false,
      "Manual",
      1.5,
      0.5,
      0.25,
      false,
      "EdgeTag"};

    Helper::expect_near(correctGlazingSystem, actualGlazingSystem, 1e-6);
}