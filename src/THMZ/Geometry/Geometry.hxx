#pragma once

#include <string>
#include <vector>
#include <optional>

#include "Enumerators.hxx"

namespace ThermFile
{
    //! \brief A point in 2D space. Used in various structures withing THERM model.
    struct Point
    {
        double x{0};
        double y{0};
    };

    // TODO: Consider splitting this structure into FrameCavity and GlazingCavity
    //! \brief Represents cavity model in THERM. Refer to ISO 15099 for more details on the procedure.
    //! @heatFlowDirection - direction of heat flow.
    //! @emissivity1 - warm side emissivity.
    //! @emissivity2 - cold side emissivity.
    //! @temperature1 - warm side temperature.
    //! @temperature2 - cold side temperature.
    //! @maxXDimension - maximum x dimension of the cavity.
    //! @maxYDimension - maximum y dimension of the cavity.
    //! @actualHeight - actual height of the cavity.
    //! @area - area of the cavity.
    //! @localEmissivities - flag indicating if THERM should be using emissivities presented here rather than picking
    //! them from the material.
    struct Cavity
    {
        std::string uuid;
        std::optional<Direction> direction;
        Direction heatFlowDirection;
        double emissivity1{0};
        double emissivity2{0};
        double temperature1{0};
        double temperature2{0};
        double maxXDimension{0};
        double maxYDimension{0};
        double actualHeight{0};
        double area{0};
        bool localEmissivities{false};
        double pressure{0};
        Point warmLocator{0, 0};
        Point coldLocator{0, 0};
    };

    //! \brief Represents glazing system in THERM model.
    //! @ID - unique identifier of the glazing system.
    //! @index - index of the glazing system.
    struct GlazingSystemData
    {
        size_t ID{0u};
        size_t index{0u};
    };

    //! \brief Represents a polygon in THERM model.
    //! @uuid - unique identifier of the polygon.
    //! @materialName - name of the material used in the polygon.
    //! @materialUUID - unique identifier of the material used in the polygon.
    //! @glazingSystemID - unique identifier of the glazing system used in the polygon.
    //! @origin - origin of the polygon.
    //! @points - points that is polygon made of.
    //! @cavityUUID - unique identifier of the cavity if the polygon represents the cavity.
    //! @attributes - attributes of the polygon.
    //! @polygonType - type of the polygon.
    //! @linkID - used to link material type to another polyshape
    struct Polygon
    {
        std::string uuid;
        int ID{-1};
        std::string materialUUID;
        std::string materialName;
        std::optional<GlazingSystemData> glazingSystem;
        Point origin;
        std::vector<Point> points;
        std::optional<std::string> cavityUUID;
        std::vector<std::string> attributes;
        PolygonType polygonType{PolygonType::None};
        std::optional<int> linkID;
    };

    //! \brief Represents geometry data that are used in certain boundary condition types
    //! @tilt - tilt of the boundary condition surface. [deg]
    //! @length - length of the boundary condition surface. [mm]
    struct SurfaceData
    {
        double tilt{90.0};
        double length{1000};
    };

    //! \brief Represents geometry specification that are used in certain boundary condition types.
    //! @width - width of the boundary condition surface. [m]
    //! @height - height of the boundary condition surface. [m]
    struct BuildingData
    {
        double width{20};
        double height{5};
    };

    //! \brief Represents thermal emission properties that are used in certain boundary condition types.
    //! @emissivity - emissivity of the boundary condition surface.
    //! @temperature - temperature of the boundary condition surface.
    //! @useGlobalEmissivity - flag indicating if THERM should be using emissivity from the material.
    struct ThermalEmissionProperties
    {
        double emissivity{0};
        double temperature{0};
        bool useGlobalEmissivity{false};
    };

    //! \brief Represents shade modifier that are used in certain boundary condition types.
    //! @shadeModifier - shade modifier of the boundary condition surface.
    //! @shadeModifierGlazingID - glazing ID of the boundary condition surface.
    struct ShadeData
    {
        ShadeModifier shadeModifier{ShadeModifier::None};
        int shadeModifierGlazingID{0};
    };

    //! \brief Represents transient data that are used in certain boundary condition types.
    //! @typeRecordUUID - type record UUID of the boundary condition surface.
    //! @transientFileName - transient file name of the boundary condition surface.
    struct TransientData
    {
        std::string typeRecordUUID;
        std::string transientFileName;
        std::optional<SurfaceData> surfaceData;
        std::optional<BuildingData> buildingData;
    };

    //! \brief Represents boundary condition in THERM model.
    //! @uuid - unique identifier of the boundary condition.
    //! @name - name of the boundary condition.
    //! @fluxTag - flux tag of the boundary condition.
    //! @isBlocking - flag indicating if the boundary condition is blocking.
    //! @neighborPolygonUUID - unique identifier of the neighbor polygon.
    //! @origin - origin of the boundary condition.
    //! @points - points that is boundary condition made of.
    //! @Side - side of the boundary condition.
    //! @thermalEmissionProperties - thermal emission properties of the boundary condition.
    //! @ID - unique identifier of the boundary condition.
    //! @shadeData - shade data of the boundary condition.
    //! @isIlluminated - flag indicating if the boundary condition is illuminated.
    //! @typeRecordUUID - type record UUID of the boundary condition.
    //! @geometrySpecification - geometry specification of the boundary condition.
    //! @EdgeID - unique identifier of the edge.
    struct Boundary
    {
        size_t ID{0u};
        std::string uuid;
        std::string name;
        std::string fluxTag;
        bool isBlocking{false};
        std::string neighborPolygonUUID;
        Point origin;
        Point startPoint;
        Point endPoint;
        size_t Side{0u};
        ThermalEmissionProperties thermalEmissionProperties;
        std::optional<ShadeData> shadeData;
        std::optional<bool> isIlluminated{false};
        std::optional<MaterialSide> materialSide;
        std::optional<TransientData> transientRecordData;
        size_t EdgeID{0u};
        std::optional<size_t> enclosureNumber;
        SurfaceType surfaceType{SurfaceType::Unknown};
        std::string color{"Ox000000"};
        int status;
    };
}   // namespace ThermFile
