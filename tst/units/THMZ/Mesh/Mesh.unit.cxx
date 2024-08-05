#include <gtest/gtest.h>

#include "THMZ/Mesh/Mesh.hxx"
#include "THMZ/Mesh/Serializers.hxx"

#include "MockNodeAdapter.hxx"
#include "THMXMeshHelper.hxx"

class TestMesh : public testing::Test
{};

TEST_F(TestMesh, NodeDeserializer)
{
    auto elementNode(Helper::Mesh::generateMockPoint({"1", "2.0", "3.0"}));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::Mesh::Node point;
    adapter >> point;

    Helper::Mesh::expect_near(ThermFile::Mesh::Node{1, 2.0, 3.0}, point, 1e-6);
}

TEST_F(TestMesh, NodeSerializer)
{
    ThermFile::Mesh::Node point{1, 2.0, 3.0};

    Helper::MockNode elementNode("Node");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << point;

    auto correctNodes = Helper::Mesh::generateMockPoint({"1", "2", "3"});

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes));
}

TEST_F(TestMesh, ElementDeserializer)
{
    auto elementNode(Helper::Mesh::generateMockElement({"1", "1", "2", "3", "3", "1"}));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::Mesh::Element element;
    adapter >> element;

    Helper::Mesh::expect_eq(ThermFile::Mesh::Element{1, 1, 2, 3, 3, 1}, element);
}

TEST_F(TestMesh, ElementSerializer)
{
    ThermFile::Mesh::Element element{1, 1, 2, 3, 3, 1};

    Helper::MockNode elementNode("Element");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << element;

    auto correctNodes = Helper::Mesh::generateMockElement({"1", "1", "2", "3", "3", "1"});

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes));
}

TEST_F(TestMesh, FluxSegmentDeserializer)
{
    auto elementNode(Helper::Mesh::generateMockSegmentFlux({"1", "1", "2", "4.23", "2.3545", "6"}));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::Mesh::Segment::Flux segment;

    adapter >> segment;

    Helper::Mesh::expect_near(ThermFile::Mesh::Segment::Flux{1u, 1u, 2u, 4.23, 2.3545, 6u}, segment, 1e-6);
}

TEST_F(TestMesh, FluxSegmentSerializer)
{
    ThermFile::Mesh::Segment::Flux segment{1, 1, 2, 4.23, 2.3545, 6};

    Helper::MockNode elementNode("Flux");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << segment;

    auto correctNodes = Helper::Mesh::generateMockSegmentFlux({"1", "1", "2", "4.23", "2.3545", "6"});

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes));
}

TEST_F(TestMesh, TemperatureSegmentDeserializer)
{
    auto elementNode(Helper::Mesh::generateMockSegmentTemperature({"1", "14.702914", "6"}));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::Mesh::Segment::Temperature segment;

    adapter >> segment;

    Helper::Mesh::expect_near(ThermFile::Mesh::Segment::Temperature{1u, 14.702914, 6u}, segment, 1e-6);
}

TEST_F(TestMesh, TemperatureSegmentSerializer)
{
    ThermFile::Mesh::Segment::Temperature segment{181u, 14.702914, 3u};

    Helper::MockNode elementNode("Temperature");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << segment;

    auto correctNodes = Helper::Mesh::generateMockSegmentTemperature({"181", "14.702914", "3"});

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes));
}

TEST_F(TestMesh, ConvectionSegmentDeserializer)
{
    auto elementNode(
      Helper::Mesh::generateMockSegmentConvection({"1", "18", "35", "2.43", "4.241542", "-13.897517", "12"}));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::Mesh::Segment::Convection segment;

    adapter >> segment;

    Helper::Mesh::expect_near(
      ThermFile::Mesh::Segment::Convection{1u, 18u, 35u, 2.43, 4.241542, -13.897517, 12u}, segment, 1e-6);
}

TEST_F(TestMesh, ConvectionSegmentSerializer)
{
    ThermFile::Mesh::Segment::Convection segment{1, 18, 35, 2.43, 4.241542, -13.897517, 12};

    Helper::MockNode elementNode("Convection");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << segment;

    auto correctNodes =
      Helper::Mesh::generateMockSegmentConvection({"1", "18", "35", "2.43", "4.241542", "-13.897517", "12"});

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes));
}

