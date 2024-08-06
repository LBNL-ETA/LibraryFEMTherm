#include <gtest/gtest.h>

#include <THMZ/Geometry/Geometry.hxx>
#include <utility>

#include "THMXGeometryHelper.hxx"
#include "MockNodeAdapter.hxx"


namespace Helper
{
    ///////////////////////////////////////////////////////////////////////////
    /// Test functions
    ///////////////////////////////////////////////////////////////////////////

    void expect_near(const ThermFile::Point & expected, const ThermFile::Point & actual, const double tolerance)
    {
        EXPECT_NEAR(expected.x, actual.x, tolerance);
        EXPECT_NEAR(expected.y, actual.y, tolerance);
    }

    void expect_near(const ThermFile::Cavity & expected, const ThermFile::Cavity & actual, const double tolerance)
    {
        EXPECT_EQ(expected.uuid, actual.uuid);
        EXPECT_EQ(expected.heatFlowDirection, actual.heatFlowDirection);
        EXPECT_NEAR(expected.emissivity1, actual.emissivity1, tolerance);
        EXPECT_NEAR(expected.emissivity2, actual.emissivity2, tolerance);
        EXPECT_NEAR(expected.temperature1, actual.temperature1, tolerance);
        EXPECT_NEAR(expected.temperature2, actual.temperature2, tolerance);
        EXPECT_NEAR(expected.maxXDimension, actual.maxXDimension, tolerance);
        EXPECT_NEAR(expected.maxYDimension, actual.maxYDimension, tolerance);
        EXPECT_NEAR(expected.actualHeight, actual.actualHeight, tolerance);
        EXPECT_NEAR(expected.area, actual.area, tolerance);
        EXPECT_EQ(expected.localEmissivities, actual.localEmissivities);
        EXPECT_NEAR(expected.pressure, actual.pressure, tolerance);
    }

    void expect_eq(const ThermFile::GlazingSystemData & expected, const ThermFile::GlazingSystemData & actual)
    {
        EXPECT_EQ(expected.ID, actual.ID);
        EXPECT_EQ(expected.index, actual.index);
    }

    void expect_near(const ThermFile::Polygon & expected, const ThermFile::Polygon & actual, const double tolerance)
    {
        EXPECT_EQ(expected.materialName, actual.materialName);
        EXPECT_EQ(expected.materialUUID, actual.materialUUID);
        EXPECT_EQ(expected.glazingSystem.has_value(), actual.glazingSystem.has_value());
        if(expected.glazingSystem.has_value() && actual.glazingSystem.has_value())
        {
            expect_eq(*expected.glazingSystem, *actual.glazingSystem);
        }
        expect_near(expected.origin, actual.origin, tolerance);
        ASSERT_EQ(expected.points.size(), actual.points.size());
        for(size_t i = 0; i < expected.points.size(); ++i)
        {
            expect_near(expected.points[i], actual.points[i], tolerance);
        }
        EXPECT_EQ(expected.cavityUUID, actual.cavityUUID);
        ASSERT_EQ(expected.attributes.size(), actual.attributes.size());
        for(size_t i = 0; i < expected.attributes.size(); ++i)
        {
            EXPECT_EQ(expected.attributes[i], actual.attributes[i]);
        }
        EXPECT_EQ(expected.polygonType, actual.polygonType);
    }

    void expect_near(const ThermFile::SurfaceData & expected,
                     const ThermFile::SurfaceData & actual,
                     const double tolerance)
    {
        EXPECT_NEAR(expected.tilt, actual.tilt, tolerance);
        EXPECT_NEAR(expected.length, actual.length, tolerance);
    }

    void expect_near(const ThermFile::BuildingData & expected,
                     const ThermFile::BuildingData & actual,
                     const double tolerance)
    {
        EXPECT_NEAR(expected.width, actual.width, tolerance);
        EXPECT_NEAR(expected.height, actual.height, tolerance);
    }

