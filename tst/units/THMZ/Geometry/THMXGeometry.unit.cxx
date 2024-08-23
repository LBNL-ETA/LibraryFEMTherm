#include "gtest/gtest.h"

#include <vector>

#include "THMZ/Geometry/Geometry.hxx"
#include "THMZ/Geometry/Serializers.hxx"

#include "MockNodeAdapter.hxx"
#include "THMXGeometryHelper.hxx"

class TestTHMXGeometry : public testing::Test
{};

TEST_F(TestTHMXGeometry, PointDeserialization)
{
    auto elementNode(Helper::generatePointNode({"20", "0.5"}));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::Point point;
    adapter >> point;

    ThermFile::Point correctPoint{20.0, 0.5};
    Helper::expect_near(correctPoint, point, 1e-6);
}

TEST_F(TestTHMXGeometry, PointSerialization)
{
    ThermFile::Point point{32.7, 0.65};

    Helper::MockNode node{"Point"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << point;

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), Helper::generatePointNode({"32.7", "0.65"})));
}

TEST_F(TestTHMXGeometry, CavityDeserialization)
{
    auto node{Helper::generateCavityWithoutDirectionNode({"7a863ad6-c537-11ea-87d0-0242ac130003",
                                                          "Up",
                                                          "Down",
                                                          "0.9",
                                                          "0.8",
                                                          "20",
                                                          "25",
                                                          "0.5",
                                                          "0.45",
                                                          "0.55",
                                                          "0.25",
                                                          "true",
                                                          "101325",
                                                          {"2.3", "2.6"},
                                                          {"3.8", "4.12"}})};
    const Helper::MockNodeAdapter adapter{&node};

    ThermFile::Cavity cavity;
    adapter >> cavity;

    ThermFile::Cavity correctCavity{"7a863ad6-c537-11ea-87d0-0242ac130003",
                                    std::nullopt,
                                    ThermFile::Direction::Down,
                                    0.9,
                                    0.8,
                                    20.0,
                                    25.0,
                                    0.5,
                                    0.45,
                                    0.55,
                                    0.25,
                                    true,
                                    101325.0,
                                    {2.3, 2.6},
                                    {3.8, 4.12}};

    Helper::expect_near(correctCavity, cavity, 1e-6);
}

TEST_F(TestTHMXGeometry, CavitySerialization)
{
    ThermFile::Cavity cavity{"7a863ad6-c537-11ea-87d0-0242ac130003",
                             std::nullopt,
                             ThermFile::Direction::Down,
                             0.9,
                             0.8,
                             20.0,
                             25.0,
                             0.5,
                             0.45,
                             0.55,
                             0.25,
                             true,
                             101325.0,
                             {1.1, 1.2},
                             {1.3, 1.4}};

    Helper::MockNode node{"Cavity"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << cavity;

    auto correctNode{Helper::generateCavityWithoutDirectionNode({"7a863ad6-c537-11ea-87d0-0242ac130003",
                                                                 "Up",
                                                                 "Down",
                                                                 "0.9",
                                                                 "0.8",
                                                                 "20",
                                                                 "25",
                                                                 "0.5",
                                                                 "0.45",
                                                                 "0.55",
                                                                 "0.25",
                                                                 "true",
                                                                 "1.01325e+05",
                                                                 {"1.1", "1.2"},
                                                                 {"1.3", "1.4"}})};

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNode));
}

TEST_F(TestTHMXGeometry, CavityWithDirectionDeserialization)
{
    auto node{Helper::generateCavityNodeWithDirection({"7a863ad6-c537-11ea-87d0-0242ac130003",
                                                       "Up",
                                                       "Down",
                                                       "0.9",
                                                       "0.8",
                                                       "20",
                                                       "25",
                                                       "0.5",
                                                       "0.45",
                                                       "0.55",
                                                       "0.25",
                                                       "true",
                                                       "101325",
                                                       {"0.1", "0.2"},
                                                       {"0.3", "0.4"}})};
    const Helper::MockNodeAdapter adapter{&node};

    ThermFile::Cavity cavity;
    adapter >> cavity;

    ThermFile::Cavity correctCavity{"7a863ad6-c537-11ea-87d0-0242ac130003",
                                    ThermFile::Direction::Up,
                                    ThermFile::Direction::Down,
                                    0.9,
                                    0.8,
                                    20.0,
                                    25.0,
                                    0.5,
                                    0.45,
                                    0.55,
                                    0.25,
                                    true,
                                    101325.0,
                                    {0.1, 0.2},
                                    {0.3, 0.4}};

    Helper::expect_near(correctCavity, cavity, 1e-6);
}

