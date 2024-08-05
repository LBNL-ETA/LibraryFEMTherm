#pragma once

namespace ThermFile
{
    struct NodeResults;
    struct EdgeResults;
    struct CaseMeshResults;
    struct MeshResults;
}   // namespace ThermFile

namespace Helper
{
    struct MockNode;

    void expect_near(const ThermFile::NodeResults & expected, const ThermFile::NodeResults & actual, double tolerance);

    void expect_near(const ThermFile::EdgeResults & expected, const ThermFile::EdgeResults & actual, double tolerance);

    void expect_near(const ThermFile::CaseMeshResults & expected,
                     const ThermFile::CaseMeshResults & actual,
                     double tolerance);

    void expect_near(const ThermFile::MeshResults & expected, const ThermFile::MeshResults & actual, double tolerance);

    struct MockMeshResultsNode
    {
        std::string index;
        std::string temperature;
        std::string xFlux;
        std::string yFlux;

        [[maybe_unused]] MockMeshResultsNode(std::string index,
                                             std::string temperature,
                                             std::string xFlux,
                                             std::string yFlux);
    };

    Helper::MockNode generateMockMeshResultsNode(const MockMeshResultsNode & resultsNode);

    struct MockEdgeResultsNode
    {
        std::string segmentType;
        std::string index;
        std::string area;
        std::string flux;
        std::string heatRate;

        [[maybe_unused]] MockEdgeResultsNode(std::string segmentType,
                                             std::string index,
                                             std::string area,
                                             std::string flux,
                                             std::string heatRate);
    };

    Helper::MockNode generateMockEdgeResultsNode(const MockEdgeResultsNode & resultsNode);

    struct MockCaseMeshResults
    {
        std::string resultsType;
        std::vector<MockMeshResultsNode> nodes;

        [[maybe_unused]] explicit MockCaseMeshResults(std::string resultsType, std::vector<MockMeshResultsNode> nodes);
    };

    Helper::MockNode generateMockCaseMeshResults(const MockCaseMeshResults & results);

    struct MockMeshResults
    {
        std::string version;
        std::vector<MockCaseMeshResults> cases;

        [[maybe_unused]] explicit MockMeshResults(std::string version, std::vector<MockCaseMeshResults> cases);
    };

    Helper::MockNode generateMockMeshResults(const MockMeshResults & results);

}   // namespace Helper