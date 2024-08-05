#include <gtest/gtest.h>

#include "THMZ/Geometry/Enumerators.hxx"
#include "THMZ/Geometry/Serializers.hxx"

#include "MockNodeAdapter.hxx"
#include "THMXGeometryHelper.hxx"

class TestTHMXGeometryEnumerators : public testing::Test
{};

TEST_F(TestTHMXGeometryEnumerators, HeatFlowDirectionDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "HeatFlowDirection", "Up");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::Direction direction;
    adapter >> FileParse::Child{"HeatFlowDirection", direction};

    EXPECT_EQ(static_cast<int>(ThermFile::Direction::Up), static_cast<int>(direction));
}

TEST_F(TestTHMXGeometryEnumerators, HeatFlowDirectionSerialization)
{
    ThermFile::Direction direction{ThermFile::Direction::Left};

    Helper::MockNode elementNode("MainNode");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"HeatFlowDirection", direction};

    auto correctNodes = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "HeatFlowDirection", "Left");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestTHMXGeometryEnumerators, SurfaceTypeDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "SurfaceType", "Enclosure");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::SurfaceType type;
    adapter >> FileParse::Child{"SurfaceType", type};

    EXPECT_EQ(static_cast<int>(ThermFile::SurfaceType::Enclosure), static_cast<int>(type));
}

TEST_F(TestTHMXGeometryEnumerators, SurfaceTypeSerialization)
{
    ThermFile::SurfaceType type{ThermFile::SurfaceType::BoundaryCondition};

    Helper::MockNode elementNode("MainNode");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"SurfaceType", type};

    auto correctNodes = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "SurfaceType", "Boundary Condition");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestTHMXGeometryEnumerators, ShadeModifierDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "ShadeModifier", "Exterior");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::ShadeModifier modifier;
    adapter >> FileParse::Child{"ShadeModifier", modifier};

    EXPECT_EQ(static_cast<int>(ThermFile::ShadeModifier::Exterior), static_cast<int>(modifier));
}

TEST_F(TestTHMXGeometryEnumerators, ShadeModifierSerialization)
{
    ThermFile::ShadeModifier modifier{ThermFile::ShadeModifier::Interior};

    Helper::MockNode elementNode("MainNode");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"ShadeModifier", modifier};

    auto correctNodes = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "ShadeModifier", "Interior");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestTHMXGeometryEnumerators, PolygonTypeDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "PolygonType", "Gap");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::PolygonType type;
    adapter >> FileParse::Child{"PolygonType", type};

    EXPECT_EQ(static_cast<int>(ThermFile::PolygonType::Gap), static_cast<int>(type));
}

TEST_F(TestTHMXGeometryEnumerators, PolygonTypeSerialization)
{
    ThermFile::PolygonType type{ThermFile::PolygonType::Glass};

    Helper::MockNode elementNode("MainNode");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"PolygonType", type};

    auto correctNodes = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "PolygonType", "Glass");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}