TEST_F(TestTHMXGeometry, CavityWithDirectionSerialization)
{
    ThermFile::Cavity cavity{"7a863ad6-c537-11ea-87d0-0242ac130003",
                             ThermFile::Direction::Left,
                             ThermFile::Direction::Up,
                             0.9,
                             0.8,
                             20.0,
                             25.0,
                             0.5,
                             0.45,
                             0.55,
                             0.25,
                             true,
                             101325.0,
                             {0.1, 0.2},
                             {0.3, 0.4}};

    Helper::MockNode node{"Cavity"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << cavity;

    auto correctNode{Helper::generateCavityNodeWithDirection({"7a863ad6-c537-11ea-87d0-0242ac130003",
                                                              "Left",
                                                              "Up",
                                                              "0.9",
                                                              "0.8",
                                                              "20",
                                                              "25",
                                                              "0.5",
                                                              "0.45",
                                                              "0.55",
                                                              "0.25",
                                                              "true",
                                                              "1.01325e+05",
                                                              {"0.1", "0.2"},
                                                              {"0.3", "0.4"}})};

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNode));
}

TEST_F(TestTHMXGeometry, PolygonDeserialization)
{
    auto node{Helper::generatePolygonNode({"Some uuid",
                                           "38",
                                           "Some material uuid",
                                           "Some Name",
                                           {"2", "1"},
                                           {"0.2", "0.4"},
                                           {{"20", "0.5"}, {"25", "0.6"}, {"30", "0.7"}},
                                           "Some Cavity uuid",
                                           {"Attribute1", "Attribute2"},
                                           "Glass"})};
    const Helper::MockNodeAdapter adapter{&node};

    ThermFile::Polygon polygon;
    adapter >> polygon;

    ThermFile::Polygon correctPolygon{"Some uuid",
                                      38,
                                      "Some material uuid",
                                      "Some Name",
                                      ThermFile::GlazingSystemData{2, 1},
                                      {0.2, 0.4},
                                      {{20.0, 0.5}, {25.0, 0.6}, {30.0, 0.7}},
                                      "Some Cavity uuid",
                                      {"Attribute1", "Attribute2"},
                                      ThermFile::PolygonType::Glass,
                                      std::nullopt};

    Helper::expect_near(correctPolygon, polygon, 1e-6);
}

TEST_F(TestTHMXGeometry, PolygonSerialization)
{
    ThermFile::Polygon polygon{"Some uuid",
                               45,
                               "Some material uuid",
                               "Some Name",
                               ThermFile::GlazingSystemData{2, 2},
                               {0.2, 0.4},
                               {{20.0, 0.5}, {25.0, 0.6}, {30.0, 0.7}},
                               "Some Cavity uuid",
                               {"Attribute1", "Attribute2"},
                               ThermFile::PolygonType::Material, std::nullopt};

    Helper::MockNode node{"Polygon"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << polygon;

    auto correctNode{Helper::generatePolygonNode({"Some uuid",
                                                  "45",
                                                  "Some material uuid",
                                                  "Some Name",
                                                  {"2", "2"},
                                                  {"0.2", "0.4"},
                                                  {{"20", "0.5"}, {"25", "0.6"}, {"30", "0.7"}},
                                                  "Some Cavity uuid",
                                                  {"Attribute1", "Attribute2"},
                                                  "Material"})};

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNode));
}

TEST_F(TestTHMXGeometry, SurfaceDataDeserialization)
{
    auto node{Helper::generateSurfaceDataNode({"45", "2000"})};
    const Helper::MockNodeAdapter adapter{&node};

    ThermFile::SurfaceData surfaceData;
    adapter >> surfaceData;

    ThermFile::SurfaceData correctSurfaceData{45.0, 2000.0};

    Helper::expect_near(correctSurfaceData, surfaceData, 1e-6);
}

