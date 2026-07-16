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
    inputGeometry.loadFromFile(fileName);

    const auto numOfMaterials{inputGeometry.materials.size()};
    EXPECT_EQ(numOfMaterials, 1u);

    const auto numOfNodes{inputGeometry.nodes.size()};
    EXPECT_EQ(numOfNodes, 20u);

    const auto numOfElements{inputGeometry.elements.size()};
    EXPECT_EQ(numOfElements, 12u);

    const auto numOfMaterialPolygons{inputGeometry.materialPolygons.size()};
    EXPECT_EQ(numOfMaterialPolygons, 1u);

    const auto numOfEnclosureNodes{inputGeometry.modelEnclosure.size()};
    EXPECT_EQ(numOfEnclosureNodes, 14u);

    std::filesystem::remove(fileName);
}

TEST_F(TestGeometryXMLReading, ElementConnectivityIsPreserved)
{
    // Regression: the four repeated <NodeID> tags of an element must read back as four distinct
    // corners. They used to collapse onto the first tag (FileParse's scalar Child read ignores
    // the occurrence index), which produced zero-area elements in the THERM transient display.
    const std::string fileContent{TestGeometry::testGeometry1()};
    const std::string fileName{"TestConnectivity.xml"};

    std::filesystem::remove(fileName);

    File::createFileFromString(fileName, fileContent);

    GeometryLibrary::InputGeometryDataRecord inputGeometry;
    inputGeometry.loadFromFile(fileName);

    ASSERT_GE(inputGeometry.elements.size(), 4u);

    const auto & elementThree{inputGeometry.elements[2]};
    EXPECT_EQ(elementThree.id, 3u);
    EXPECT_EQ(elementThree.nodeID1, 6u);
    EXPECT_EQ(elementThree.nodeID2, 3u);
    EXPECT_EQ(elementThree.nodeID3, 5u);
    EXPECT_EQ(elementThree.nodeID4, 9u);

    const auto & elementFour{inputGeometry.elements[3]};
    EXPECT_EQ(elementFour.id, 4u);
    EXPECT_EQ(elementFour.nodeID1, 4u);
    EXPECT_EQ(elementFour.nodeID2, 7u);
    EXPECT_EQ(elementFour.nodeID3, 11u);
    EXPECT_EQ(elementFour.nodeID4, 8u);

    std::filesystem::remove(fileName);
}