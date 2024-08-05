#include <gtest/gtest.h>

#include <fileParse/Common.hxx>

#include "THMZ/Common/EnumSerializers.hxx"

#include "MockNodeAdapter.hxx"

class TestSteadyStateMeshResultsEnumerators : public testing::Test
{};

TEST_F(TestSteadyStateMeshResultsEnumerators, MeshResultsTypeDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "Type", "Convection");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::SegmentType segmentType;
    adapter >> FileParse::Child{"Type", segmentType};

    EXPECT_EQ(static_cast<int>(ThermFile::SegmentType::Convection), static_cast<int>(segmentType));
}

TEST_F(TestSteadyStateMeshResultsEnumerators, MeshResultsTypeSerialization)
{
    ThermFile::SegmentType segmentType{ThermFile::SegmentType::RadiationEnclosure};

    Helper::MockNode elementNode("MainNode");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"Type", segmentType};

    auto correctNodes = []() {
        Helper::MockNode node{"MainNode"};

        addChildNode(node, "Type", "Radiation Enclosure");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}