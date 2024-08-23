#include <gtest/gtest.h>

#include "THMXSteadyStateMeshResultsHelper.hxx"

#include "THMZ/SteadyStateMeshResults/Results.hxx"
#include "MockNodeAdapter.hxx"
#include "HelperUtility.hxx"

namespace Helper
{
    void expect_near(const ThermFile::NodeResults & expected, const ThermFile::NodeResults & actual, double tolerance)
    {
        EXPECT_NEAR(expected.temperature, actual.temperature, tolerance);
        EXPECT_NEAR(expected.xFlux, actual.xFlux, tolerance);
        EXPECT_NEAR(expected.yFlux, actual.yFlux, tolerance);
    }

    void expect_near(const ThermFile::EdgeResults & expected, const ThermFile::EdgeResults & actual, double tolerance)
    {
        EXPECT_EQ(expected.segmentType, actual.segmentType);
        EXPECT_EQ(expected.index, actual.index);
        EXPECT_NEAR(expected.area, actual.area, tolerance);
        EXPECT_NEAR(expected.flux, actual.flux, tolerance);
        EXPECT_NEAR(expected.heatRate, actual.heatRate, tolerance);
    }

    void expect_eq(const ThermFile::TagNodes & expected, const ThermFile::TagNodes & actual)
    {
        EXPECT_EQ(expected.name, actual.name);
        ASSERT_EQ(expected.nodes.size(), actual.nodes.size());
        for(size_t i = 0; i < expected.nodes.size(); ++i)
        {
            EXPECT_EQ(expected.nodes[i], actual.nodes[i]);
        }
    }

    void expect_eq(const ThermFile::TagNodesCase & expected, const ThermFile::TagNodesCase & actual)
    {
        Helper::assertOptionalEq(expected.resultsType, actual.resultsType);
        Helper::assertOptionalEq(expected.glazingCase, actual.glazingCase);
        Helper::assertOptionalEq(expected.spacerCase, actual.spacerCase);

        ASSERT_EQ(expected.tagNodes.size(), actual.tagNodes.size());
        for(size_t i = 0; i < expected.tagNodes.size(); ++i)
        {
            expect_eq(expected.tagNodes[i], actual.tagNodes[i]);
        }
    }

    void expect_near(const ThermFile::CaseMeshResults & expected,
                     const ThermFile::CaseMeshResults & actual,
                     double tolerance)
    {
        ASSERT_EQ(expected.nodes.size(), actual.nodes.size());
        for(size_t i = 0; i < expected.nodes.size(); ++i)
        {
            expect_near(expected.nodes[i], actual.nodes[i], tolerance);
        }
    }

    void expect_near(const ThermFile::MeshResults & expected, const ThermFile::MeshResults & actual, double tolerance)
    {
        ASSERT_EQ(expected.cases.size(), actual.cases.size());
        for(size_t i = 0; i < expected.cases.size(); ++i)
        {
            expect_near(expected.cases[i], actual.cases[i], tolerance);
        }
        ASSERT_EQ(expected.tagNodes.size(), actual.tagNodes.size());
        for(size_t i = 0; i < expected.tagNodes.size(); ++i)
        {
            expect_eq(expected.tagNodes[i], actual.tagNodes[i]);
        }
    }

    MockMeshResultsNode::MockMeshResultsNode(std::string index,
                                             std::string temperature,
                                             std::string xFlux,
                                             std::string yFlux) :
        index(std::move(index)), temperature(std::move(temperature)), xFlux(std::move(xFlux)), yFlux(std::move(yFlux))
    {}

    Helper::MockNode generateMockMeshResultsNode(const MockMeshResultsNode & resultsNode)
    {
        Helper::MockNode node{"Node"};
        addChildNode(node, "Index", resultsNode.index);
        addChildNode(node, "Temperature", resultsNode.temperature);
        addChildNode(node, "X-flux", resultsNode.xFlux);
        addChildNode(node, "Y-flux", resultsNode.yFlux);

        return node;
    }