TEST_F(TestMesh, RadiationSegmentDeserializer)
{
    auto elementNode(Helper::Mesh::generateMockRadiationSegments({"1", "18", "35", "2.43", "4.241542", "-13.897517"}));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::Mesh::Segment::Radiation segment;

    adapter >> segment;

    Helper::Mesh::expect_near(
      ThermFile::Mesh::Segment::Radiation{1u, 18u, 35u, 2.43, 4.241542, -13.897517}, segment, 1e-6);
}

TEST_F(TestMesh, EnclosureRadiationSegmentDeserializer)
{
    auto elementNode(
      Helper::Mesh::generateMockEnclosureRadiationSegments({"1", "18", "35", "24", "4.241542", "-13.897517"}));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::Mesh::Segment::RadiationEnclosure segment;

    adapter >> segment;

    Helper::Mesh::expect_near(
      ThermFile::Mesh::Segment::RadiationEnclosure{1u, 18u, 35u, 24u, 4.241542, -13.897517}, segment, 1e-6);
}

TEST_F(TestMesh, RadiationEnclosureSegmentSerializer)
{
    ThermFile::Mesh::Segment::RadiationEnclosure segment{1, 18, 35, 24, 4.241542, -13.897517};

    Helper::MockNode elementNode("RadiationEnclosure");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << segment;

    auto correctNodes =
      Helper::Mesh::generateMockEnclosureRadiationSegments({"1", "18", "35", "24", "4.241542", "-13.897517"});

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes));
}

TEST_F(TestMesh, BCSegmentsDeserializer)
{
    std::vector<Helper::Mesh::MockSegmentFlux> mockFluxes{{"1", "1", "2", "4.23", "2.3545", "6"},
                                                          {"2", "3", "2", "1", "1", "1"}};
    std::vector<Helper::Mesh::MockSegmentTemperature> mockTemperatures{{"1", "53.23", "6"}};
    std::vector<Helper::Mesh::MockSegmentConvection> mockConvections{
      {"1", "18", "35", "2.43", "4.241542", "-13.897517", "12"},
      {"2", "54", "42", "26.299068", "33.539087", "15.35978", "32"}};
    std::vector<Helper::Mesh::MockRadiationSegment> mockRadiations{{"1", "18", "35", "2.43", "4.241542", "-13.897517"}};
    std::vector<Helper::Mesh::MockEnclosureRadiationSegment> mockRadiationEnclosures{
      {"2", "81", "53", "42", "14.249542", "-31.975817"}};

    Helper::Mesh::MockBCSegments mockBCSegments{
      mockFluxes, mockTemperatures, mockConvections, mockRadiations, mockRadiationEnclosures};
    auto elementNode(Helper::Mesh::generateMockBCSegments(mockBCSegments));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::Mesh::BCSegments bcSegments;
    adapter >> bcSegments;

    std::vector<ThermFile::Mesh::Segment::Flux> correctFluxes{{1u, 1u, 2u, 4.23, 2.3545, 6u},
                                                              {2u, 3u, 2u, 1.0, 1.0, 1u}};
    std::vector<ThermFile::Mesh::Segment::Temperature> correctTemperatures{{1u, 53.23, 6u}};
    std::vector<ThermFile::Mesh::Segment::Convection> correctConvections{
      {1u, 18u, 35u, 2.43, 4.241542, -13.897517, 12u}, {2u, 54u, 42u, 26.299068, 33.539087, 15.35978, 32u}};
    std::vector<ThermFile::Mesh::Segment::Radiation> correctRadiations{{1u, 18u, 35u, 2.43, 4.241542, -13.897517}};
    std::vector<ThermFile::Mesh::Segment::RadiationEnclosure> correctRadiationEnclosures{
      {2u, 81u, 53u, 42u, 14.249542, -31.975817}};

    Helper::Mesh::expect_near(
      ThermFile::Mesh::BCSegments{
        correctFluxes, correctTemperatures, correctConvections, correctRadiations, correctRadiationEnclosures},
      bcSegments,
      1e-6);
}

