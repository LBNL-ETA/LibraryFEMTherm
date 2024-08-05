#include <gtest/gtest.h>

#include "THMXMeshHelper.hxx"

#include "THMZ/Mesh/Mesh.hxx"

#include "MockNodeAdapter.hxx"

namespace Helper::Mesh
{
    void expect_near(const ThermFile::Mesh::Node & expected, const ThermFile::Mesh::Node & result, double tolerance)
    {
        EXPECT_EQ(expected.index, result.index);
        EXPECT_NEAR(expected.x, result.x, tolerance);
        EXPECT_NEAR(expected.y, result.y, tolerance);
    }

    void expect_eq(const ThermFile::Mesh::Element & expected, const ThermFile::Mesh::Element & result)
    {
        EXPECT_EQ(expected.index, result.index);
        EXPECT_EQ(expected.node1, result.node1);
        EXPECT_EQ(expected.node2, result.node2);
        EXPECT_EQ(expected.node3, result.node3);
        EXPECT_EQ(expected.node4, result.node4);
        EXPECT_EQ(expected.materialID, result.materialID);
    }

    void expect_near(const ThermFile::Mesh::Segment::Flux & expected,
                     const ThermFile::Mesh::Segment::Flux & result,
                     double tolerance)
    {
        EXPECT_EQ(expected.index, result.index);
        EXPECT_EQ(expected.node1, result.node1);
        EXPECT_EQ(expected.node2, result.node2);
        EXPECT_NEAR(expected.flux1, result.flux1, tolerance);
        EXPECT_NEAR(expected.flux2, result.flux2, tolerance);
        EXPECT_EQ(expected.label, result.label);
    }

    void expect_near(const ThermFile::Mesh::Segment::Temperature & expected,
                     const ThermFile::Mesh::Segment::Temperature & result,
                     double tolerance)
    {
        EXPECT_EQ(expected.node, result.node);
        EXPECT_NEAR(expected.temperature, result.temperature, tolerance);
        EXPECT_EQ(expected.label, result.label);
    }

    void expect_near(const ThermFile::Mesh::Segment::Convection & expected,
                     const ThermFile::Mesh::Segment::Convection & result,
                     double tolerance)
    {
        EXPECT_EQ(expected.index, result.index);
        EXPECT_EQ(expected.node1, result.node1);
        EXPECT_EQ(expected.node2, result.node2);
        EXPECT_NEAR(expected.filmCoefficient, result.filmCoefficient, tolerance);
        EXPECT_NEAR(expected.temperature1, result.temperature1, tolerance);
        EXPECT_NEAR(expected.temperature2, result.temperature2, tolerance);
        EXPECT_EQ(expected.label, result.label);
    }

    void expect_near(const ThermFile::Mesh::Segment::Radiation & expected,
                     const ThermFile::Mesh::Segment::Radiation & result,
                     double tolerance)
    {
        EXPECT_EQ(expected.index, result.index);
        EXPECT_EQ(expected.node1, result.node1);
        EXPECT_EQ(expected.node2, result.node2);
        EXPECT_NEAR(expected.filmCoefficient, result.filmCoefficient, tolerance);
        EXPECT_NEAR(expected.temperature1, result.temperature1, tolerance);
        EXPECT_NEAR(expected.temperature2, result.temperature2, tolerance);
    }

    void expect_near(const ThermFile::Mesh::Segment::RadiationEnclosure & expected,
                     const ThermFile::Mesh::Segment::RadiationEnclosure & result,
                     double tolerance)
    {
        EXPECT_EQ(expected.index, result.index);
        EXPECT_EQ(expected.node1, result.node1);
        EXPECT_EQ(expected.node2, result.node2);
        EXPECT_EQ(expected.condCode, result.condCode);
        EXPECT_NEAR(expected.emissivity, result.emissivity, tolerance);
        EXPECT_NEAR(expected.temperature, result.temperature, tolerance);
    }

    void expect_near(const ThermFile::Mesh::BCSegments & expected,
                     const ThermFile::Mesh::BCSegments & result,
                     double tolerance)
    {
        if(expected.fluxes.has_value())
        {
            ASSERT_TRUE(result.fluxes.has_value());
            ASSERT_EQ(expected.fluxes->size(), result.fluxes->size());
            for(size_t i = 0; i < expected.fluxes->size(); ++i)
            {
                expect_near((*expected.fluxes)[i], (*result.fluxes)[i], tolerance);
            }
        }
        else
        {
            EXPECT_FALSE(result.fluxes.has_value());
        }
    }

    void expect_near(const ThermFile::Mesh::Case & expected, const ThermFile::Mesh::Case & result, double tolerance)
    {
        EXPECT_EQ(expected.modelType, result.modelType);
        ASSERT_EQ(expected.nodes.size(), result.nodes.size());
        for(size_t i = 0; i < expected.nodes.size(); ++i)
        {
            expect_near(expected.nodes[i], result.nodes[i], tolerance);
        }

        ASSERT_EQ(expected.elements.size(), result.elements.size());
        for(size_t i = 0; i < expected.elements.size(); ++i)
        {
            expect_eq(expected.elements[i], result.elements[i]);
        }
    }