    MockTagNodes::MockTagNodes(std::string name, std::vector<std::string> nodes) :
        name(std::move(name)), nodes(std::move(nodes))
    {}

    Helper::MockNode generateMockTagNodes(const MockTagNodes & results)
    {
        Helper::MockNode node{"Tag"};
        addChildNode(node, "Name", results.name);
        auto & nodesNode = addChildNode(node, "Nodes");

        for(const auto & nodeCase : results.nodes)
        {
            addChildNode(nodesNode, "Node", nodeCase);
        }

        return node;
    }

    MockEdgeResultsNode::MockEdgeResultsNode(
      std::string segmentType, std::string index, std::string area, std::string flux, std::string heatRate) :
        segmentType(std::move(segmentType)),
        index(std::move(index)),
        area(std::move(area)),
        flux(std::move(flux)),
        heatRate(std::move(heatRate))
    {}

    Helper::MockNode generateMockEdgeResultsNode(const MockEdgeResultsNode & resultsNode)
    {
        Helper::MockNode node{"Edge"};
        addChildNode(node, "Type", resultsNode.segmentType);
        addChildNode(node, "Index", resultsNode.index);
        addChildNode(node, "Area", resultsNode.area);
        addChildNode(node, "Flux", resultsNode.flux);
        addChildNode(node, "HeatRate", resultsNode.heatRate);

        return node;
    }

    MockCaseMeshResults::MockCaseMeshResults(std::string resultsType, std::vector<MockMeshResultsNode> nodes) :
        resultsType{std::move(resultsType)}, nodes{std::move(nodes)}
    {}

    Helper::MockNode generateMockCaseMeshResults(const MockCaseMeshResults & meshResults)
    {
        Helper::MockNode node{"Case"};
        addChildNode(node, "ResultsType", meshResults.resultsType);
        auto & nodesNode = addChildNode(node, "Nodes");

        for(const auto & nodeCase : meshResults.nodes)
        {
            addChildNode(nodesNode, generateMockMeshResultsNode(nodeCase));
        }

        return node;
    }

    Helper::MockNode generateMockTagNodesCase(const MockTagNodesCase & results)
    {
        Helper::MockNode node{"TagNodesCase"};
        addChildNodeIfNotEmpty(node, "ResultsType", results.resultsType);
        addChildNodeIfNotEmpty(node, "GlazingCase", results.glazingCase);
        addChildNodeIfNotEmpty(node, "SpacerCase", results.spacerCase);

        for(const auto & tagNodes : results.tagNodes)
        {
            addChildNode(node, generateMockTagNodes(tagNodes));
        }

        return node;
    }

    MockTagNodesCase::MockTagNodesCase(std::string resultsType,
                                       std::string glazingCase,
                                       std::string spacerCase,
                                       std::vector<MockTagNodes> tagNodes) :
        resultsType{std::move(resultsType)},
        glazingCase{std::move(glazingCase)},
        spacerCase{std::move(spacerCase)},
        tagNodes{std::move(tagNodes)}
    {}

    MockMeshResults::MockMeshResults(std::string version,
                                     std::vector<MockCaseMeshResults> cases,
                                     std::vector<MockTagNodesCase> tagNodes) :
        version{std::move(version)}, cases{std::move(cases)}, tagNodes{std::move(tagNodes)}
    {}

    Helper::MockNode generateMockMeshResults(const MockMeshResults & results)
    {
        Helper::MockNode node{"MeshResults"};

        addChildNode(node, "Version", results.version);

        for(const auto & caseResults : results.cases)
        {
            addChildNode(node, generateMockCaseMeshResults(caseResults));
        }

        for(const auto & tagNodes : results.tagNodes)
        {
            addChildNode(node, generateMockTagNodesCase(tagNodes));
        }

        return node;
    }
}   // namespace Helper
