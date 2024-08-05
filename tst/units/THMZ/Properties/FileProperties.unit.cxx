#include "gtest/gtest.h"

#include <variant>

#include "THMZ/Properties/Serializers.hxx"

#include "PropertiesHelper.hxx"
#include "TestExamples.hxx"
#include "MockNodeAdapter.hxx"

class TestFileProperties : public testing::Test
{};

TEST_F(TestFileProperties, ConstantInitialConditionsDeserializer)
{
    auto elementNode(Helper::constantInitialConditionsSteadyStateNode({"20", "0.5"}));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::ConstantInitialConditions initialConditions;
    adapter >> initialConditions;

    ThermFile::ConstantInitialConditions correctInitialConditions{20.0, 0.5};
    Helper::expect_near(correctInitialConditions, initialConditions, 1e-6);
}

TEST_F(TestFileProperties, ConstantInitialConditionsSerializer)
{
    ThermFile::ConstantInitialConditions initialConditions{32.7, 0.65};

    Helper::MockNode node{"ConstantInitialConditionsSteadyState"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << initialConditions;

    EXPECT_TRUE(
      Helper::compareNodes(adapter.getNode(), Helper::constantInitialConditionsSteadyStateNode({"32.7", "0.65"})));
}

TEST_F(TestFileProperties, ModelingOptionsDeserializer)
{
    auto elementNode(Helper::modelingOptionsNode({"true", "false", "true", "false", "true", "false", "true"}));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::ModelingOptions options;
    adapter >> options;

    ThermFile::ModelingOptions correctOptions{true, false, true, false, true, false, true};
    Helper::expect_eq(correctOptions, options);
}

TEST_F(TestFileProperties, ModelingOptionsSerializer)
{
    ThermFile::ModelingOptions options{true, false, true, false, true, false, true};

    Helper::MockNode node{"ModelingOptions"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << options;

    EXPECT_TRUE(Helper::compareNodes(
      adapter.getNode(), Helper::modelingOptionsNode({"true", "false", "true", "false", "true", "false", "true"})));
}

TEST_F(TestFileProperties, EngineParametersDeserialization)
{
    auto elementNode(Helper::engineParametersNode({"1e-06", "0.84", "3600", "1245"}));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::EngineParameters engineParameters;
    adapter >> engineParameters;

    ThermFile::EngineParameters correctParameters{1e-6, 0.84, 3600, 1245};
    Helper::expect_near(correctParameters, engineParameters, 1e-6);
}

TEST_F(TestFileProperties, EngineParametersSerialization)
{
    ThermFile::EngineParameters engineParameters{1e-6, 0.84, 3600, 1245};

    Helper::MockNode node{"EngineParameters"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << engineParameters;

    EXPECT_TRUE(
      Helper::compareNodes(adapter.getNode(), Helper::engineParametersNode({"1e-06", "0.84", "3600", "1245"})));
}

TEST_F(TestFileProperties, MeshControlDeserialization)
{
    auto elementNode(Helper::meshControlNode({"QuadTree Mesher", "7", "true", "1e-06", "100"}));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::MeshControl meshControl;
    adapter >> meshControl;

    ThermFile::MeshControl correctControl{ThermFile::MesherType::QuadTreeMesher, 7u, true, 1e-6, 100};
    Helper::expect_near(correctControl, meshControl, 1e-6);
}

TEST_F(TestFileProperties, MeshControlSerialization)
{
    ThermFile::MeshControl meshControl{ThermFile::MesherType::Simmetrix_Version_2022, 5u, true, 1e-3, 25};

    Helper::MockNode node{"MeshControl"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << meshControl;

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(),
                                     Helper::meshControlNode({"Simmetrix Version 2022", "5", "true", "0.001", "25"})));
}

TEST_F(TestFileProperties, MiscPropertiesDeserialization)
{
    auto elementNode(Helper::miscPropertiesNode({"false", "true", "true"}));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::MiscProperties miscProperties;
    adapter >> miscProperties;

    ThermFile::MiscProperties correctProperties{false, true, true};
    Helper::expect_eq(correctProperties, miscProperties);
}

TEST_F(TestFileProperties, MiscPropertiesSerialization)
{
    ThermFile::MiscProperties miscProperties{false, true, true};

    Helper::MockNode node{"MiscellaneousProperties"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << miscProperties;

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), Helper::miscPropertiesNode({"false", "true", "true"})));
}

