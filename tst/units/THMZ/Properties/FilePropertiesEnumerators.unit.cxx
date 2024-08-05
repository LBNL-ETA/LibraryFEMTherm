#include "gtest/gtest.h"

#include <variant>

#include <fileParse/Common.hxx>

#include "THMZ/Properties/Properties.hxx"
#include "THMZ/Properties/EnumSerializers.hxx"

#include "HelperUtility.hxx"
#include "MockNodeAdapter.hxx"

class TestFilePropertiesEnumerators : public testing::Test
{};

TEST_F(TestFilePropertiesEnumerators, AllOrientationStrings)
{
    const std::vector<std::string> correct{"Down", "Up", "Left", "Right", "Into Screen", "Out Of Screen"};

    const auto orientationStrings{ThermFile::Gravity::orientationStrings()};

    Helper::checkVectorEquality(correct, orientationStrings);
}

TEST_F(TestFilePropertiesEnumerators, GravityOrientationDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "GravityOrientation", "Into Screen");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::Gravity::Orientation orientation;
    adapter >> FileParse::Child{"GravityOrientation", orientation};

    EXPECT_EQ(static_cast<int>(ThermFile::Gravity::Orientation::IntoScreen), static_cast<int>(orientation));
}

TEST_F(TestFilePropertiesEnumerators, GravityOrientationSerialization)
{
    ThermFile::Gravity::Orientation orientation{ThermFile::Gravity::Orientation::OutOfScreen};

    Helper::MockNode elementNode("FileProperties");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"GravityOrientation", orientation};

    auto correctNodes = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "GravityOrientation", "Out Of Screen");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestFilePropertiesEnumerators, AllModelPurposesStrings)
{
    const std::vector<std::string> correct{"Window/Transparent Facade", "Opaque Facade", "Other"};

    const auto modelPurposesStrings{ThermFile::modelPurposes()};

    Helper::checkVectorEquality(correct, modelPurposesStrings);
}

TEST_F(TestFilePropertiesEnumerators, ModelPurposeDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "ModelPurpose", "Opaque Facade");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::ModelPurpose modelPurpose;
    adapter >> FileParse::Child{"ModelPurpose", modelPurpose};

    EXPECT_EQ(static_cast<int>(ThermFile::ModelPurpose::OpaqueFacade), static_cast<int>(modelPurpose));
}

TEST_F(TestFilePropertiesEnumerators, ModelPurposeSerialization)
{
    ThermFile::ModelPurpose modelPurpose{ThermFile::ModelPurpose::Window_TransparentFacade};

    Helper::MockNode elementNode("FileProperties");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"ModelPurpose", modelPurpose};

    auto correctNodes = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "ModelPurpose", "Window/Transparent Facade");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestFilePropertiesEnumerators, SimulationEngineDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "SimulationEngine", "HygroThermFEM");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::SimulationEngine simulationEngine;
    adapter >> FileParse::Child{"SimulationEngine", simulationEngine};

    EXPECT_EQ(static_cast<int>(ThermFile::SimulationEngine::HygroThermFEM), static_cast<int>(simulationEngine));
}

TEST_F(TestFilePropertiesEnumerators, SimulationEngineSerialization)
{
    ThermFile::SimulationEngine simulationEngine{ThermFile::SimulationEngine::Conrad};

    Helper::MockNode elementNode("FileProperties");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"SimulationEngine", simulationEngine};

    auto correctNodes = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "SimulationEngine", "Conrad");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestFilePropertiesEnumerators, CalculationModeDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "CalculationMode", "Transient");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::CalculationMode calculationMode;
    adapter >> FileParse::Child{"CalculationMode", calculationMode};

    EXPECT_EQ(static_cast<int>(ThermFile::CalculationMode::cmTransient), static_cast<int>(calculationMode));
}

TEST_F(TestFilePropertiesEnumerators, CalculationModeSerialization)
{
    ThermFile::CalculationMode calculationMode{ThermFile::CalculationMode::cmSteadyState};

    Helper::MockNode elementNode("FileProperties");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"CalculationMode", calculationMode};

    auto correctNodes = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "CalculationMode", "Steady-State");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestFilePropertiesEnumerators, TransientCalculationMethodologyDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "TransientCalculationMethodology", "Use first timestep");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::TransientCalculationMethodology transientCalculationMethodology;
    adapter >> FileParse::Child{"TransientCalculationMethodology", transientCalculationMethodology};

    EXPECT_EQ(static_cast<int>(ThermFile::TransientCalculationMethodology::icUseFirstTimestep),
              static_cast<int>(transientCalculationMethodology));
}