TEST_F(TestMesh, BCSegmentsSerializer)
{
    std::vector<ThermFile::Mesh::Segment::Flux> fluxes{{1u, 1u, 2u, 4.23, 2.3545, 6u}, {2u, 3u, 2u, 1.0, 1.0, 1u}};
    std::vector<ThermFile::Mesh::Segment::Temperature> temperatures{{1u, 53.23, 6u}};
    std::vector<ThermFile::Mesh::Segment::Convection> convections{{1u, 18u, 35u, 2.43, 4.241542, -13.897517, 12u},
                                                                  {2u, 54u, 42u, 26.299068, 33.539087, 15.35978, 32u}};
    std::vector<ThermFile::Mesh::Segment::Radiation> radiations{{1u, 18u, 35u, 2.43, 4.241542, -13.897517}};
    std::vector<ThermFile::Mesh::Segment::RadiationEnclosure> radiationEnclosures{
      {2u, 81u, 53u, 42u, 14.249542, -31.975817}};
    ThermFile::Mesh::BCSegments bcSegments{fluxes, temperatures, convections, radiations, radiationEnclosures};

    Helper::MockNode elementNode("BCSegments");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << bcSegments;

    std::vector<Helper::Mesh::MockSegmentFlux> correctMockFluxes{{"1", "1", "2", "4.23", "2.3545", "6"},
                                                                 {"2", "3", "2", "1", "1", "1"}};
    std::vector<Helper::Mesh::MockSegmentTemperature> mockTemperatures{{"1", "53.23", "6"}};
    std::vector<Helper::Mesh::MockSegmentConvection> mockConvections{
      {"1", "18", "35", "2.43", "4.241542", "-13.897517", "12"},
      {"2", "54", "42", "26.299068", "33.539087", "15.35978", "32"}};
    std::vector<Helper::Mesh::MockRadiationSegment> mockRadiations{{"1", "18", "35", "2.43", "4.241542", "-13.897517"}};
    std::vector<Helper::Mesh::MockEnclosureRadiationSegment> mockRadiationEnclosures{
      {"2", "81", "53", "42", "14.249542", "-31.975817"}};
    Helper::Mesh::MockBCSegments correctMockBCSegments{
      correctMockFluxes, mockTemperatures, mockConvections, mockRadiations, mockRadiationEnclosures};
    auto correctNodes = Helper::Mesh::generateMockBCSegments(correctMockBCSegments);

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes));
}
TEST_F(TestMesh, MeshCaseDeserializer)
{
    std::vector<Helper::Mesh::MockPoint> mockPoints{{"1", "2.0", "3.0"}, {"2", "4.0", "5.0"}, {"3", "6.0", "7.0"}};
    std::vector<Helper::Mesh::MockElement> mockElements{{"1", "1", "2", "3", "3", "1"}, {"2", "3", "2", "1", "1", "1"}};

    std::vector<Helper::Mesh::MockSegmentFlux> mockFluxes{{"1", "1", "2", "4.23", "2.3545", "6"},
                                                          {"2", "3", "2", "1", "1", "1"}};
    std::vector<Helper::Mesh::MockSegmentTemperature> mockTemperatures{{"1", "53.23", "6"}};
    std::vector<Helper::Mesh::MockSegmentConvection> mockConvections{
      {"1", "18", "35", "2.43", "4.241542", "-13.897517", "12"},
      {"2", "54", "42", "26.299068", "33.539087", "15.35978", "32"}};
    std::vector<Helper::Mesh::MockRadiationSegment> mockRadiations{{"1", "18", "35", "2.43", "4.241542", "-13.897517"}};
    std::vector<Helper::Mesh::MockEnclosureRadiationSegment> mockRadiationEnclosures{
      {"2", "81", "53", "42", "14.249542", "-31.975817"}};

    Helper::Mesh::MockBCSegments mockBCSegments{
      mockFluxes, mockTemperatures, mockConvections, mockRadiations, mockRadiationEnclosures};

    Helper::Mesh::MockMeshCase mockMesh("U-factor", mockPoints, mockElements, mockBCSegments);
    auto elementNode(Helper::Mesh::generateMockMeshCase(mockMesh));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::Mesh::Case mesh;
    adapter >> mesh;

    std::vector<ThermFile::Mesh::Node> correctNodes{{1, 2.0, 3.0}, {2, 4.0, 5.0}, {3, 6.0, 7.0}};
    std::vector<ThermFile::Mesh::Element> correctElements{{1, 1, 2, 3, 3, 1}, {2, 3, 2, 1, 1, 1}};

    std::vector<ThermFile::Mesh::Segment::Flux> correctFluxes{{1u, 1u, 2u, 4.23, 2.3545, 6u},
                                                              {2u, 3u, 2u, 1.0, 1.0, 1u}};
    std::vector<ThermFile::Mesh::Segment::Temperature> correctTemperatures{{1u, 53.23, 6u}};
    std::vector<ThermFile::Mesh::Segment::Convection> correctConvections{
      {1u, 18u, 35u, 2.43, 4.241542, -13.897517, 12u}, {2u, 54u, 42u, 26.299068, 33.539087, 15.35978, 32u}};
    std::vector<ThermFile::Mesh::Segment::Radiation> correctRadiations{{1u, 18u, 35u, 2.43, 4.241542, -13.897517}};
    std::vector<ThermFile::Mesh::Segment::RadiationEnclosure> correctRadiationEnclosures{
      {2u, 81u, 53u, 42u, 14.249542, -31.975817}};

    ThermFile::Mesh::BCSegments correctBCSegments{
      correctFluxes, correctTemperatures, correctConvections, correctRadiations, correctRadiationEnclosures};

    Helper::Mesh::expect_near(
      ThermFile::Mesh::Case{
        ThermFile::RunType::UFactor, std::nullopt, std::nullopt, correctNodes, correctElements, correctBCSegments},
      mesh,
      1e-6);
}

