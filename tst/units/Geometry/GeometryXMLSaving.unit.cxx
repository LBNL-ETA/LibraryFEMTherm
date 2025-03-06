#include <filesystem>
#include <gtest/gtest.h>

#include "GeometryForResults/Geometry.hxx"
#include "GeometryForResults/DB.hxx"

#include "geometry1.hxx"

class TestGeometryXMLSaving : public testing::Test
{
protected:
    void SetUp() override
    {}
};

TEST_F(TestGeometryXMLSaving, SaveGeometryXML)
{
    const std::string fileName{"TestRead.xm"};

    std::filesystem::remove(fileName);

    GeometryLibrary::InputGeometryDataRecord inGeom;
    inGeom.add(1u, "Test name", "ff8080");
    inGeom.addNode(1u, 0, 0);
    inGeom.addNode(2u, 0, 1);
    inGeom.addNode(3u, 1, 1);
    inGeom.addNode(4u, 2, 1);

    inGeom.addElement(1u, 1u, 2u, 3u, 4u, 1u);

    GeometryLibrary::MaterialPolygon matPoly{.id = 1u, .materialID = 1u};
    matPoly.m_Nodes.insert(std::begin(matPoly.m_Nodes), {1u, 2u, 3u, 4u});
    inGeom.addPolygon(matPoly);

    inGeom.modelEnclosure = {1u, 2u, 3u, 4u};

    const auto error{inGeom.saveToXML(fileName)};
    EXPECT_EQ(error, 0);

    GeometryLibrary::InputGeometryDataRecord inputGeometry;

    inputGeometry.loadFromXMLFile(fileName);

    const auto matName{inputGeometry.materials[0].Name};
    EXPECT_EQ(matName, "Test name");

    const auto numOfMaterials{inputGeometry.materials.size()};
    EXPECT_EQ(numOfMaterials, 1u);

    const auto numOfNodes{inputGeometry.nodes.size()};
    EXPECT_EQ(numOfNodes, 4u);

    const auto numOfElements{inputGeometry.elements.size()};
    EXPECT_EQ(numOfElements, 1u);

    const auto numOfMaterialPolygons{inputGeometry.materialPolygons.size()};
    EXPECT_EQ(numOfMaterialPolygons, 1u);

    const auto numOfEnclosureNodes{inputGeometry.modelEnclosure.size()};
    EXPECT_EQ(numOfEnclosureNodes, 4u);

    std::filesystem::remove(fileName);
}