TEST_F(TestFilePropertiesEnumerators, TransientCalculationMethodologySerialization)
{
    ThermFile::TransientCalculationMethodology transientCalculationMethodology{
      ThermFile::TransientCalculationMethodology::icConstantEverywhere};

    Helper::MockNode elementNode("FileProperties");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"TransientCalculationMethodology", transientCalculationMethodology};

    auto correctNodes = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "TransientCalculationMethodology", "Constant everywhere");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestFilePropertiesEnumerators, MesherTypeDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "MesherType", "QuadTree Mesher");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::MesherType mesherType;
    adapter >> FileParse::Child{"MesherType", mesherType};

    EXPECT_EQ(static_cast<int>(ThermFile::MesherType::QuadTreeMesher), static_cast<int>(mesherType));
}

TEST_F(TestFilePropertiesEnumerators, MesherTypeSerialization)
{
    ThermFile::MesherType mesherType{ThermFile::MesherType::Simmetrix_Version_2022};

    Helper::MockNode elementNode("FileProperties");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"MesherType", mesherType};

    auto correctNodes = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "MesherType", "Simmetrix Version 2022");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestFilePropertiesEnumerators, SteadyStateCalculationMethodologyDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "SteadyStateCalculationMethodology", "No time variable");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::SteadyStateCalculationMethodology steadyStateCalculationMethodology;
    adapter >> FileParse::Child{"SteadyStateCalculationMethodology", steadyStateCalculationMethodology};

    EXPECT_EQ(static_cast<int>(ThermFile::SteadyStateCalculationMethodology::ssNoTimeVariable),
              static_cast<int>(steadyStateCalculationMethodology));
}

TEST_F(TestFilePropertiesEnumerators, SteadyStateCalculationMethodologySerialization)
{
    ThermFile::SteadyStateCalculationMethodology steadyStateCalculationMethodology{
      ThermFile::SteadyStateCalculationMethodology::ssFalseTransient};

    Helper::MockNode elementNode("FileProperties");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"SteadyStateCalculationMethodology", steadyStateCalculationMethodology};

    auto correctNodes = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "SteadyStateCalculationMethodology", "False transient");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestFilePropertiesEnumerators, OrientationDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "Orientation", "East");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::Orientation orientation;
    adapter >> FileParse::Child{"Orientation", orientation};

    EXPECT_EQ(static_cast<int>(ThermFile::Orientation::East), static_cast<int>(orientation));
}

TEST_F(TestFilePropertiesEnumerators, OrientationSerialization)
{
    ThermFile::Orientation orientation{ThermFile::Orientation::North};

    Helper::MockNode elementNode("FileProperties");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"Orientation", orientation};

    auto correctNodes = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "Orientation", "North");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestFilePropertiesEnumerators, AllWindowCrossSectionStrings)
{
    const std::vector<std::string> correct{"Sill",
                                           "Jamb",
                                           "Head",
                                           "Horizontal Divider",
                                           "Vertical Divider",
                                           "Horizontal Meeting Rail",
                                           "Vertical Meeting Rail",
                                           "Common Frame",
                                           "Spacer"};

    const auto windowCrossSectionStrings{ThermFile::windowCrossSectionStrings()};

    Helper::checkVectorEquality(correct, windowCrossSectionStrings);
}

TEST_F(TestFilePropertiesEnumerators, IsJambType)
{
    const std::set<ThermFile::WindowCrossSectionType> jambTypes{ThermFile::WindowCrossSectionType::Jamb,
                                                                ThermFile::WindowCrossSectionType::VerticalDivider,
                                                                ThermFile::WindowCrossSectionType::VerticalMeetingRail};

    for(const auto & type : jambTypes)
    {
        EXPECT_TRUE(ThermFile::isJambType(type));
    }
}

TEST_F(TestFilePropertiesEnumerators, WindowCrossSectionTypeDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "WindowCrossSectionType", "Jamb");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::WindowCrossSectionType windowCrossSectionType;
    adapter >> FileParse::Child{"WindowCrossSectionType", windowCrossSectionType};

    EXPECT_EQ(static_cast<int>(ThermFile::WindowCrossSectionType::Jamb), static_cast<int>(windowCrossSectionType));
}