TEST_F(TestMesh, MeshCaseSerializer)
{
    std::vector<ThermFile::Mesh::Node> nodes{{1, 2.0, 3.0}, {2, 4.0, 5.0}, {3, 6.0, 7.0}};
    std::vector<ThermFile::Mesh::Element> elements{{1, 1, 2, 3, 3, 1}, {2, 3, 2, 1, 1, 1}};

    std::vector<ThermFile::Mesh::Segment::Flux> fluxes{{1u, 1u, 2u, 4.23, 2.3545, 6u}, {2u, 3u, 2u, 1.0, 1.0, 1u}};
    std::vector<ThermFile::Mesh::Segment::Temperature> temperatures{{1u, 53.23, 6u}};
    std::vector<ThermFile::Mesh::Segment::Convection> convections{{1u, 18u, 35u, 2.43, 4.241542, -13.897517, 12u},
                                                                  {2u, 54u, 42u, 26.299068, 33.539087, 15.35978, 32u}};
    std::vector<ThermFile::Mesh::Segment::Radiation> radiations{{1u, 18u, 35u, 2.43, 4.241542, -13.897517}};
    std::vector<ThermFile::Mesh::Segment::RadiationEnclosure> radiationEnclosures{
      {2u, 81u, 53u, 42u, 14.249542, -31.975817}};
    ThermFile::Mesh::BCSegments bcSegments{fluxes, temperatures, convections, radiations, radiationEnclosures};

    ThermFile::Mesh::Case mesh{
      ThermFile::RunType::CondensationResistance, std::nullopt, std::nullopt, nodes, elements, bcSegments};

    Helper::MockNode elementNode("Case");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << mesh;

    std::vector<Helper::Mesh::MockPoint> correctMockPoints{{"1", "2", "3"}, {"2", "4", "5"}, {"3", "6", "7"}};
    std::vector<Helper::Mesh::MockElement> correctMockElements{{"1", "1", "2", "3", "3", "1"},
                                                               {"2", "3", "2", "1", "1", "1"}};

    std::vector<Helper::Mesh::MockSegmentFlux> correctMockFluxes{{"1", "1", "2", "4.23", "2.3545", "6"},
                                                                 {"2", "3", "2", "1", "1", "1"}};
    std::vector<Helper::Mesh::MockSegmentTemperature> mockTemperatures{{"1", "53.23", "6"}};
    std::vector<Helper::Mesh::MockSegmentConvection> mockConvections{
      {"1", "18", "35", "2.43", "4.241542", "-13.897517", "12"},
      {"2", "54", "42", "26.299068", "33.539087", "15.35978", "32"}};
    std::vector<Helper::Mesh::MockRadiationSegment> mockRadiations{{"1", "18", "35", "2.43", "4.241542", "-13.897517"}};
    std::vector<Helper::Mesh::MockEnclosureRadiationSegment> mockRadiationEnclosures{
      {"2", "81", "53", "42", "14.249542", "-31.975817"}};
    Helper::Mesh::MockBCSegments correctMockBCSegments{
      correctMockFluxes, mockTemperatures, mockConvections, mockRadiations, mockRadiationEnclosures};

    Helper::Mesh::MockMeshCase correctMockMesh(
      {"Condensation Resistance", correctMockPoints, correctMockElements, correctMockBCSegments});
    auto correctNodes = Helper::Mesh::generateMockMeshCase(correctMockMesh);

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes));
}