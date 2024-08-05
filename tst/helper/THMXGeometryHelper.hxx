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

    void expect_near(const ThermFile::Cavity & expected, const ThermFile::Cavity & actual, double tolerance);

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

    void expect_near(const ThermFile::BoundaryCondition & expected,
                     const ThermFile::BoundaryCondition & actual,
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

    struct MockCavity
    {
        std::string uuid;
        std::string direction;
        std::string heatFlowDirection;
        std::string emissivity1;
        std::string emissivity2;
        std::string temperature1;
        std::string temperature2;
        std::string maxXDimension;
        std::string maxYDimension;
        std::string actualHeight;
        std::string area;
        std::string localEmissivities;
        std::string pressure;
        MockPointNode warmLocator;
        MockPointNode coldLocator;

        [[maybe_unused]] MockCavity(std::string uuid,
                                    std::string direction,
                                    std::string heatFlowDirection,
                                    std::string emissivity1,
                                    std::string emissivity2,
                                    std::string temperature1,
                                    std::string temperature2,
                                    std::string maxXDimension,
                                    std::string maxYDimension,
                                    std::string actualHeight,
                                    std::string area,
                                    std::string localEmissivities,
                                    std::string pressure,
                                    MockPointNode warmLocator,
                                    MockPointNode coldLocator);
    };

    //! \brief Generation of a cavity node for testing
    //! \param uuid The UUID of the cavity
    //! \param heatFlowDirection The heat flow direction of the cavity
    //! \param emissivity1 The emissivity of the first surface
    //! \param emissivity2 The emissivity of the second surface
    //! \param temperature1 The temperature of the first surface
    //! \param temperature2 The temperature of the second surface
    //! \param maxXDimension The maximum x dimension of the cavity
    //! \param maxYDimension The maximum y dimension of the cavity
    //! \param actualHeight The actual height of the cavity
    //! \param area The area of the cavity
    //! \param localEmissivities The local emissivities of the cavity
    //! \param pressure The pressure of the cavity
    Helper::MockNode generateCavityWithoutDirectionNode(const MockCavity & cavity);

    // Generates the same object as generateCavityNode, but with a direction
    Helper::MockNode generateCavityNodeWithDirection(const MockCavity & cavity);

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
        std::string cavityUUID;
        MockAttributes attributes;
        std::string polygonType;

        [[maybe_unused]] MockPolygon(std::string uuid,
                                     std::string id,
                                     std::string materialUUID,
                                     std::string name,
                                     MockGlazingSystemData glazingSystem,
                                     MockPointNode origin,
                                     MockPoints points,
                                     std::string cavityUUID,
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
        MockPoints points;
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
                                               MockPoints && points,
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

    Helper::MockNode generateBoundaryConditionNode(MockBoundaryCondition && boundaryCondition);

}   // namespace Helper