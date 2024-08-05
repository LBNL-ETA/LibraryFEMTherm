#include <variant>

#include <gtest/gtest.h>

#include <fileParse/Common.hxx>

#include "THMZ/GlazingSystem/Enumerators.hxx"
#include "THMZ/GlazingSystem/EnumSerializers.hxx"

#include "MockNodeAdapter.hxx"

class TestGlazingSystemEnumerators : public testing::Test
{};

TEST_F(TestGlazingSystemEnumerators, EnvironmentConditionDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "EnvironmentalCondition", "Winter");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::EnvironmentalCondition environmentalCondition;
    adapter >> FileParse::Child{"EnvironmentalCondition", environmentalCondition};

    EXPECT_EQ(static_cast<int>(ThermFile::EnvironmentalCondition::Winter), static_cast<int>(environmentalCondition));
}

TEST_F(TestGlazingSystemEnumerators, EnvironmentConditionSerialization)
{
    ThermFile::EnvironmentalCondition environmentalCondition{ThermFile::EnvironmentalCondition::Summer};

    Helper::MockNode elementNode("MainNode");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"EnvironmentalCondition", environmentalCondition};

    auto correctNodes = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "EnvironmentalCondition", "Summer");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestGlazingSystemEnumerators, PositionDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "Position", "Interior");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::Position position;
    adapter >> FileParse::Child{"Position", position};

    EXPECT_EQ(static_cast<int>(ThermFile::Position::Interior), static_cast<int>(position));
}

TEST_F(TestGlazingSystemEnumerators, PositionSerialization)
{
    ThermFile::Position position{ThermFile::Position::Exterior};

    Helper::MockNode elementNode("MainNode");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"Position", position};

    auto correctNodes = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "Position", "Exterior");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}
