#include <gtest/gtest.h>

#include "THMZ/SteadyStateMeshResults/Results.hxx"
#include "THMZ/Common/EnumSerializers.hxx"
#include "THMZ/SteadyStateMeshResults/Serializers.hxx"

#include "MockNodeAdapter.hxx"
#include "THMXSteadyStateMeshResultsHelper.hxx"

class TestMesh : public testing::Test
{};

TEST_F(TestMesh, NodeResultsDeserialization)
{
    auto elementNode(Helper::generateMockMeshResultsNode({"1", "12.38", "1.29", "0.12"}));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::NodeResults nodeResults;
    adapter >> nodeResults;

    ThermFile::NodeResults correctNodeResults{1u, 12.38, 1.29, 0.12};
    Helper::expect_near(correctNodeResults, nodeResults, 1e-6);
}

TEST_F(TestMesh, NodeResultsSerialization)
{
    ThermFile::NodeResults nodeResults{1u, 12.38, 1.29, 0.12};

    Helper::MockNode node{"Node"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << nodeResults;

    EXPECT_TRUE(
      Helper::compareNodes(adapter.getNode(), Helper::generateMockMeshResultsNode({"1", "12.38", "1.29", "0.12"})));
}

TEST_F(TestMesh, EdgeResultsDeserialization)
{
    Helper::MockEdgeResultsNode node{"Convection", "1", "12.38", "1.29", "0.12"};
    auto edgeResultsNode(Helper::generateMockEdgeResultsNode(node));
    const Helper::MockNodeAdapter adapter{&edgeResultsNode};

    ThermFile::EdgeResults edgeResults;
    adapter >> edgeResults;

    ThermFile::EdgeResults correctEdgeResults{ThermFile::SegmentType::Convection, 1u, 12.38, 1.29, 0.12};
    Helper::expect_near(correctEdgeResults, edgeResults, 1e-6);
}

TEST_F(TestMesh, EdgeResultsSerialization)
{
    ThermFile::EdgeResults edgeResults{ThermFile::SegmentType::Convection, 1u, 12.38, 1.29, 0.12};

    Helper::MockNode node{"Edge"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << edgeResults;

    Helper::MockEdgeResultsNode correctNode{"Convection", "1", "12.38", "1.29", "0.12"};
    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), Helper::generateMockEdgeResultsNode(correctNode)));
}

TEST_F(TestMesh, MeshCaseResultsDeserialization)
{
    Helper::MockCaseMeshResults node{"U-factor", {{"1", "12.38", "1.29", "0.12"}, {"2", "13.38", "2.29", "1.12"}}};
    auto meshResultsNode(Helper::generateMockCaseMeshResults(node));
    const Helper::MockNodeAdapter adapter{&meshResultsNode};

    ThermFile::CaseMeshResults meshResults;
    adapter >> meshResults;

    ThermFile::CaseMeshResults correctMeshResults{ThermFile::RunType::UFactor,
                                                  std::nullopt,   // glazingCase
                                                  std::nullopt,   // spacerCase
                                                  {{1u, 12.38, 1.29, 0.12}, {2u, 13.38, 2.29, 1.12}}};
    Helper::expect_near(correctMeshResults, meshResults, 1e-6);
}

TEST_F(TestMesh, MeshCaseResultsSerialization)
{
    ThermFile::CaseMeshResults meshResults{ThermFile::RunType::UFactor,
                                           std::nullopt,   // glazingCase
                                           std::nullopt,   // spacerCase
                                           {{1u, 12.38, 1.29, 0.12}, {2u, 13.38, 2.29, 1.12}}};

    Helper::MockNode node{"Case"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << meshResults;

    Helper::MockCaseMeshResults correctNode{"U-factor",
                                            {{"1", "12.38", "1.29", "0.12"}, {"2", "13.38", "2.29", "1.12"}}};
    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), Helper::generateMockCaseMeshResults(correctNode)));
}

TEST_F(TestMesh, MeshResultsDeserialization)
{
    Helper::MockCaseMeshResults uFactorCase{"U-factor",
                                            {{"1", "12.38", "1.29", "0.12"}, {"2", "13.38", "2.29", "1.12"}}};
    Helper::MockCaseMeshResults crCase{"Condensation Resistance",
                                       {{"1", "14.40", "2.30", "0.25"}, {"2", "15.50", "3.45", "1.25"}}};

    Helper::MockMeshResults meshResults{"1", {uFactorCase, crCase}};
    auto meshResultsNode(Helper::generateMockMeshResults(meshResults));
    const Helper::MockNodeAdapter adapter{&meshResultsNode};

    ThermFile::MeshResults results;
    adapter >> results;

    ThermFile::MeshResults correctResults{"1",
                                          {{ThermFile::RunType::UFactor,
                                            std::nullopt,   // glazingCase
                                            std::nullopt,   // spacerCase
                                            {{1u, 12.38, 1.29, 0.12}, {2u, 13.38, 2.29, 1.12}}},
                                           {ThermFile::RunType::CondensationResistance,
                                            std::nullopt,   // glazingCase
                                            std::nullopt,   // spacerCase
                                            {{1u, 14.40, 2.30, 0.25}, {2u, 15.50, 3.45, 1.25}}}}};
    Helper::expect_near(correctResults, results, 1e-6);
}

TEST_F(TestMesh, MeshResultsSerialization)
{
    ThermFile::MeshResults meshResults{"1",
                                       {{ThermFile::RunType::UFactor,
                                         std::nullopt,   // glazingCase
                                         std::nullopt,   // spacerCase
                                         {{1u, 12.38, 1.29, 0.12}, {2u, 13.38, 2.29, 1.12}}},
                                        {ThermFile::RunType::CondensationResistance,
                                         std::nullopt,   // glazingCase
                                         std::nullopt,   // spacerCase
                                         {{1u, 14.40, 2.30, 0.25}, {2u, 15.50, 3.45, 1.25}}}}};

    Helper::MockNode node{"MeshResults"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << meshResults;

    Helper::MockCaseMeshResults uFactorCase{"U-factor",
                                            {{"1", "12.38", "1.29", "0.12"}, {"2", "13.38", "2.29", "1.12"}}};
    Helper::MockCaseMeshResults crCase{"Condensation Resistance",
                                       {{"1", "14.4", "2.3", "0.25"}, {"2", "15.5", "3.45", "1.25"}}};
    Helper::MockMeshResults expectedMeshResults{"1", {uFactorCase, crCase}};

    // Generate the expected mock mesh results node for comparison
    auto expectedMeshResultsNode = Helper::generateMockMeshResults(expectedMeshResults);

    // Compare the serialized results with what is expected
    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), expectedMeshResultsNode));
}