TEST_F(TestTHMXGeometry, SurfaceDataSerialization)
{
    ThermFile::SurfaceData surfaceData{55.0, 1890.0};

    Helper::MockNode node{"Surface"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << surfaceData;

    auto correctNode{Helper::generateSurfaceDataNode({"55", "1890"})};

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNode));
}

TEST_F(TestTHMXGeometry, BuildingDataDeserialization)
{
    auto node{Helper::generateBuildingDataNode({"45", "2000"})};
    const Helper::MockNodeAdapter adapter{&node};

    ThermFile::BuildingData buildingData;
    adapter >> buildingData;

    ThermFile::BuildingData correctBuildingData{45.0, 2000.0};

    Helper::expect_near(correctBuildingData, buildingData, 1e-6);
}

TEST_F(TestTHMXGeometry, BuildingDataSerialization)
{
    ThermFile::BuildingData buildingData{55.0, 1890.0};

    Helper::MockNode node{"Building"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << buildingData;

    auto correctNode{Helper::generateBuildingDataNode({"55", "1890"})};

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNode));
}

TEST_F(TestTHMXGeometry, ThermalEmissionPropertiesDeserialization)
{
    auto node{Helper::generateThermalEmissionPropertiesNode({"0.9", "20", "true"})};
    const Helper::MockNodeAdapter adapter{&node};

    ThermFile::ThermalEmissionProperties thermalEmissionProperties;
    adapter >> thermalEmissionProperties;

    ThermFile::ThermalEmissionProperties correctThermalEmissionProperties{0.9, 20.0, true};

    Helper::expect_near(correctThermalEmissionProperties, thermalEmissionProperties, 1e-6);
}

TEST_F(TestTHMXGeometry, ThermalEmissionPropertiesSerialization)
{
    ThermFile::ThermalEmissionProperties thermalEmissionProperties{0.8, 25.0, false};

    Helper::MockNode node{"ThermalEmissionProperties"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << thermalEmissionProperties;

    auto correctNode{Helper::generateThermalEmissionPropertiesNode({"0.8", "25", "false"})};

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNode));
}

TEST_F(TestTHMXGeometry, ShadeDataDeserialization)
{
    auto node{Helper::generateShadeDataNode({"Interior", "125"})};
    const Helper::MockNodeAdapter adapter{&node};

    ThermFile::ShadeData shadeData;
    adapter >> shadeData;

    ThermFile::ShadeData correctShadeData{ThermFile::ShadeModifier::Interior, 125};

    Helper::expect_eq(correctShadeData, shadeData);
}

TEST_F(TestTHMXGeometry, ShadeDataSerialization)
{
    ThermFile::ShadeData shadeData{ThermFile::ShadeModifier::Exterior, 200};

    Helper::MockNode node{"ShadeData"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << shadeData;

    auto correctNode{Helper::generateShadeDataNode({"Exterior", "200"})};

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNode));
}

TEST_F(TestTHMXGeometry, TransientDataDeserialization)
{
    auto node{Helper::generateTransientDataNode(
      {"7a863ad6-c537-11ea-87d0-0242ac130003", "SomeFileName", {"45", "2000"}, {"25", "10"}})};
    const Helper::MockNodeAdapter adapter{&node};

    ThermFile::TransientData transientData;
    adapter >> transientData;

    ThermFile::TransientData correctTransientData{"7a863ad6-c537-11ea-87d0-0242ac130003",
                                                  "SomeFileName",
                                                  ThermFile::SurfaceData{45.0, 2000.0},
                                                  ThermFile::BuildingData{25.0, 10.0}};

    Helper::expect_near(correctTransientData, transientData, 1e-6);
}