TEST_F(TestFileProperties, HeatTransferModelingOptionsDeserialization)
{
    auto elementNode(Helper::heatTransferModelingOptionsNode({"false", "-0.01", "28", "true"}));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::HeatTransferModelingOptions modelingOptions;
    adapter >> modelingOptions;

    ThermFile::HeatTransferModelingOptions correctOptions{false, -0.01, 28u, true};
    Helper::expect_near(correctOptions, modelingOptions, 1e-6);
}

TEST_F(TestFileProperties, HeatTransferModelingOptionsSerialization)
{
    ThermFile::HeatTransferModelingOptions modelingOptions{false, -0.05, 34u, false};

    Helper::MockNode node{"HeatTransferModelingOptions"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << modelingOptions;

    Helper::MockNode correctNodes{Helper::heatTransferModelingOptionsNode({"false", "-0.05", "34", "false"})};

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes));
}

TEST_F(TestFileProperties, ModelOrientationDeserialization)
{
    auto elementNode(Helper::modelOrientationNode({"East", "112"}));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::ModelOrientation orientation;
    adapter >> orientation;

    ThermFile::ModelOrientation correctOrientation{ThermFile::Orientation::East, 112};
    Helper::expect_near(correctOrientation, orientation, 1e-6);
}

TEST_F(TestFileProperties, ModelOrientationSerialization)
{
    ThermFile::ModelOrientation orientation;
    orientation.orientation = ThermFile::Orientation::South;
    orientation.cardinalOrientation = 231;

    Helper::MockNode node{"ModelOrientation"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << orientation;

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), Helper::modelOrientationNode({"South", "231"})));
}

TEST_F(TestFileProperties, GeneralDeserialization)
{
    auto elementNode(Helper::generalNode({"18",
                                          "testFile",
                                          "C:/testDirectory",
                                          "2021-12-31 18:23:34",
                                          "2023-03-08 08:15:46",
                                          "15",
                                          "17",
                                          "Test Title",
                                          "Test User",
                                          "Test Company",
                                          "Test Client",
                                          "Test Notes"}));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::General general;
    adapter >> general;

    ThermFile::General correctGeneral{"18",
                                      "testFile",
                                      "C:/testDirectory",
                                      "2021-12-31 18:23:34",
                                      "2023-03-08 08:15:46",
                                      "15",
                                      "17",
                                      "Test Title",
                                      "Test User",
                                      "Test Company",
                                      "Test Client",
                                      "Test Notes"};

    Helper::expect_eq(correctGeneral, general);
}

TEST_F(TestFileProperties, GeneralSerialization)
{
    ThermFile::General general{"20",
                               "testFile",
                               "C:/testDirectory",
                               "2021-12-31 18:23:34",
                               "2023-03-08 08:15:46",
                               "15",
                               "17",
                               "Test Title",
                               "Test User",
                               "Test Company",
                               "Test Client",
                               "Test Notes"};

    Helper::MockNode node{"General"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << general;

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(),
                                     Helper::generalNode({"20",
                                                          "testFile",
                                                          "C:/testDirectory",
                                                          "2021-12-31 18:23:34",
                                                          "2023-03-08 08:15:46",
                                                          "15",
                                                          "17",
                                                          "Test Title",
                                                          "Test User",
                                                          "Test Company",
                                                          "Test Client",
                                                          "Test Notes"})));
}

