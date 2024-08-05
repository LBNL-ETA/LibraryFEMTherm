#include <gtest/gtest.h>

#include <fileParse/Common.hxx>

#include "THMZ/Common/EnumSerializers.hxx"

#include "MockNodeAdapter.hxx"

class TestTHMZCommon : public testing::Test
{};

TEST_F(TestTHMZCommon, RunTypeDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"ModelNode"};

        addChildNode(node, "ModelType", "U-factor");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::RunType resultsType;
    adapter >> FileParse::Child{"ModelType", resultsType};

    EXPECT_EQ(static_cast<int>(ThermFile::RunType::UFactor), static_cast<int>(resultsType));
}

TEST_F(TestTHMZCommon, RunTypeSerialization)
{
    ThermFile::RunType runType{ThermFile::RunType::CondensationResistance};

    Helper::MockNode elementNode("ModelNode");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"ModelType", runType};

    auto correctNodes = []() {
        Helper::MockNode node{"ModelNode"};

        addChildNode(node, "ModelType", "Condensation Resistance");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}