#pragma once

#include <string>

namespace ThermFile
{
    struct Point;
}

namespace Helper
{
    struct MockNode;

    void expect_near(const ThermFile::Point & expected, const ThermFile::Point & actual, double tolerance);

    void expect_eq(const ThermFile::GlazingSystemData & expected, const ThermFile::GlazingSystemData & actual);

    void expect_near(const ThermFile::Polygon & expected, const ThermFile::Polygon & actual, double tolerance);

    void expect_near(const ThermFile::SurfaceData & expected, const ThermFile::SurfaceData & actual, double tolerance);

    void
      expect_near(const ThermFile::BuildingData & expected, const ThermFile::BuildingData & actual, double tolerance);

    void expect_near(const ThermFile::ThermalEmissionProperties & expected,
                     const ThermFile::ThermalEmissionProperties & actual,
                     double tolerance);

    void expect_eq(const ThermFile::ShadeData & expected, const ThermFile::ShadeData & actual);

    void
      expect_near(const ThermFile::TransientData & expected, const ThermFile::TransientData & actual, double tolerance);

    void expect_near(const ThermFile::Boundary & expected,
                     const ThermFile::Boundary & actual,
                     double tolerance);

    struct MockPointNode
    {
        std::string x;
        std::string y;

        [[maybe_unused]] MockPointNode(std::string x, std::string y);
    };

    //! \brief Generate a node with the given x and y coordinates
    //! \param geometryNode The node to generate
    Helper::MockNode generatePointNode(const MockPointNode & geometryNode);

    //! \brief Mock glazing system data
    //! \param ID The ID of the glazing system
    //! \param index The index of the glazing system
    struct MockGlazingSystemData
    {
        std::string ID;
        std::string index;

        [[maybe_unused]] MockGlazingSystemData(std::string ID, std::string index);
    };

    //! \brief Generate a glazing system node for testing
    //! \param glazingSystemData The glazing system data
    Helper::MockNode generateGlazingSystemNode(const MockGlazingSystemData & glazingSystemData);

    using MockPoints = std::vector<MockPointNode>;
    using MockAttributes = std::vector<std::string_view>;

    struct MockPolygon
    {
        std::string uuid;
        std::string id;
        std::string materialUUID;
        std::string name;
        MockGlazingSystemData glazingSystem;
        MockPointNode origin;
        MockPoints points;
        MockAttributes attributes;
        std::string polygonType;

        [[maybe_unused]] MockPolygon(std::string uuid,
                                     std::string id,
                                     std::string materialUUID,
                                     std::string name,
                                     MockGlazingSystemData glazingSystem,
                                     MockPointNode origin,
                                     MockPoints points,
                                     MockAttributes attributes,
                                     std::string polygonType);
    };

    Helper::MockNode generatePolygonNode(const MockPolygon & polygon);

    struct MockSurfaceData
    {
        std::string tilt;
        std::string length;

        [[maybe_unused]] MockSurfaceData(std::string tilt, std::string length);
    };

    Helper::MockNode generateSurfaceDataNode(MockSurfaceData && surfaceData);

    struct MockBuildingData
    {
        std::string width;
        std::string height;

        [[maybe_unused]] MockBuildingData(std::string width, std::string height);
    };

    Helper::MockNode generateBuildingDataNode(MockBuildingData && buildingData);

    struct MockThermalEmissionProperties
    {
        std::string emissivity;
        std::string temperature;
        std::string useGlobalEmissivity;

        [[maybe_unused]] MockThermalEmissionProperties(std::string emissivity,
                                                       std::string temperature,
                                                       std::string useGlobalEmissivity);
    };

    Helper::MockNode generateThermalEmissionPropertiesNode(MockThermalEmissionProperties && thermalEmissionProperties);

    struct MockShadeData
    {
        std::string modifier;
        std::string modifierType;

        [[maybe_unused]] MockShadeData(std::string modifier, std::string modifierType);
    };

    Helper::MockNode generateShadeDataNode(MockShadeData && shadeData);

    struct MockTransientData
    {
        std::string uuid;
        std::string timestepFileName;
        MockSurfaceData surfaceData;
        MockBuildingData buildingData;


        [[maybe_unused]] MockTransientData(std::string uuid,
                                           std::string timestepFileName,
                                           MockSurfaceData && surfaceData,
                                           MockBuildingData && buildingData);
    };

    Helper::MockNode generateTransientDataNode(MockTransientData && transientData);

    struct MockBoundaryCondition
    {
        std::string id;
        std::string uuid;
        std::string name;
        std::string fluxTag;
        std::string isBlocking;
        std::string neighborPolygonUUID;
        MockPointNode origin;
        MockPointNode startPoint;
        MockPointNode endPoint;
        std::string side;
        MockThermalEmissionProperties thermalEmissionProperties;
        MockShadeData shadeData;
        std::string isIlluminated;
        std::string materialSide;
        MockTransientData transientData;
        std::string edgeID;
        std::string enclosureNumber;
        std::string surfaceType;
        std::string color;
        std::string status;

        [[maybe_unused]] MockBoundaryCondition(std::string id,
                                               std::string uuid,
                                               std::string name,
                                               std::string fluxTag,
                                               std::string isBlocking,
                                               std::string neighborPolygonUUID,
                                               MockPointNode && origin,
                                               MockPointNode && startPoint,
                                               MockPointNode && endPoint,
                                               std::string side,
                                               MockThermalEmissionProperties && thermalEmissionProperties,
                                               MockShadeData && shadeData,
                                               std::string isIlluminated,
                                               std::string materialSide,
                                               MockTransientData && transientData,
                                               std::string edgeID,
                                               std::string enclosureNumber,
                                               std::string surfaceType,
                                               std::string color,
                                               std::string status);
    };

    Helper::MockNode generateBoundaryNode(MockBoundaryCondition && boundaryCondition);

}   // namespace Helper