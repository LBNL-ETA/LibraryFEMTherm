#include <variant>

#include <gtest/gtest.h>

#include <fileParse/Common.hxx>

#include "THMZ/Preferences/Enumerators.hxx"
#include "THMZ/Preferences/EnumSerializers.hxx"

#include "MockNodeAdapter.hxx"

class TestFilePreferencesEnumerators : public testing::Test
{};

TEST_F(TestFilePreferencesEnumerators, UnitSystemDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "UnitSystem", "SI");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::UnitSystem unitSystem;
    adapter >> FileParse::Child{"UnitSystem", unitSystem};

    EXPECT_EQ(static_cast<int>(ThermFile::UnitSystem::SI), static_cast<int>(unitSystem));
}

TEST_F(TestFilePreferencesEnumerators, UnitTypeSerialization)
{
    ThermFile::UnitSystem unitSystem{ThermFile::UnitSystem::IP};

    Helper::MockNode elementNode("MainNode");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"UnitSystem", unitSystem};

    auto correctNodes = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "UnitSystem", "IP");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestFilePreferencesEnumerators, ConductivityUnitsDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "ConductivityUnits", "Btu/hr-ft-F");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::ConductivityUnits conductivityUnits;
    adapter >> FileParse::Child{"ConductivityUnits", conductivityUnits};

    EXPECT_EQ(static_cast<int>(ThermFile::ConductivityUnits::BtuPerHr_Ft_Fahrenheit), static_cast<int>(conductivityUnits));
}

TEST_F(TestFilePreferencesEnumerators, ConductivityUnitsSerialization)
{
    ThermFile::ConductivityUnits conductivityUnits{
      ThermFile::ConductivityUnits::Btu_InchPerHour_FootSquared_Fahrenheit};

    Helper::MockNode elementNode("MainNode");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"ConductivityUnits", conductivityUnits};

    auto correctNodes = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "ConductivityUnits", "Btu-in/hr-ft2-F");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestFilePreferencesEnumerators, UValueBasisTypeDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "UValueBasis", "Total length");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::UValueBasisType uValueBasisType;
    adapter >> FileParse::Child{"UValueBasis", uValueBasisType};

    EXPECT_EQ(static_cast<int>(ThermFile::UValueBasisType::TotalLength), static_cast<int>(uValueBasisType));
}

TEST_F(TestFilePreferencesEnumerators, UValueBasisTypeSerialization)
{
    ThermFile::UValueBasisType uValueBasisType{ThermFile::UValueBasisType::Custom};

    Helper::MockNode elementNode("MainNode");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"UValueBasis", uValueBasisType};

    auto correctNodes = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "UValueBasis", "Custom");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestFilePreferencesEnumerators, UnderlayTypeDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "UnderlayType", "None");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::UnderlayType underlayType;
    adapter >> FileParse::Child{"UnderlayType", underlayType};

    EXPECT_EQ(static_cast<int>(ThermFile::UnderlayType::None), static_cast<int>(underlayType));
}

TEST_F(TestFilePreferencesEnumerators, UnderlayTypeSerialization)
{
    ThermFile::UnderlayType underlayType{ThermFile::UnderlayType::DXF};

    Helper::MockNode elementNode("MainNode");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"UnderlayType", underlayType};

    auto correctNodes = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "UnderlayType", "DXF");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}