TEST_F(TestFileProperties, CalculationOptionsDeserialization)
{
    auto mockData{Helper::calculationOptionsNode({"HygroThermFEM",
                                                  "Steady-State",
                                                  "true",
                                                  "true",
                                                  {"false", "true", "false", "true", "false", "true", "false"},
                                                  "Constant everywhere",
                                                  "No time variable",
                                                  {"20", "0.5"},
                                                  {"25", "0.75"},
                                                  {"1e-06", "0.84", "3600", "1803"},
                                                  {"QuadTree Mesher", "7", "true", "1e-06", "100"},
                                                  {"false", "-0.01", "28", "true"},
                                                  {"false", "true", "true"}})};

    auto elementNode(mockData);
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::CalculationOptions calculationOptions;
    adapter >> calculationOptions;

    ThermFile::CalculationOptions correctOptions{ThermFile::SimulationEngine::HygroThermFEM,
                                                 ThermFile::CalculationMode::cmSteadyState,
                                                 true,
                                                 true,
                                                 {false, true, false, true, false, true, false},
                                                 ThermFile::TransientCalculationMethodology::icConstantEverywhere,
                                                 ThermFile::SteadyStateCalculationMethodology::ssNoTimeVariable,
                                                 {20.0, 0.5},
                                                 {25.0, 0.75},
                                                 {1e-6, 0.84, 3600, 1803},
                                                 {ThermFile::MesherType::QuadTreeMesher, 7u, true, 1e-6, 100},
                                                 {false, -0.01, 28u, true},
                                                 {false, true, true}};

    constexpr auto tolerance{1e-6};
    Helper::expect_near(correctOptions, calculationOptions, tolerance);
}

TEST_F(TestFileProperties, CalculationOptionsSerialization)
{
    ThermFile::CalculationOptions calculationOptions{ThermFile::SimulationEngine::HygroThermFEM,
                                                     ThermFile::CalculationMode::cmSteadyState,
                                                     true,
                                                     true,
                                                     {false, true, false, true, false, true, false},
                                                     ThermFile::TransientCalculationMethodology::icConstantEverywhere,
                                                     ThermFile::SteadyStateCalculationMethodology::ssNoTimeVariable,
                                                     {20.0, 0.5},
                                                     {25.0, 0.75},
                                                     {1e-6, 0.84, 3600, 1803},
                                                     {ThermFile::MesherType::QuadTreeMesher, 7u, true, 1e-6, 100},
                                                     {false, -0.01, 28u, true},
                                                     {false, true, true}};

    Helper::MockNode node{"CalculationOptions"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << calculationOptions;

    auto correctNodes{Helper::calculationOptionsNode({"HygroThermFEM",
                                                      "Steady-State",
                                                      "true",
                                                      "true",
                                                      {"false", "true", "false", "true", "false", "true", "false"},
                                                      "Constant everywhere",
                                                      "No time variable",
                                                      {"20", "0.5"},
                                                      {"25", "0.75"},
                                                      {"1e-06", "0.84", "3600", "1803"},
                                                      {"QuadTree Mesher", "7", "true", "1e-06", "100"},
                                                      {"false", "-0.01", "28", "true"},
                                                      {"false", "true", "true"}})};

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes));
}

TEST_F(TestFileProperties, ExposureTypeWindowDeserialization)
{
    auto elementNode(Helper::exposureTypeWindowNode("Horizontal Divider"));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::ExposureType exposureType;
    adapter >> exposureType;

    ThermFile::ExposureType correctExposureType{ThermFile::ModelPurpose::Window_TransparentFacade,
                                                ThermFile::WindowCrossSectionType::HorizontalDivider};
    Helper::expect_eq(correctExposureType, exposureType);
}

TEST_F(TestFileProperties, ExposureTypeWindowSerialization)
{
    ThermFile::ExposureType exposureType{ThermFile::ModelPurpose::Window_TransparentFacade,
                                         ThermFile::WindowCrossSectionType::VerticalMeetingRail};

    Helper::MockNode node{"Exposure"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << exposureType;

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), Helper::exposureTypeWindowNode("Vertical Meeting Rail")));
}