    void expect_near(const ThermFile::ThermalEmissionProperties & expected,
                     const ThermFile::ThermalEmissionProperties & actual,
                     const double tolerance)
    {
        EXPECT_NEAR(expected.emissivity, actual.emissivity, tolerance);
        EXPECT_NEAR(expected.temperature, actual.temperature, tolerance);
        EXPECT_EQ(expected.useGlobalEmissivity, actual.useGlobalEmissivity);
    }

    void expect_eq(const ThermFile::ShadeData & expected, const ThermFile::ShadeData & actual)
    {
        EXPECT_EQ(expected.shadeModifier, actual.shadeModifier);
        EXPECT_EQ(expected.shadeModifierGlazingID, actual.shadeModifierGlazingID);
    }

    void
      expect_near(const ThermFile::TransientData & expected, const ThermFile::TransientData & actual, double tolerance)
    {
        EXPECT_EQ(expected.typeRecordUUID, actual.typeRecordUUID);
        EXPECT_EQ(expected.transientFileName, actual.transientFileName);
        ASSERT_EQ(expected.surfaceData.has_value(), actual.surfaceData.has_value());
        if(expected.surfaceData.has_value() && actual.surfaceData.has_value())
        {
            expect_near(*expected.surfaceData, *actual.surfaceData, tolerance);
        }
        ASSERT_EQ(expected.buildingData.has_value(), actual.buildingData.has_value());
        if(expected.buildingData.has_value() && actual.buildingData.has_value())
        {
            expect_near(*expected.buildingData, *actual.buildingData, tolerance);
        }
    }

