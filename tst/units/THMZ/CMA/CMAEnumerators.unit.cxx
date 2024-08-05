#include <gtest/gtest.h>

#include <fileParse/Common.hxx>

#include "THMZ/CMA/EnumSerializers.hxx"

#include "MockNodeAdapter.hxx"

class TestThermFileCMAEnumerators : public testing::Test
{};

TEST_F(TestThermFileCMAEnumerators, CMABCPositionDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"ModelNode"};

        addChildNode(node, "BCPosition", "Exterior");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::CMABCPosition cmabcPosition;
    adapter >> FileParse::Child{"BCPosition", cmabcPosition};

    EXPECT_EQ(static_cast<int>(ThermFile::CMABCPosition::Exterior), static_cast<int>(cmabcPosition));
}

TEST_F(TestThermFileCMAEnumerators, CMABCPositionSerialization)
{
    ThermFile::CMABCPosition cmabcPosition{ThermFile::CMABCPosition::Interior};

    Helper::MockNode elementNode("ModelNode");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"BCPosition", cmabcPosition};

    auto correctNodes = []() {
        Helper::MockNode node{"ModelNode"};

        addChildNode(node, "BCPosition", "Interior");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestThermFileCMAEnumerators, CMAIGUTypeDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"ModelNode"};

        addChildNode(node, "CMAIGUType", "SingleLayer");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::CMAIGUType cmaIGUType;
    adapter >> FileParse::Child{"CMAIGUType", cmaIGUType};

    EXPECT_EQ(static_cast<int>(ThermFile::CMAIGUType::SingleLayer), static_cast<int>(cmaIGUType));
}

TEST_F(TestThermFileCMAEnumerators, CMAIGUTypeSerialization)
{
    ThermFile::CMAIGUType cmaIGUType{ThermFile::CMAIGUType::DoubleLayer};

    Helper::MockNode elementNode("ModelNode");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"CMAIGUType", cmaIGUType};

    auto correctNodes = []() {
        Helper::MockNode node{"ModelNode"};

        addChildNode(node, "CMAIGUType", "DoubleLayer");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}