    void expect_near(const ThermFile::Mesh::Mesh & expected, const ThermFile::Mesh::Mesh & result, double tolerance)
    {
        ASSERT_EQ(expected.cases.size(), result.cases.size());
        for(size_t i = 0; i < expected.cases.size(); ++i)
        {
            expect_near(expected.cases[i], result.cases[i], tolerance);
        }
    }

    MockPoint::MockPoint(std::string index, std::string x, std::string y) :
        index(std::move(index)), x(std::move(x)), y(std::move(y))
    {}

    Helper::MockNode generateMockPoint(const Mesh::MockPoint & point)
    {
        Helper::MockNode node{"Node"};
        addChildNode(node, "index", point.index);
        addChildNode(node, "x", point.x);
        addChildNode(node, "y", point.y);

        return node;
    }

    MockElement::MockElement(std::string index,
                             std::string node1,
                             std::string node2,
                             std::string node3,
                             std::string node4,
                             std::string materialID) :
        index(std::move(index)),
        node1(std::move(node1)),
        node2(std::move(node2)),
        node3(std::move(node3)),
        node4(std::move(node4)),
        materialID(std::move(materialID))
    {}

    Helper::MockNode generateMockElement(const MockElement & element)
    {
        Helper::MockNode node{"Element"};
        addChildNode(node, "index", element.index);
        addChildNode(node, "node1", element.node1);
        addChildNode(node, "node2", element.node2);
        addChildNode(node, "node3", element.node3);
        addChildNode(node, "node4", element.node4);
        addChildNode(node, "materialID", element.materialID);

        return node;
    }

    MockSegmentFlux::MockSegmentFlux(std::string index,
                                     std::string node1,
                                     std::string node2,
                                     std::string flux1,
                                     std::string flux2,
                                     std::string label) :
        index(std::move(index)),
        node1(std::move(node1)),
        node2(std::move(node2)),
        flux1(std::move(flux1)),
        flux2(std::move(flux2)),
        label(std::move(label))
    {}


    Helper::MockNode generateMockSegmentFlux(const MockSegmentFlux & segmentFlux)
    {
        Helper::MockNode node{"Flux"};
        addChildNode(node, "index", segmentFlux.index);
        addChildNode(node, "node1", segmentFlux.node1);
        addChildNode(node, "node2", segmentFlux.node2);
        addChildNode(node, "flux1", segmentFlux.flux1);
        addChildNode(node, "flux2", segmentFlux.flux2);
        addChildNode(node, "label", segmentFlux.label);

        return node;
    }

    MockSegmentTemperature::MockSegmentTemperature(std::string node, std::string temperature, std::string label) :
        node(std::move(node)), temperature(std::move(temperature)), label(std::move(label))
    {}

    Helper::MockNode generateMockSegmentTemperature(const MockSegmentTemperature & segmentTemperature)
    {
        Helper::MockNode node{"Temperature"};
        addChildNode(node, "node", segmentTemperature.node);
        addChildNode(node, "temperature", segmentTemperature.temperature);
        addChildNode(node, "label", segmentTemperature.label);

        return node;
    }

    MockSegmentConvection::MockSegmentConvection(std::string index,
                                                 std::string node1,
                                                 std::string node2,
                                                 std::string filmCoefficient,
                                                 std::string temperature1,
                                                 std::string temperature2,
                                                 std::string label) :
        index(std::move(index)),
        node1(std::move(node1)),
        node2(std::move(node2)),
        filmCoefficient(std::move(filmCoefficient)),
        temperature1(std::move(temperature1)),
        temperature2(std::move(temperature2)),
        label(std::move(label))
    {}

    Helper::MockNode generateMockSegmentConvection(const MockSegmentConvection & segmentConvection)
    {
        Helper::MockNode node{"Convection"};
        addChildNode(node, "index", segmentConvection.index);
        addChildNode(node, "node1", segmentConvection.node1);
        addChildNode(node, "node2", segmentConvection.node2);
        addChildNode(node, "filmCoefficient", segmentConvection.filmCoefficient);
        addChildNode(node, "temperature1", segmentConvection.temperature1);
        addChildNode(node, "temperature2", segmentConvection.temperature2);
        addChildNode(node, "label", segmentConvection.label);

        return node;
    }

    MockRadiationSegment::MockRadiationSegment(std::string index,
                                                 std::string node1,
                                                 std::string node2,
                                                 std::string filmCoefficient,
                                                 std::string temperature1,
                                                 std::string temperature2) :
        index(std::move(index)),
        node1(std::move(node1)),
        node2(std::move(node2)),
        filmCoefficient(std::move(filmCoefficient)),
        temperature1(std::move(temperature1)),
        temperature2(std::move(temperature2))
    {}