TEST_F(TestTHMXGeometry, TransientDataSerialization)
{
    ThermFile::TransientData transientData{"7a863ad6-c537-11ea-87d0-0242ac130003",
                                           "SomeFileName",
                                           ThermFile::SurfaceData{45.0, 2000.0},
                                           ThermFile::BuildingData{25.0, 10.0}};

    Helper::MockNode node{"Transient"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << transientData;

    auto correctNode{Helper::generateTransientDataNode(
      {"7a863ad6-c537-11ea-87d0-0242ac130003", "SomeFileName", {"45", "2000"}, {"25", "10"}})};

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNode));
}

TEST_F(TestTHMXGeometry, BoundaryConditionDeserialization)
{
    auto node{Helper::generateBoundaryNode(
      {"14",
       "Some UUID",
       "Some Name",
       "Some Flux Tag",
       "true",
       "Some Neighbor Polygon UUID",
       {"20", "0.5"},
       {"20", "0.5"},
       {"25", "0.6"},
       "1",
       {"0.9", "20", "true"},
       {"Interior", "125"},
       "true",
       "",
       {"7a863ad6-c537-11ea-87d0-0242ac130003", "SomeFileName", {"45", "2000"}, {"25", "10"}},
       "2",
       "",
       "Boundary Condition",
       "OxFF0000",
       "4"})};
    const Helper::MockNodeAdapter adapter{&node};

    ThermFile::Boundary boundaryCondition;
    adapter >> boundaryCondition;

    ThermFile::Boundary correctBoundaryCondition{14u,
                                                 "Some UUID",
                                                 "Some Name",
                                                 "Some Flux Tag",
                                                 true,
                                                 "Some Neighbor Polygon UUID",
                                                 std::nullopt,
                                                 {20.0, 0.5},
                                                 {20.0, 0.5},
                                                 {25.0, 0.6},
                                                 1u,
                                                 {0.9, 20.0, true},
                                                 ThermFile::ShadeData{ThermFile::ShadeModifier::Interior, 125},
                                                 true,
                                                 std::nullopt,
                                                 ThermFile::TransientData{"7a863ad6-c537-11ea-87d0-0242ac130003",
                                                                          "SomeFileName",
                                                                          ThermFile::SurfaceData{45.0, 2000.0},
                                                                          ThermFile::BuildingData{25.0, 10.0}},
                                                 2u,
                                                 std::nullopt,
                                                 ThermFile::SurfaceType::BoundaryCondition,
                                                 "OxFF0000",
                                                 4};

    Helper::expect_near(correctBoundaryCondition, boundaryCondition, 1e-6);
}

TEST_F(TestTHMXGeometry, BoundaryConditionSerialization)
{
    ThermFile::Boundary boundaryCondition{14u,
                                          "Some UUID",
                                          "Some Name",
                                          "Some Flux Tag",
                                          true,
                                          "Some Neighbor Polygon UUID",
                                          std::nullopt,
                                          {20.0, 0.5},
                                          {20.0, 0.5},
                                          {25.0, 0.6},
                                          1u,
                                          {0.9, 20.0, true},
                                          ThermFile::ShadeData{ThermFile::ShadeModifier::Interior, 125},
                                          true,
                                          std::nullopt,
                                          ThermFile::TransientData{"7a863ad6-c537-11ea-87d0-0242ac130003",
                                                                   "SomeFileName",
                                                                   ThermFile::SurfaceData{45.0, 2000.0},
                                                                   ThermFile::BuildingData{25.0, 10.0}},
                                          2u,
                                          std::nullopt,
                                          ThermFile::SurfaceType::BoundaryCondition,
                                          "OxFF0000",
                                          5};

    Helper::MockNode node{"Boundary"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << boundaryCondition;

    auto correctNode{Helper::generateBoundaryNode(
      {"14",
       "Some UUID",
       "Some Name",
       "Some Flux Tag",
       "true",
       "Some Neighbor Polygon UUID",
       {"20", "0.5"},
       {"20", "0.5"},
       {"25", "0.6"},
       "1",
       {"0.9", "20", "true"},
       {"Interior", "125"},
       "true",
       "",
       {"7a863ad6-c537-11ea-87d0-0242ac130003", "SomeFileName", {"45", "2000"}, {"25", "10"}},
       "2",
       "",
       "Boundary Condition",
       "OxFF0000",
       "5"})};

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNode));
}