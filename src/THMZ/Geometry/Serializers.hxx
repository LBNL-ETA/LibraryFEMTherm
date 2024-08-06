#pragma once

#include <fileParse/Vector.hxx>
#include <fileParse/Optional.hxx>

#include "Geometry.hxx"
#include "EnumSerializers.hxx"

namespace ThermFile
{
    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, Point & point)
    {
        node >> FileParse::Child{"x", point.x};
        node >> FileParse::Child{"y", point.y};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const Point & point)
    {
        node << FileParse::Child{"x", point.x};
        node << FileParse::Child{"y", point.y};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, Cavity & cavity)
    {
        node >> FileParse::Child{"UUID", cavity.uuid};
        node >> FileParse::Child{"Direction", cavity.direction};
        node >> FileParse::Child{"HeatFlowDirection", cavity.heatFlowDirection};
        node >> FileParse::Child{"Emissivity1", cavity.emissivity1};
        node >> FileParse::Child{"Emissivity2", cavity.emissivity2};
        node >> FileParse::Child{"Temperature1", cavity.temperature1};
        node >> FileParse::Child{"Temperature2", cavity.temperature2};
        node >> FileParse::Child{"MaxXDimension", cavity.maxXDimension};
        node >> FileParse::Child{"MaxYDimension", cavity.maxYDimension};
        node >> FileParse::Child{"ActualHeight", cavity.actualHeight};
        node >> FileParse::Child{"Area", cavity.area};
        node >> FileParse::Child{"LocalEmissivities", cavity.localEmissivities};
        node >> FileParse::Child{"Pressure", cavity.pressure};
        node >> FileParse::Child{"WarmLocator", cavity.warmLocator};
        node >> FileParse::Child{"ColdLocator", cavity.coldLocator};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const Cavity & cavity)
    {
        node << FileParse::Child{"UUID", cavity.uuid};
        node << FileParse::Child{"Direction", cavity.direction};
        node << FileParse::Child{"HeatFlowDirection", cavity.heatFlowDirection};
        node << FileParse::Child{"Emissivity1", cavity.emissivity1};
        node << FileParse::Child{"Emissivity2", cavity.emissivity2};
        node << FileParse::Child{"Temperature1", cavity.temperature1};
        node << FileParse::Child{"Temperature2", cavity.temperature2};
        node << FileParse::Child{"MaxXDimension", cavity.maxXDimension};
        node << FileParse::Child{"MaxYDimension", cavity.maxYDimension};
        node << FileParse::Child{"ActualHeight", cavity.actualHeight};
        node << FileParse::Child{"Area", cavity.area};
        node << FileParse::Child{"LocalEmissivities", cavity.localEmissivities};
        node << FileParse::Child{"Pressure", cavity.pressure};
        node << FileParse::Child{"WarmLocator", cavity.warmLocator};
        node << FileParse::Child{"ColdLocator", cavity.coldLocator};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, GlazingSystemData & glazingSystem)
    {
        node >> FileParse::Child{"ID", glazingSystem.ID};
        node >> FileParse::Child{"Index", glazingSystem.index};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const GlazingSystemData & glazingSystem)
    {
        node << FileParse::Child{"ID", glazingSystem.ID};
        node << FileParse::Child{"Index", glazingSystem.index};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, Polygon & polygon)
    {
        node >> FileParse::Child{"UUID", polygon.uuid};
        node >> FileParse::Child{"ID", polygon.ID};
        node >> FileParse::Child{"MaterialUUID", polygon.materialUUID};
        node >> FileParse::Child{"MaterialName", polygon.materialName};
        node >> FileParse::Child{"GlazingSystem", polygon.glazingSystem};
        node >> FileParse::Child{"Origin", polygon.origin};
        node >> FileParse::Child{{"Points", "Point"}, polygon.points};
        node >> FileParse::Child{"CavityUUID", polygon.cavityUUID};
        node >> FileParse::Child{{"Attributes", "Attribute"}, polygon.attributes};
        node >> FileParse::Child{"Type", polygon.polygonType};
        node >> FileParse::Child{"LinkID", polygon.linkID};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const Polygon & polygon)
    {
        node << FileParse::Child{"UUID", polygon.uuid};
        node << FileParse::Child{"ID", polygon.ID};
        node << FileParse::Child{"MaterialUUID", polygon.materialUUID};
        node << FileParse::Child{"MaterialName", polygon.materialName};
        node << FileParse::Child{"GlazingSystem", polygon.glazingSystem};
        node << FileParse::Child{"Origin", polygon.origin};
        node << FileParse::Child{{"Points", "Point"}, polygon.points};
        node << FileParse::Child{"CavityUUID", polygon.cavityUUID};
        node << FileParse::Child{{"Attributes", "Attribute"}, polygon.attributes};
        node << FileParse::Child{"Type", polygon.polygonType};
        node << FileParse::Child{"LinkID", polygon.linkID};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, SurfaceData & surface)
    {
        node >> FileParse::Child{"Tilt", surface.tilt};
        node >> FileParse::Child{"Length", surface.length};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const SurfaceData & surface)
    {
        node << FileParse::Child{"Tilt", surface.tilt};
        node << FileParse::Child{"Length", surface.length};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BuildingData & building)
    {
        node >> FileParse::Child{"Width", building.width};
        node >> FileParse::Child{"Height", building.height};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BuildingData & building)
    {
        node << FileParse::Child{"Width", building.width};
        node << FileParse::Child{"Height", building.height};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermalEmissionProperties & thermalEmissionProperties)
    {
        node >> FileParse::Child{"Emissivity", thermalEmissionProperties.emissivity};
        node >> FileParse::Child{"Temperature", thermalEmissionProperties.temperature};
        node >> FileParse::Child{"UseGlobalEmissivity", thermalEmissionProperties.useGlobalEmissivity};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermalEmissionProperties & thermalEmissionProperties)
    {
        node << FileParse::Child{"Emissivity", thermalEmissionProperties.emissivity};
        node << FileParse::Child{"Temperature", thermalEmissionProperties.temperature};
        node << FileParse::Child{"UseGlobalEmissivity", thermalEmissionProperties.useGlobalEmissivity};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ShadeData & shadeData)
    {
        node >> FileParse::Child{"ShadeModifier", shadeData.shadeModifier};
        node >> FileParse::Child{"ShadeModifierGlazingID", shadeData.shadeModifierGlazingID};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ShadeData & shadeData)
    {
        node << FileParse::Child{"ShadeModifier", shadeData.shadeModifier};
        node << FileParse::Child{"ShadeModifierGlazingID", shadeData.shadeModifierGlazingID};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, TransientData & transientData)
    {
        node >> FileParse::Child{"UUID", transientData.typeRecordUUID};
        node >> FileParse::Child{"TimestepFileName", transientData.transientFileName};
        node >> FileParse::Child{"Surface", transientData.surfaceData};
        node >> FileParse::Child{"Building", transientData.buildingData};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const TransientData & transientData)
    {
        node << FileParse::Child{"UUID", transientData.typeRecordUUID};
        node << FileParse::Child{"TimestepFileName", transientData.transientFileName};
        node << FileParse::Child{"Surface", transientData.surfaceData};
        node << FileParse::Child{"Building", transientData.buildingData};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, Boundary & boundaryCondition)
    {
        node >> FileParse::Child{"ID", boundaryCondition.ID};
        node >> FileParse::Child{"UUID", boundaryCondition.uuid};
        node >> FileParse::Child{"Name", boundaryCondition.name};
        node >> FileParse::Child{"FluxTag", boundaryCondition.fluxTag};
        node >> FileParse::Child{"IsBlocking", boundaryCondition.isBlocking};
        node >> FileParse::Child{"NeighborPolygonUUID", boundaryCondition.neighborPolygonUUID};
        node >> FileParse::Child{"Origin", boundaryCondition.origin};
        node >> FileParse::Child{{"Points", "Point"}, boundaryCondition.points};
        node >> FileParse::Child{"Side", boundaryCondition.Side};
        node >> FileParse::Child{"ThermalEmissionProperties", boundaryCondition.thermalEmissionProperties};
        node >> FileParse::Child{"ShadeData", boundaryCondition.shadeData};
        node >> FileParse::Child{"IsIlluminated", boundaryCondition.isIlluminated};
        node >> FileParse::Child{"MaterialSide", boundaryCondition.materialSide};
        node >> FileParse::Child{"TransientData", boundaryCondition.transientRecordData};
        node >> FileParse::Child{"EdgeID", boundaryCondition.EdgeID};
        node >> FileParse::Child{"EnclosureNumber", boundaryCondition.enclosureNumber};
        node >> FileParse::Child{"Type", boundaryCondition.surfaceType};
        node >> FileParse::Child{"Color", boundaryCondition.color};
        node >> FileParse::Child{"Status", boundaryCondition.status};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const Boundary & boundaryCondition)
    {
        node << FileParse::Child{"ID", boundaryCondition.ID};
        node << FileParse::Child{"UUID", boundaryCondition.uuid};
        node << FileParse::Child{"Name", boundaryCondition.name};
        node << FileParse::Child{"FluxTag", boundaryCondition.fluxTag};
        node << FileParse::Child{"IsBlocking", boundaryCondition.isBlocking};
        node << FileParse::Child{"NeighborPolygonUUID", boundaryCondition.neighborPolygonUUID};
        node << FileParse::Child{"Origin", boundaryCondition.origin};
        node << FileParse::Child{{"Points", "Point"}, boundaryCondition.points};
        node << FileParse::Child{"Side", boundaryCondition.Side};
        node << FileParse::Child{"ThermalEmissionProperties", boundaryCondition.thermalEmissionProperties};
        node << FileParse::Child{"ShadeData", boundaryCondition.shadeData};
        node << FileParse::Child{"IsIlluminated", boundaryCondition.isIlluminated};
        node << FileParse::Child{"MaterialSide", boundaryCondition.materialSide};
        node << FileParse::Child{"TransientData", boundaryCondition.transientRecordData};
        node << FileParse::Child{"EdgeID", boundaryCondition.EdgeID};
        node << FileParse::Child{"EnclosureNumber", boundaryCondition.enclosureNumber};
        node << FileParse::Child{"Type", boundaryCondition.surfaceType};
        node << FileParse::Child{"Color", boundaryCondition.color};
        node << FileParse::Child{"Status", boundaryCondition.status};

        return node;
    }

}   // namespace ThermFile