TEST_F(TestFilePropertiesEnumerators, WindowCrossSectionTypeSerialization)
{
    ThermFile::WindowCrossSectionType windowCrossSectionType{ThermFile::WindowCrossSectionType::Spacer};

    Helper::MockNode elementNode("FileProperties");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"WindowCrossSectionType", windowCrossSectionType};

    auto correctNodes = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "WindowCrossSectionType", "Spacer");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestFilePropertiesEnumerators, AllOpaqueAssemblyTypeStrings)
{
    const std::vector<std::string> correct{"Wall", "Roof"};

    const auto opaqueAssemblyTypeStrings{ThermFile::opaqueAssemblyTypeStrings()};

    Helper::checkVectorEquality(correct, opaqueAssemblyTypeStrings);
}

TEST_F(TestFilePropertiesEnumerators, OpaqueAssemblyTypeDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "OpaqueAssemblyType", "Roof");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::OpaqueAssemblyType opaqueAssemblyType;
    adapter >> FileParse::Child{"OpaqueAssemblyType", opaqueAssemblyType};

    EXPECT_EQ(static_cast<int>(ThermFile::OpaqueAssemblyType::Roof), static_cast<int>(opaqueAssemblyType));
}

TEST_F(TestFilePropertiesEnumerators, OpaqueAssemblyTypeSerialization)
{
    ThermFile::OpaqueAssemblyType opaqueAssemblyType{ThermFile::OpaqueAssemblyType::Wall};

    Helper::MockNode elementNode("FileProperties");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"OpaqueAssemblyType", opaqueAssemblyType};

    auto correctNodes = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "OpaqueAssemblyType", "Wall");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestFilePropertiesEnumerators, AllOpaqueCrossSectionStrings)
{
    const std::vector<std::string> correct{"Sill Plate",
                                           "Header",
                                           "End Section",
                                           "Middle Section",
                                           "Thermal Bridge",
                                           "Window Framing - Sill",
                                           "Rough Opening - Header",
                                           "Rough Opening - Jamb"};

    const auto opaqueCrossSectionStrings{ThermFile::opaqueCrossSectionStrings()};

    Helper::checkVectorEquality(correct, opaqueCrossSectionStrings);
}

TEST_F(TestFilePropertiesEnumerators, OpaqueCrossSectionTypeDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "OpaqueCrossSectionType", "Rough Opening - Jamb");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::OpaqueCrossSectionType opaqueCrossSection;
    adapter >> FileParse::Child{"OpaqueCrossSectionType", opaqueCrossSection};

    EXPECT_EQ(static_cast<int>(ThermFile::OpaqueCrossSectionType::RoughOpeningJamb),
              static_cast<int>(opaqueCrossSection));
}

TEST_F(TestFilePropertiesEnumerators, OpaqueCrossSectionTypeSerialization)
{
    ThermFile::OpaqueCrossSectionType opaqueCrossSection{ThermFile::OpaqueCrossSectionType::ThermalBridge};

    Helper::MockNode elementNode("FileProperties");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"OpaqueCrossSectionType", opaqueCrossSection};

    auto correctNodes = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "OpaqueCrossSectionType", "Thermal Bridge");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestFilePropertiesEnumerators, AllOtherCrossSectionTypeStrings)
{
    const std::vector<std::string> correct{"Common Thermal Bridge", "General Cross Section"};

    const auto otherCrossSectionTypeStrings{ThermFile::otherCrossSectionTypeStrings()};

    Helper::checkVectorEquality(correct, otherCrossSectionTypeStrings);
}

TEST_F(TestFilePropertiesEnumerators, OtherCrossSectionTypeDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "OtherCrossSectionType", "General Cross Section");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::OtherCrossSectionType otherCrossSectionType;
    adapter >> FileParse::Child{"OtherCrossSectionType", otherCrossSectionType};

    EXPECT_EQ(static_cast<int>(ThermFile::OtherCrossSectionType::GeneralCrossSection),
              static_cast<int>(otherCrossSectionType));
}

TEST_F(TestFilePropertiesEnumerators, OtherCrossSectionTypeSerialization)
{
    ThermFile::OtherCrossSectionType otherCrossSectionType{ThermFile::OtherCrossSectionType::CommonThermalBridge};

    Helper::MockNode elementNode("FileProperties");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"OtherCrossSectionType", otherCrossSectionType};

    auto correctNodes = []() {
        Helper::MockNode node{"FileProperties"};

        addChildNode(node, "OtherCrossSectionType", "Common Thermal Bridge");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}