    Helper::MockNode generateMockRadiationSegments(const MockRadiationSegment & radiationSegments)
    {
        Helper::MockNode node{"Radiation"};
        addChildNode(node, "index", radiationSegments.index);
        addChildNode(node, "node1", radiationSegments.node1);
        addChildNode(node, "node2", radiationSegments.node2);
        addChildNode(node, "filmCoefficient", radiationSegments.filmCoefficient);
        addChildNode(node, "temperature1", radiationSegments.temperature1);
        addChildNode(node, "temperature2", radiationSegments.temperature2);

        return node;
    }

    MockEnclosureRadiationSegment::MockEnclosureRadiationSegment(std::string index,
                                                                   std::string node1,
                                                                   std::string node2,
                                                                   std::string condCode,
                                                                   std::string emissivity,
                                                                   std::string temperature) :
        index(std::move(index)),
        node1(std::move(node1)),
        node2(std::move(node2)),
        condCode(std::move(condCode)),
        emissivity(std::move(emissivity)),
        temperature(std::move(temperature))
    {}

    Helper::MockNode
      generateMockEnclosureRadiationSegments(const MockEnclosureRadiationSegment & enclosureRadiationSegments)
    {
        Helper::MockNode node{"RadiationEnclosure"};
        addChildNode(node, "index", enclosureRadiationSegments.index);
        addChildNode(node, "node1", enclosureRadiationSegments.node1);
        addChildNode(node, "node2", enclosureRadiationSegments.node2);
        addChildNode(node, "condCode", enclosureRadiationSegments.condCode);
        addChildNode(node, "emissivity", enclosureRadiationSegments.emissivity);
        addChildNode(node, "temperature", enclosureRadiationSegments.temperature);

        return node;
    }

    MockBCSegments::MockBCSegments(std::vector<MockSegmentFlux> fluxes,
                                   std::vector<MockSegmentTemperature> temperatures,
                                   std::vector<MockSegmentConvection> convections,
                                   std::vector<MockRadiationSegment> radiations,
                                   std::vector<MockEnclosureRadiationSegment> enclosureRadiations) :
        fluxes(std::move(fluxes)),
        temperatures(std::move(temperatures)),
        convections(std::move(convections)),
        radiations(std::move(radiations)),
        enclosureRadiations(std::move(enclosureRadiations))
    {}

    Helper::MockNode generateMockBCSegments(const Mesh::MockBCSegments & bcSegments)
    {
        Helper::MockNode node{"BCSegments"};
        auto & fluxes = addChildNode(node, "Fluxes");
        for(const auto & flux : bcSegments.fluxes)
        {
            addChildNode(fluxes, "Flux", generateMockSegmentFlux(flux));
        }

        auto & temperatures = addChildNode(node, "Temperatures");
        for(const auto & temperature : bcSegments.temperatures)
        {
            addChildNode(temperatures, generateMockSegmentTemperature(temperature));
        }

        auto & convections = addChildNode(node, "Convections");
        for(const auto & convection : bcSegments.convections)
        {
            addChildNode(convections, generateMockSegmentConvection(convection));
        }

        auto & radiations = addChildNode(node, "Radiations");
        for(const auto & radiation : bcSegments.radiations)
        {
            addChildNode(radiations, generateMockRadiationSegments(radiation));
        }

        auto & enclosureRadiations = addChildNode(node, "RadiationEnclosures");
        for(const auto & enclosureRadiation : bcSegments.enclosureRadiations)
        {
            addChildNode(enclosureRadiations, generateMockEnclosureRadiationSegments(enclosureRadiation));
        }

        return node;
    }

    MockMeshCase::MockMeshCase(std::string modelType,
                               std::vector<MockPoint> nodes,
                               std::vector<MockElement> elements,
                               MockBCSegments bcSegments) :
        modelType(std::move(modelType)),
        nodes(std::move(nodes)),
        elements(std::move(elements)),
        bcSegments(std::move(bcSegments))
    {}

    Helper::MockNode generateMockMeshCase(const MockMeshCase & mesh)
    {
        Helper::MockNode node{"Case"};
        addChildNode(node, "ModelType", mesh.modelType);
        auto & nodes = addChildNode(node, "Nodes");
        for(const auto & point : mesh.nodes)
        {
            addChildNode(nodes, "Node", generateMockPoint(point));
        }

        auto & elements = addChildNode(node, "Elements");
        for(const auto & element : mesh.elements)
        {
            addChildNode(elements, "Element", generateMockElement(element));
        }

        addChildNode(node, generateMockBCSegments(mesh.bcSegments));

        return node;
    }

    MockMesh::MockMesh(std::vector<MockMeshCase> cases) : cases(std::move(cases))
    {}

    Helper::MockNode generateMockMesh(const Mesh::MockMesh & mesh)
    {
        Helper::MockNode node{"Mesh"};
        auto & cases = addChildNode(node, "Cases");
        for(const auto & meshCase : mesh.cases)
        {
            addChildNode(cases, generateMockMeshCase(meshCase));
        }

        return node;
    }
}   // namespace Helper::Mesh
