#include <filesystem>
#include <gtest/gtest.h>

#include "GeometryForResults/Geometry.hxx"
#include "GeometryForResults/DB.hxx"
#include "LibraryUtilities/FileManipulation.hxx"

#include "geometry1.hxx"

class TestGeometryXMLReading : public testing::Test
{
protected:

    void SetUp() override
    {}
};

TEST_F(TestGeometryXMLReading, ReadGeomteryXML)
{
    const std::string fileContent{TestGeometry::testGeometry1()};
    const std::string fileName{"TestRead.xml"};

    std::filesystem::remove(fileName);

    File::createFileFromString(fileName, fileContent);

    GeometryLibrary::InputGeometryDataRecord inputGeometry;
    inputGeometry.loadFromXMLFile(fileName);

    const auto numOfMaterials{inputGeometry.materials.size()};
    EXPECT_EQ(numOfMaterials, 1u);

    const auto numOfNodes{inputGeometry.nodes.size()};
    EXPECT_EQ(numOfNodes, 20u);

    const auto numOfElements{inputGeometry.elements.size()};
    EXPECT_EQ(numOfElements, 12u);

    const auto numOfMaterialPolygons{inputGeometry.materialPolygons.size()};
    EXPECT_EQ(numOfMaterialPolygons, 1u);

    const auto numOfEnclosureNodes{inputGeometry.modelEnclosure.node.size()};
    EXPECT_EQ(numOfEnclosureNodes, 14u);

    std::filesystem::remove(fileName);
}