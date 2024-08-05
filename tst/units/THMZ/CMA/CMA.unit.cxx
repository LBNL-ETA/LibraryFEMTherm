#include <gtest/gtest.h>

#include "THMZ/CMA/CMA.hxx"
#include "THMZ/CMA/Serializers.hxx"

#include "MockNodeAdapter.hxx"
#include "Utility.hxx"

class TestThermFileCMA : public testing::Test
{};

namespace
{
    Helper::MockNode &
      insertPositionNode(Helper::MockNode & parent, const std::string & value, const std::string & uuid)
    {
        Helper::MockNode & positionNode = Helper::addChildNode(parent, "Position");
        addChildNode(positionNode, "Value", value);
        addChildNode(positionNode, "UUID", uuid);
        return parent;
    }
}   // namespace

TEST_F(TestThermFileCMA, CMABCContainerDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"CMAData"};
        Helper::MockNode & positionsNode = Helper::addChildNode(node, "CMABCPositions");

        positionsNode = insertPositionNode(positionsNode, "Exterior", "3f50e7c8-9b49-4f72-8427-b9b8fbbd163d");
        positionsNode = insertPositionNode(positionsNode, "Interior", "4e3a1f79-c5cb-478e-8127-9c0a1ec27a93");
        positionsNode = insertPositionNode(positionsNode, "Exterior", "6a7e7bb9-8148-4a4d-939b-4b5315c5ad78");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::CMAData cmaData;
    adapter >> cmaData;

    const ThermFile::CMABCContainer correct{
      {ThermFile::CMABCPosition::Exterior, "3f50e7c8-9b49-4f72-8427-b9b8fbbd163d"},
      {ThermFile::CMABCPosition::Interior, "4e3a1f79-c5cb-478e-8127-9c0a1ec27a93"}};

    Helper::checkMapValuesEqual(correct, cmaData.cmaBCPositions);
}

TEST_F(TestThermFileCMA, CMABCContainerSerialization)
{
    ThermFile::CMAData cmaData;
    cmaData.cmaBCPositions = {{ThermFile::CMABCPosition::Exterior, "3f50e7c8-9b49-4f72-8427-b9b8fbbd163d"},
                              {ThermFile::CMABCPosition::Interior, "4e3a1f79-c5cb-478e-8127-9c0a1ec27a93"}};

    Helper::MockNode elementNode("CMAData");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << cmaData;

    auto correctNodes = []() {
        Helper::MockNode node{"CMAData"};
        Helper::addChildNode(node, "IGUType", "Unknown");

        Helper::MockNode & positionsNode = Helper::addChildNode(node, "CMABCPositions");

        positionsNode = insertPositionNode(positionsNode, "Interior", "4e3a1f79-c5cb-478e-8127-9c0a1ec27a93");
        positionsNode = insertPositionNode(positionsNode, "Exterior", "3f50e7c8-9b49-4f72-8427-b9b8fbbd163d");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}
