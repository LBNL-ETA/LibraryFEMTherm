#include <gtest/gtest.h>

#include <fileParse/Common.hxx>

#include "THMZ/Common/Enumerators.hxx"
#include "THMZ/Common/EnumSerializers.hxx"
#include "THMZ/SteadyStateResults/Enumerators.hxx"
#include "THMZ/SteadyStateResults/EnumSerializers.hxx"

#include "MockNodeAdapter.hxx"

class TestSteadyStateResultsEnumerators : public testing::Test
{};

TEST_F(TestSteadyStateResultsEnumerators, UValueDimensionTypeDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "Length-type", "Projected Y");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::UValueDimensionType dimensionType;
    adapter >> FileParse::Child{"Length-type", dimensionType};

    EXPECT_EQ(static_cast<int>(ThermFile::UValueDimensionType::YProjected), static_cast<int>(dimensionType));
}

TEST_F(TestSteadyStateResultsEnumerators, UValueDimensionTypeSerialization)
{
    ThermFile::UValueDimensionType dimensionType{ThermFile::UValueDimensionType::XProjected};

    Helper::MockNode elementNode("MainNode");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"Length-type", dimensionType};

    auto correctNodes = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "Length-type", "Projected X");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestSteadyStateResultsEnumerators, UValueDimensionTypeDeserializationUnknown)
{
    auto mockData = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "Length-type", "Some random text");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::UValueDimensionType dimensionType;
    adapter >> FileParse::Child{"Length-type", dimensionType};

    EXPECT_EQ(static_cast<int>(ThermFile::UValueDimensionType::Unknown), static_cast<int>(dimensionType));
}