TEST_F(TestFileProperties, ExposureTypeOpaqueFacadeDeserialization)
{
    auto elementNode(Helper::exposureTypeOpaqueNode("End Section", "Wall"));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::ExposureType exposureType;
    adapter >> exposureType;

    ThermFile::ExposureType correctExposureType{ThermFile::ModelPurpose::OpaqueFacade,
                                                ThermFile::OpaqueCrossSectionType::EndSection,
                                                ThermFile::OpaqueAssemblyType::Wall};
    Helper::expect_eq(correctExposureType, exposureType);
}

TEST_F(TestFileProperties, ExposureTypeOpaqueFacadeSerialization)
{
    ThermFile::ExposureType exposureType{ThermFile::ModelPurpose::OpaqueFacade,
                                         ThermFile::OpaqueCrossSectionType::RoughOpeningHeader,
                                         ThermFile::OpaqueAssemblyType::Wall};

    Helper::MockNode node{"Exposure"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << exposureType;

    EXPECT_TRUE(
      Helper::compareNodes(adapter.getNode(), Helper::exposureTypeOpaqueNode("Rough Opening - Header", "Wall")));
}

TEST_F(TestFileProperties, ExposureTypeOtherDeserialization)
{
    auto elementNode(Helper::exposureTypeOtherNode("General Cross Section"));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::ExposureType exposureType;
    adapter >> exposureType;

    ThermFile::ExposureType correctExposureType{ThermFile::ModelPurpose::Other,
                                                ThermFile::OtherCrossSectionType::GeneralCrossSection};
    Helper::expect_eq(correctExposureType, exposureType);
}

TEST_F(TestFileProperties, ExposureTypeOtherSerialization)
{
    ThermFile::ExposureType exposureType{ThermFile::ModelPurpose::Other,
                                         ThermFile::OtherCrossSectionType::CommonThermalBridge};

    Helper::MockNode node{"Exposure"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << exposureType;

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), Helper::exposureTypeOtherNode("Common Thermal Bridge")));
}

TEST_F(TestFileProperties, ModelExposureDeserialization)
{
    auto elementNode(
      Helper::modelExposureNode({"112", "Down", ThermFile::ModelPurpose::OpaqueFacade, "End Section", "Wall"}));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::ModelExposure modelExposure;
    adapter >> modelExposure;

    ThermFile::ModelExposure correctExposure{112,
                                             ThermFile::Gravity::Orientation::Down,
                                             {ThermFile::ModelPurpose::OpaqueFacade,
                                              ThermFile::OpaqueCrossSectionType::EndSection,
                                              ThermFile::OpaqueAssemblyType::Wall}};

    Helper::expect_eq(correctExposure, modelExposure);
}

TEST_F(TestFileProperties, MiscellaneousSerialization)
{
    ThermFile::Miscellaneous miscellaneous{0.03, 0.01};

    Helper::MockNode node{"Miscellaneous"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << miscellaneous;

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), Helper::miscellaneousNode({"0.03", "0.01"})));
}

TEST_F(TestFileProperties, MiscellaneousDeserialization)
{
    auto elementNode(Helper::miscellaneousNode({"0.08", "0.03"}));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::Miscellaneous miscellaneous;
    adapter >> miscellaneous;

    ThermFile::Miscellaneous correctMiscellaneous{0.08, 0.03};
    Helper::expect_near(correctMiscellaneous, miscellaneous, 1e-6);
}

TEST_F(TestFileProperties, FilePropertiesSerialization)
{
    ThermFile::Properties fileProperties{Helper::FileProperties1::testObject()};

    Helper::MockNode node{"Properties"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << fileProperties;

    auto correctNodes{Helper::FileProperties1::equivalentNodeStructure()};

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes));
}

TEST_F(TestFileProperties, FilePropertiesDeserialization)
{
    auto mockData{Helper::FileProperties1::equivalentNodeStructure()};

    auto elementNode(mockData);
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::Properties fileProperties;
    adapter >> fileProperties;

    ThermFile::Properties correctFileProperties{Helper::FileProperties1::testObject()};

    constexpr auto tolerance{1e-6};
    Helper::expect_near(correctFileProperties, fileProperties, tolerance);
}