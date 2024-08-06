#include "TestExamples.hxx"

#include "PropertiesHelper.hxx"
#include "PreferencesHelper.hxx"

#include "Common/Common.hxx"

namespace Helper
{
    namespace FileProperties1
    {
        ThermFile::Properties testObject()
        {
            ThermFile::General general{"18",
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
            ThermFile::CalculationOptions calculationOptions{
              ThermFile::SimulationEngine::HygroThermFEM,
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
            ThermFile::ModelExposure exposure{112,
                                              ThermFile::Gravity::Orientation::Down,
                                              {ThermFile::ModelPurpose::OpaqueFacade,
                                               ThermFile::OpaqueCrossSectionType::EndSection,
                                               ThermFile::OpaqueAssemblyType::Wall}};
            ThermFile::Miscellaneous miscellaneous{0.1, 0.01};

            return {general, calculationOptions, exposure, miscellaneous};
        }

        Helper::MockNode equivalentNodeStructure()
        {
            Helper::MockNode node{"Properties"};

            addChildNode(node,
                         Helper::generalNode({"18",
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

            addChildNode(node,
                         Helper::calculationOptionsNode({"HygroThermFEM",
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
                                                         {"false", "true", "true"}}));
            addChildNode(
              node,
              Helper::modelExposureNode({"112", "Down", ThermFile::ModelPurpose::OpaqueFacade, "End Section", "Wall"}));

            addChildNode(node, Helper::miscellaneousNode({"0.1", "0.01"}));

            return node;
        }
    }   // namespace FileProperties1

    namespace THMXFile1
    {
        ThermFile::Cavity cavity1()
        {
            return {"7a863ad6-c537-11ea-87d0-0242ac130003",
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
        }

        Helper::MockNode mockCavity1()
        {
            return Helper::generateCavityWithoutDirectionNode({"7a863ad6-c537-11ea-87d0-0242ac130003",
                                                               "nul",
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
                                                               {"2.3", "2.6"},
                                                               {"3.8", "4.12"}});
        }

        ThermFile::Cavity cavity2()
        {
            return {"7a863ad6-c537-11ea-87d0-0242ac130004",
                    std::nullopt,
                    ThermFile::Direction::Up,
                    0.8,
                    0.7,
                    25.0,
                    30.0,
                    0.6,
                    0.55,
                    0.65,
                    0.35,
                    false,
                    101325.0,
                    {2.6, 2.9},
                    {4.12, 4.44}};
        }

        Helper::MockNode mockCavity2()
        {
            return Helper::generateCavityWithoutDirectionNode({"7a863ad6-c537-11ea-87d0-0242ac130004",
                                                               "nul",
                                                               "Up",
                                                               "0.8",
                                                               "0.7",
                                                               "25",
                                                               "30",
                                                               "0.6",
                                                               "0.55",
                                                               "0.65",
                                                               "0.35",
                                                               "false",
                                                               "1.01325e+05",
                                                               {"2.6", "2.9"},
                                                               {"4.12", "4.44"}});
        }

        ThermFile::Polygon polygon1()
        {
            return {"Some uuid",
                    12,
                    "Material uuid",
                    "Some Name",
                    ThermFile::GlazingSystemData{1, 0},
                    {0.2, 0.4},
                    {{20.0, 0.5}, {25.0, 0.6}, {30.0, 0.7}},
                    "Some Cavity uuid",
                    {"Attribute1", "Attribute2"},
                    ThermFile::PolygonType::Material};
        }

        Helper::MockNode mockPolygon1()
        {
            return Helper::generatePolygonNode({"Some uuid",
                                                "12",
                                                "Material uuid",
                                                "Some Name",
                                                {"1", "0"},
                                                {"0.2", "0.4"},
                                                {{"20", "0.5"}, {"25", "0.6"}, {"30", "0.7"}},
                                                "Some Cavity uuid",
                                                {"Attribute1", "Attribute2"},
                                                "Material"});
        }

        ThermFile::Polygon polygon2()
        {
            return {"Some other uuid",
                    23,
                    "Some other Material uuid",
                    "Some other Name",
                    ThermFile::GlazingSystemData{2, 1},
                    {0.3, 0.5},
                    {{25.0, 0.6}, {30.0, 0.7}, {35.0, 0.8}},
                    "Some other Cavity uuid",
                    {"Attribute3", "Attribute4"},
                    ThermFile::PolygonType::Glass};
        }

        Helper::MockNode mockPolygon2()
        {
            return Helper::generatePolygonNode({"Some other uuid",
                                                "23",
                                                "Some other Material uuid",
                                                "Some other Name",
                                                {"2", "1"},
                                                {"0.3", "0.5"},
                                                {{"25", "0.6"}, {"30", "0.7"}, {"35", "0.8"}},
                                                "Some other Cavity uuid",
                                                {"Attribute3", "Attribute4"},
                                                "Glass"});
        }

        ThermFile::Boundary bc1()
        {
            return {14u,
                    "Some UUID",
                    "Some Name",
                    "Some Flux Tag",
                    true,
                    "Some Neighbor Polygon UUID",
                    {20.0, 0.5},
                    {{20.0, 0.5}, {25.0, 0.6}, {30.0, 0.7}},
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
                    "0xFF0000",
                    7};
        }

        Helper::MockNode mockBC1()
        {
            return Helper::generateBoundaryNode({"14",
                                                 "Some UUID",
                                                 "Some Name",
                                                 "Some Flux Tag",
                                                 "true",
                                                 "Some Neighbor Polygon UUID",
                                                 {"20", "0.5"},
                                                 {{"20", "0.5"}, {"25", "0.6"}, {"30", "0.7"}},
                                                 "1",
                                                 {"0.9", "20", "true"},
                                                 {"Interior", "125"},
                                                 "true",
                                                 "",
                                                 Helper::MockTransientData{"7a863ad6-c537-11ea-87d0-0242ac130003",
                                                                           "SomeFileName",
                                                                           Helper::MockSurfaceData{"45", "2000"},
                                                                           Helper::MockBuildingData{"25", "10"}},
                                                 "2",
                                                 "",
                                                 "Boundary Condition",
                                                 "0xFF0000",
                                                 "7"});
        }

        ThermFile::Boundary bc2()
        {
            return {15u,
                    "Some other UUID",
                    "Some other Name",
                    "Some other Flux Tag",
                    false,
                    "Some other Neighbor Polygon UUID",
                    {25.0, 0.6},
                    {{25.0, 0.6}, {30.0, 0.7}, {35.0, 0.8}},
                    2u,
                    {0.8, 25.0, false},
                    ThermFile::ShadeData{ThermFile::ShadeModifier::Exterior, 150},
                    false,
                    std::nullopt,
                    ThermFile::TransientData{"7a863ad6-c537-11ea-87d0-0242ac130004",
                                             "Some other FileName",
                                             ThermFile::SurfaceData{50.0, 2500.0},
                                             ThermFile::BuildingData{30.0, 15.0}},
                    3u,
                    std::nullopt,
                    ThermFile::SurfaceType::GlazingCavity,
                    "0x00FF00",
                    9};
        }

        Helper::MockNode mockBC2()
        {
            return Helper::generateBoundaryNode(
              {"15",
               "Some other UUID",
               "Some other Name",
               "Some other Flux Tag",
               "false",
               "Some other Neighbor Polygon UUID",
               {"25", "0.6"},
               {{"25", "0.6"}, {"30", "0.7"}, {"35", "0.8"}},
               "2",
               {"0.8", "25", "false"},
               {"Exterior", "150"},
               "false",
               "",
               {"7a863ad6-c537-11ea-87d0-0242ac130004", "Some other FileName", {"50", "2500"}, {"30", "15"}},
               "3",
               "",
               "Glazing Cavity",
               "0x00FF00",
               "9"});
        }

        ThermFile::ThermModel testObject()
        {
            ThermFile::Preferences preferences{
              ThermFile::Underlay{
                "UnderlayName", true, ThermFile::UnitSystem::SI, {1.0, 2.0}, ThermFile::UnderlayType::None, 1.0, 1.0},
              ThermFile::Settings{{1.0, 2.0},
                                  3.0,
                                  ThermFile::ConductivityUnits::Btu_InchPerHour_FootSquared_Fahrenheit,
                                  ThermFile::UValueBasisType::YProjected,
                                  false}};
            const auto properties{FileProperties1::testObject()};
            const auto glazingOrigin{ThermFile::Point{0.3, 1.8}};
            std::vector<ThermFile::Cavity> cavities{cavity1(), cavity2()};
            std::vector<ThermFile::Polygon> polygons{polygon1(), polygon2()};
            std::vector<ThermFile::Boundary> bcs{bc1(), bc2()};

            return {"1", false, preferences, properties, glazingOrigin, cavities, polygons, bcs};
        }

        Helper::MockNode equivalentNodeStructure()
        {
            Helper::MockNode node{"ThermModel"};

            addChildNode(node, "Version", "1");
            addChildNode(node, "CalculationReady", "false");

            addChildNode(
              node,
              Helper::generatePreferencesNode(
                {{"UnderlayName", "true", "SI", {"1", "2"}, "None", "1", "1"},
                 {{"1", "2"}, "3", "Btu-in/hr-ft2-F", "Y-projected", "false"},
                 {"false", "false", "false", "false", "false", "false", "false", "false", "0", "0", "0", {"0", "0"}}}));

            addChildNode(node, FileProperties1::equivalentNodeStructure());
            auto glazingOrigin{Helper::generatePointNode({"0.3", "1.8"})};
            glazingOrigin.tag = "GlazingOrigin";
            addChildNode(node, glazingOrigin);

            auto & cavitiesNode{addChildNode(node, "Cavities")};
            addChildNode(cavitiesNode, mockCavity1());
            addChildNode(cavitiesNode, mockCavity2());

            auto & polygonsNode{addChildNode(node, "Polygons")};
            addChildNode(polygonsNode, mockPolygon1());
            addChildNode(polygonsNode, mockPolygon2());

            auto & bcsNode{addChildNode(node, "Boundaries")};
            addChildNode(bcsNode, mockBC1());
            addChildNode(bcsNode, mockBC2());

            return node;
        }
    }   // namespace THMXFile1
}   // namespace Helper