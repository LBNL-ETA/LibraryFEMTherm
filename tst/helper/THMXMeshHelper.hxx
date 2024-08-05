#pragma once

#include <string>
#include <vector>

namespace ThermFile::Mesh
{
    struct Node;
    struct Element;
    struct BCSegments;
    struct Case;
    struct Mesh;
    namespace Segment
    {
        struct Flux;
        struct Temperature;
        struct Convection;
        struct Radiation;
        struct RadiationEnclosure;
    }   // namespace Segment
}   // namespace ThermFile::Mesh

namespace Helper
{
    struct MockNode;
}   // namespace Helper

namespace Helper::Mesh
{
    void expect_near(const ThermFile::Mesh::Node & expected, const ThermFile::Mesh::Node & result, double tolerance);

    void expect_eq(const ThermFile::Mesh::Element & expected, const ThermFile::Mesh::Element & result);

    void expect_near(const ThermFile::Mesh::Segment::Flux & expected,
                     const ThermFile::Mesh::Segment::Flux & result,
                     double tolerance);

    void expect_near(const ThermFile::Mesh::Segment::Temperature & expected,
                     const ThermFile::Mesh::Segment::Temperature & result,
                     double tolerance);

    void expect_near(const ThermFile::Mesh::Segment::Convection & expected,
                     const ThermFile::Mesh::Segment::Convection & result,
                     double tolerance);

    void expect_near(const ThermFile::Mesh::Segment::Radiation & expected,
                     const ThermFile::Mesh::Segment::Radiation & result,
                     double tolerance);

    void expect_near(const ThermFile::Mesh::Segment::RadiationEnclosure & expected,
                     const ThermFile::Mesh::Segment::RadiationEnclosure & result,
                     double tolerance);

    void expect_near(const ThermFile::Mesh::BCSegments & expected,
                     const ThermFile::Mesh::BCSegments & result,
                     double tolerance);

    void expect_near(const ThermFile::Mesh::Case & expected, const ThermFile::Mesh::Case & result, double tolerance);

    void expect_near(const ThermFile::Mesh::Mesh & expected, const ThermFile::Mesh::Mesh & result, double tolerance);

    struct MockPoint
    {
        std::string index;
        std::string x;
        std::string y;

        [[maybe_unused]] MockPoint(std::string index, std::string x, std::string y);
    };

    Helper::MockNode generateMockPoint(const MockPoint & node);

    struct MockElement
    {
        std::string index;
        std::string node1;
        std::string node2;
        std::string node3;
        std::string node4;
        std::string materialID;

        [[maybe_unused]] MockElement(std::string index,
                                     std::string node1,
                                     std::string node2,
                                     std::string node3,
                                     std::string node4,
                                     std::string materialID);
    };

    Helper::MockNode generateMockElement(const MockElement & element);

    struct MockSegmentFlux
    {
        std::string index;
        std::string node1;
        std::string node2;
        std::string flux1;
        std::string flux2;
        std::string label;

        [[maybe_unused]] MockSegmentFlux(std::string index,
                                         std::string node1,
                                         std::string node2,
                                         std::string flux1,
                                         std::string flux2,
                                         std::string label);
    };

    Helper::MockNode generateMockSegmentFlux(const MockSegmentFlux & segmentFlux);

    struct MockSegmentTemperature
    {
        std::string node;
        std::string temperature;
        std::string label;

        [[maybe_unused]] MockSegmentTemperature(std::string node, std::string temperature, std::string label);
    };

    Helper::MockNode generateMockSegmentTemperature(const MockSegmentTemperature & segmentTemperature);

    struct MockSegmentConvection
    {
        std::string index;
        std::string node1;
        std::string node2;
        std::string filmCoefficient;
        std::string temperature1;
        std::string temperature2;
        std::string label;

        [[maybe_unused]] MockSegmentConvection(std::string index,
                                               std::string node1,
                                               std::string node2,
                                               std::string filmCoefficient,
                                               std::string temperature1,
                                               std::string temperature2,
                                               std::string label);
    };

    Helper::MockNode generateMockSegmentConvection(const MockSegmentConvection & segmentConvection);

    struct MockRadiationSegment
    {
        std::string index;
        std::string node1;
        std::string node2;
        std::string filmCoefficient;
        std::string temperature1;
        std::string temperature2;

        [[maybe_unused]] MockRadiationSegment(std::string index,
                                              std::string node1,
                                              std::string node2,
                                              std::string filmCoefficient,
                                              std::string temperature1,
                                              std::string temperature2);
    };

    Helper::MockNode generateMockRadiationSegments(const MockRadiationSegment & radiationSegments);

    struct MockEnclosureRadiationSegment
    {
        std::string index;
        std::string node1;
        std::string node2;
        std::string condCode;
        std::string emissivity;
        std::string temperature;

        [[maybe_unused]] MockEnclosureRadiationSegment(std::string index,
                                                        std::string node1,
                                                        std::string node2,
                                                        std::string condCode,
                                                        std::string emissivity,
                                                        std::string temperature);
    };

    Helper::MockNode
      generateMockEnclosureRadiationSegments(const MockEnclosureRadiationSegment & enclosureRadiationSegments);

    struct MockBCSegments
    {
        std::vector<MockSegmentFlux> fluxes;
        std::vector<MockSegmentTemperature> temperatures;
        std::vector<MockSegmentConvection> convections;
        std::vector<MockRadiationSegment> radiations;
        std::vector<MockEnclosureRadiationSegment> enclosureRadiations;

        [[maybe_unused]] MockBCSegments(std::vector<MockSegmentFlux> fluxes,
                                        std::vector<MockSegmentTemperature> temperatures,
                                        std::vector<MockSegmentConvection> convections,
                                        std::vector<MockRadiationSegment> radiations,
                                        std::vector<MockEnclosureRadiationSegment> enclosureRadiations);
    };

    Helper::MockNode generateMockBCSegments(const MockBCSegments & bcSegments);

    struct MockMeshCase
    {
        std::string modelType;
        std::vector<MockPoint> nodes;
        std::vector<MockElement> elements;
        MockBCSegments bcSegments;

        [[maybe_unused]] MockMeshCase(std::string modelType,
                                      std::vector<MockPoint> nodes,
                                      std::vector<MockElement> elements,
                                      MockBCSegments bcSegments);
    };

    Helper::MockNode generateMockMeshCase(const MockMeshCase & mesh);

    struct MockMesh
    {
        std::vector<MockMeshCase> cases;

        [[maybe_unused]] MockMesh(std::vector<MockMeshCase> cases);
    };

    Helper::MockNode generateMockMesh(const MockMesh & mesh);
}   // namespace Helper::Mesh