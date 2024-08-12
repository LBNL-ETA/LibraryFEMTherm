#include <gtest/gtest.h>

#include "THMZ/CMA/CMA.hxx"
#include "THMZ/CMA/Serializers.hxx"

#include "MockNodeAdapter.hxx"
#include "Utility.hxx"
#include "HelperUtility.hxx"

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

    Helper::MockNode & insertCaseNode(Helper::MockNode & parent,
                                      const std::string & glazingOption,
                                      const std::string & spacerOption,
                                      const std::string & gapConductance,
                                      const std::string & spacerConductivity)
    {
        Helper::MockNode & caseNode = Helper::addChildNode(parent, "Case");
        addChildNode(caseNode, "GlazingOption", glazingOption);
        addChildNode(caseNode, "SpacerOption", spacerOption);
        addChildNode(caseNode, "GapConductance", gapConductance);
        addChildNode(caseNode, "SpacerConductivity", spacerConductivity);
        return parent;
    }

    void expectBestWorstValuesNear(const ThermFile::CMABestWorstValues & correct,
                                   const ThermFile::CMABestWorstValues & actual,
                                   const double tolerance)
    {
        EXPECT_EQ(static_cast<int>(correct.glazingCase), static_cast<int>(actual.glazingCase));
        EXPECT_EQ(static_cast<int>(correct.spacerCase), static_cast<int>(actual.spacerCase));
        EXPECT_NEAR(correct.gapConductance, actual.gapConductance, tolerance);
        EXPECT_NEAR(correct.spacerConductivity, actual.spacerConductivity, tolerance);
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

        node = insertCaseNode(node, "Low", "Low", "0.13", "0.25");
        node = insertCaseNode(node, "Low", "High", "0.15", "0.27");
        node = insertCaseNode(node, "High", "Low", "0.17", "0.29");
        node = insertCaseNode(node, "High", "High", "0.19", "0.31");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::CMAData cmaData;
    adapter >> cmaData;

    const ThermFile::CMABCContainer correctPositions{
      {ThermFile::CMABCPosition::Exterior, "3f50e7c8-9b49-4f72-8427-b9b8fbbd163d"},
      {ThermFile::CMABCPosition::Interior, "4e3a1f79-c5cb-478e-8127-9c0a1ec27a93"}};

    Helper::checkMapValuesEqual(correctPositions, cmaData.cmaBCPositions);

    const std::vector<ThermFile::CMABestWorstValues> correctBWValues{
      {ThermFile::CMACase::Low, ThermFile::CMACase::Low, 0.13, 0.25},
      {ThermFile::CMACase::Low, ThermFile::CMACase::High, 0.15, 0.27},
      {ThermFile::CMACase::High, ThermFile::CMACase::Low, 0.17, 0.29},
      {ThermFile::CMACase::High, ThermFile::CMACase::High, 0.19, 0.31}};

    ASSERT_EQ(correctBWValues.size(), cmaData.bestWorstValues.size());
    for(size_t i = 0; i < correctBWValues.size(); ++i)
    {
        expectBestWorstValuesNear(correctBWValues[i], cmaData.bestWorstValues[i], 1e-6);
    }
}

TEST_F(TestThermFileCMA, CMABCContainerSerialization)
{
    ThermFile::CMAData cmaData;
    cmaData.cmaBCPositions = {{ThermFile::CMABCPosition::Exterior, "3f50e7c8-9b49-4f72-8427-b9b8fbbd163d"},
                              {ThermFile::CMABCPosition::Interior, "4e3a1f79-c5cb-478e-8127-9c0a1ec27a93"}};
    cmaData.bestWorstValues = {{ThermFile::CMACase::Low, ThermFile::CMACase::Low, 0.13, 0.25},
                               {ThermFile::CMACase::Low, ThermFile::CMACase::High, 0.15, 0.27},
                               {ThermFile::CMACase::High, ThermFile::CMACase::Low, 0.17, 0.29},
                               {ThermFile::CMACase::High, ThermFile::CMACase::High, 0.19, 0.31}};

    Helper::MockNode elementNode("CMAData");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << cmaData;

    auto correctNodes = []() {
        Helper::MockNode node{"CMAData"};
        Helper::addChildNode(node, "IGUType", "Unknown");

        Helper::MockNode & positionsNode = Helper::addChildNode(node, "CMABCPositions");

        positionsNode = insertPositionNode(positionsNode, "Interior", "4e3a1f79-c5cb-478e-8127-9c0a1ec27a93");
        positionsNode = insertPositionNode(positionsNode, "Exterior", "3f50e7c8-9b49-4f72-8427-b9b8fbbd163d");

        node = insertCaseNode(node, "Low", "Low", "0.13", "0.25");
        node = insertCaseNode(node, "Low", "High", "0.15", "0.27");
        node = insertCaseNode(node, "High", "Low", "0.17", "0.29");
        node = insertCaseNode(node, "High", "High", "0.19", "0.31");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}