    void expect_near(const ThermFile::Boundary & expected,
                     const ThermFile::Boundary & actual,
                     const double tolerance)
    {
        EXPECT_EQ(expected.ID, actual.ID);
        EXPECT_EQ(expected.uuid, actual.uuid);
        EXPECT_EQ(expected.name, actual.name);
        EXPECT_EQ(expected.fluxTag, actual.fluxTag);
        EXPECT_EQ(expected.isBlocking, actual.isBlocking);
        EXPECT_EQ(expected.neighborPolygonUUID, actual.neighborPolygonUUID);
        expect_near(expected.origin, actual.origin, tolerance);
        ASSERT_EQ(expected.points.size(), actual.points.size());
        for(size_t i = 0; i < expected.points.size(); ++i)
        {
            expect_near(expected.points[i], actual.points[i], tolerance);
        }
        EXPECT_EQ(expected.Side, actual.Side);
        expect_near(expected.thermalEmissionProperties, actual.thermalEmissionProperties, tolerance);
        if(expected.shadeData.has_value())
        {
            ASSERT_TRUE(actual.shadeData);
            expect_eq(*expected.shadeData, *actual.shadeData);
        }

        if(expected.isIlluminated.has_value())
        {
            ASSERT_TRUE(actual.isIlluminated);
            EXPECT_EQ(*expected.isIlluminated, *actual.isIlluminated);
        }

        if(expected.transientRecordData.has_value())
        {
            ASSERT_TRUE(actual.transientRecordData);
            expect_near(*expected.transientRecordData, *actual.transientRecordData, tolerance);
        }

        EXPECT_EQ(expected.EdgeID, actual.EdgeID);

        if(expected.enclosureNumber.has_value())
        {
            ASSERT_TRUE(actual.enclosureNumber);
            EXPECT_EQ(*expected.enclosureNumber, *actual.enclosureNumber);
        }

        EXPECT_EQ(expected.surfaceType, actual.surfaceType);
        EXPECT_EQ(expected.color, actual.color);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// Generation of mock nodes
    ///////////////////////////////////////////////////////////////////////////

    MockPointNode::MockPointNode(std::string x, std::string y) : x(std::move(x)), y(std::move(y))
    {}

    Helper::MockNode generatePointNode(const MockPointNode & geometryNode)
    {
        Helper::MockNode node{"Point"};
        addChildNode(node, "x", geometryNode.x);
        addChildNode(node, "y", geometryNode.y);

        return node;
    }

    MockCavity::MockCavity(std::string uuid,
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
                           MockPointNode coldLocator) :
        uuid(std::move(uuid)),
        direction(std::move(direction)),
        heatFlowDirection(std::move(heatFlowDirection)),
        emissivity1(std::move(emissivity1)),
        emissivity2(std::move(emissivity2)),
        temperature1(std::move(temperature1)),
        temperature2(std::move(temperature2)),
        maxXDimension(std::move(maxXDimension)),
        maxYDimension(std::move(maxYDimension)),
        actualHeight(std::move(actualHeight)),
        area(std::move(area)),
        localEmissivities(std::move(localEmissivities)),
        pressure(std::move(pressure)),
        warmLocator(std::move(warmLocator)),
        coldLocator(std::move(coldLocator))
    {}

    Helper::MockNode generateCavityWithoutDirectionNode(const MockCavity & cavity)
    {
        Helper::MockNode node{"Cavity"};
        addChildNode(node, "UUID", cavity.uuid);
        addChildNode(node, "HeatFlowDirection", cavity.heatFlowDirection);
        addChildNode(node, "Emissivity1", cavity.emissivity1);
        addChildNode(node, "Emissivity2", cavity.emissivity2);
        addChildNode(node, "Temperature1", cavity.temperature1);
        addChildNode(node, "Temperature2", cavity.temperature2);
        addChildNode(node, "MaxXDimension", cavity.maxXDimension);
        addChildNode(node, "MaxYDimension", cavity.maxYDimension);
        addChildNode(node, "ActualHeight", cavity.actualHeight);
        addChildNode(node, "Area", cavity.area);
        addChildNode(node, "LocalEmissivities", cavity.localEmissivities);
        addChildNode(node, "Pressure", cavity.pressure);
        auto warmLocatorNode = generatePointNode(cavity.warmLocator);
        warmLocatorNode.tag = "WarmLocator";
        addChildNode(node, warmLocatorNode);
        auto coldLocatorNode = generatePointNode(cavity.coldLocator);
        coldLocatorNode.tag = "ColdLocator";
        addChildNode(node, coldLocatorNode);

        return node;
    }

    Helper::MockNode generateCavityNodeWithDirection(const MockCavity & cavity)
    {
        Helper::MockNode node{"Cavity"};
        addChildNode(node, "UUID", cavity.uuid);
        addChildNode(node, "Direction", cavity.direction);
        addChildNode(node, "HeatFlowDirection", cavity.heatFlowDirection);
        addChildNode(node, "Emissivity1", cavity.emissivity1);
        addChildNode(node, "Emissivity2", cavity.emissivity2);
        addChildNode(node, "Temperature1", cavity.temperature1);
        addChildNode(node, "Temperature2", cavity.temperature2);
        addChildNode(node, "MaxXDimension", cavity.maxXDimension);
        addChildNode(node, "MaxYDimension", cavity.maxYDimension);
        addChildNode(node, "ActualHeight", cavity.actualHeight);
        addChildNode(node, "Area", cavity.area);
        addChildNode(node, "LocalEmissivities", cavity.localEmissivities);
        addChildNode(node, "Pressure", cavity.pressure);
        auto warmLocatorNode = generatePointNode(cavity.warmLocator);
        warmLocatorNode.tag = "WarmLocator";
        addChildNode(node, warmLocatorNode);
        auto coldLocatorNode = generatePointNode(cavity.coldLocator);
        coldLocatorNode.tag = "ColdLocator";
        addChildNode(node, coldLocatorNode);

        return node;
    }

    MockGlazingSystemData::MockGlazingSystemData(std::string ID, std::string index) :
        ID(std::move(ID)), index(std::move(index))
    {}

    Helper::MockNode generateGlazingSystemNode(const MockGlazingSystemData & glazingSystemData)
    {
        Helper::MockNode node{"GlazingSystemData"};
        addChildNode(node, "ID", glazingSystemData.ID);
        addChildNode(node, "Index", glazingSystemData.index);

        return node;
    }

    MockPolygon::MockPolygon(std::string uuid,
                             std::string id,
                             std::string materialUUID,
                             std::string name,
                             MockGlazingSystemData glazingSystem,
                             MockPointNode origin,
                             MockPoints points,
                             std::string cavityUUID,
                             MockAttributes attributes,
                             std::string polygonType) :
        uuid(std::move(uuid)),
        id(std::move(id)),
        materialUUID(std::move(materialUUID)),
        name(std::move(name)),
        glazingSystem(std::move(glazingSystem)),
        origin(std::move(origin)),
        points(std::move(points)),
        cavityUUID(std::move(cavityUUID)),
        attributes(std::move(attributes)),
        polygonType(std::move(polygonType))
    {}

    Helper::MockNode generatePolygonNode(const MockPolygon & polygon)
    {
        Helper::MockNode node{"Polygon"};
        addChildNode(node, "UUID", polygon.uuid);
        addChildNode(node, "ID", polygon.id);
        addChildNode(node, "MaterialUUID", polygon.materialUUID);
        addChildNode(node, "MaterialName", polygon.name);
        auto glazingSystemNode = generateGlazingSystemNode(polygon.glazingSystem);
        glazingSystemNode.tag = "GlazingSystem";
        addChildNode(node, glazingSystemNode);
        auto originNode = generatePointNode(polygon.origin);
        originNode.tag = "Origin";
        addChildNode(node, originNode);
        auto & pointsNode{addChildNode(node, "Points")};
        for(const auto & point : polygon.points)
        {
            auto pointNode = generatePointNode(point);
            addChildNode(pointsNode, pointNode);
        }
        addChildNode(node, "CavityUUID", polygon.cavityUUID);
        auto & attributesNode{addChildNode(node, "Attributes")};
        for(const auto & attribute : polygon.attributes)
        {
            addChildNode(attributesNode, "Attribute", attribute);
        }
        addChildNode(node, "Type", polygon.polygonType);

        return node;
    }

    MockSurfaceData::MockSurfaceData(std::string tilt, std::string length) :
        tilt(std::move(tilt)), length(std::move(length))
    {}

    Helper::MockNode generateSurfaceDataNode(MockSurfaceData && surface)
    {
        Helper::MockNode node{"Surface"};
        addChildNode(node, "Tilt", std::move(surface.tilt));
        addChildNode(node, "Length", std::move(surface.length));

        return node;
    }

    MockBuildingData::MockBuildingData(std::string width, std::string height) :
        width(std::move(width)), height(std::move(height))
    {}

    Helper::MockNode generateBuildingDataNode(MockBuildingData && buildingData)
    {
        Helper::MockNode node{"Building"};
        addChildNode(node, "Width", std::move(buildingData.width));
        addChildNode(node, "Height", std::move(buildingData.height));

        return node;
    }

    MockThermalEmissionProperties::MockThermalEmissionProperties(std::string emissivity,
                                                                 std::string temperature,
                                                                 std::string useGlobalEmissivity) :
        emissivity(std::move(emissivity)),
        temperature(std::move(temperature)),
        useGlobalEmissivity(std::move(useGlobalEmissivity))
    {}

    Helper::MockNode generateThermalEmissionPropertiesNode(MockThermalEmissionProperties && thermalEmissionProperties)
    {
        Helper::MockNode node{"ThermalEmissionProperties"};
        addChildNode(node, "Emissivity", std::move(thermalEmissionProperties.emissivity));
        addChildNode(node, "Temperature", std::move(thermalEmissionProperties.temperature));
        addChildNode(node, "UseGlobalEmissivity", std::move(thermalEmissionProperties.useGlobalEmissivity));

        return node;
    }

    MockShadeData::MockShadeData(std::string modifier, std::string modifierType) :
        modifier(std::move(modifier)), modifierType(std::move(modifierType))
    {}

    Helper::MockNode generateShadeDataNode(MockShadeData && shadeData)
    {
        Helper::MockNode node{"ShadeData"};
        addChildNode(node, "ShadeModifier", std::move(shadeData.modifier));
        addChildNode(node, "ShadeModifierGlazingID", std::move(shadeData.modifierType));

        return node;
    }

    MockTransientData::MockTransientData(std::string uuid,
                                         std::string timestepFileName,
                                         MockSurfaceData && surfaceData,
                                         MockBuildingData && buildingData) :
        uuid(std::move(uuid)),
        timestepFileName(std::move(timestepFileName)),
        surfaceData(std::move(surfaceData)),
        buildingData(std::move(buildingData))
    {}

    Helper::MockNode generateTransientDataNode(MockTransientData && transientData)
    {
        Helper::MockNode node{"Transient"};
        addChildNode(node, "UUID", std::move(transientData.uuid));
        addChildNode(node, "TimestepFileName", std::move(transientData.timestepFileName));
        auto surfaceNode = generateSurfaceDataNode(std::move(transientData.surfaceData));
        addChildNode(node, surfaceNode);
        auto buildingNode = generateBuildingDataNode(std::move(transientData.buildingData));
        addChildNode(node, buildingNode);

        return node;
    }

    MockBoundaryCondition::MockBoundaryCondition(std::string id,
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
                                                 std::string status) :
        id(std::move(id)),
        uuid(std::move(uuid)),
        name(std::move(name)),
        fluxTag(std::move(fluxTag)),
        isBlocking(std::move(isBlocking)),
        neighborPolygonUUID(std::move(neighborPolygonUUID)),
        origin(std::move(origin)),
        points(std::move(points)),
        side(std::move(side)),
        thermalEmissionProperties(std::move(thermalEmissionProperties)),
        shadeData(std::move(shadeData)),
        isIlluminated(std::move(isIlluminated)),
        materialSide(std::move(materialSide)),
        transientData(std::move(transientData)),
        edgeID(std::move(edgeID)),
        enclosureNumber(std::move(enclosureNumber)),
        surfaceType(std::move(surfaceType)),
        color(std::move(color)),
        status(std::move(status))
    {}

    Helper::MockNode generateBoundaryNode(MockBoundaryCondition && boundaryCondition)
    {
        Helper::MockNode node{"Boundary"};
        addChildNode(node, "ID", boundaryCondition.id);
        addChildNode(node, "UUID", boundaryCondition.uuid);
        addChildNode(node, "Name", boundaryCondition.name);
        addChildNode(node, "FluxTag", boundaryCondition.fluxTag);
        addChildNode(node, "IsBlocking", boundaryCondition.isBlocking);
        addChildNode(node, "NeighborPolygonUUID", boundaryCondition.neighborPolygonUUID);
        auto originNode = generatePointNode(boundaryCondition.origin);
        originNode.tag = "Origin";
        addChildNode(node, originNode);
        auto & pointsNode{addChildNode(node, "Points")};
        for(const auto & point : boundaryCondition.points)
        {
            auto pointNode = generatePointNode(point);
            addChildNode(pointsNode, pointNode);
        }
        addChildNode(node, "Side", boundaryCondition.side);
        auto thermalEmissionPropertiesNode =
          generateThermalEmissionPropertiesNode(std::move(boundaryCondition.thermalEmissionProperties));
        thermalEmissionPropertiesNode.tag = "ThermalEmissionProperties";
        addChildNode(node, thermalEmissionPropertiesNode);
        auto shadeDataNode = generateShadeDataNode(std::move(boundaryCondition.shadeData));
        shadeDataNode.tag = "ShadeData";
        addChildNode(node, shadeDataNode);
        addChildNode(node, "IsIlluminated", boundaryCondition.isIlluminated);
        if(!boundaryCondition.materialSide.empty())   // emulates optional
        {
            addChildNode(node, "MaterialSide", boundaryCondition.materialSide);
        }
        auto transientDataNode = generateTransientDataNode(std::move(boundaryCondition.transientData));
        transientDataNode.tag = "TransientData";
        addChildNode(node, transientDataNode);
        addChildNode(node, "EdgeID", boundaryCondition.edgeID);
        if(!boundaryCondition.enclosureNumber.empty())   // emulates optional
        {
            addChildNode(node, "EnclosureNumber", boundaryCondition.enclosureNumber);
        }
        addChildNode(node, "Type", boundaryCondition.surfaceType);
        addChildNode(node, "Color", boundaryCondition.color);
        addChildNode(node, "Status", boundaryCondition.status);

        return node;
    }
}   // namespace Helper
