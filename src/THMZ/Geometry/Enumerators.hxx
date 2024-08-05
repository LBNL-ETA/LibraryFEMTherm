#pragma once

#include <string>

namespace ThermFile
{
    //! \brief Enumerates different directions of heat flow or glazing cavity orientation.
    enum class Direction
    {
        Unknown,
        Right,
        Left,
        Up,
        Down
    };

    //! \brief Converts a string to a direction.
    //! \param direction The string to convert.
    //! \return The direction represented by the string.
    Direction directionFromString(std::string_view direction);

    //! \brief Converts a direction to a string.
    //! \param direction The direction to convert.
    //! \return The string representation of the direction.
    std::string directionToString(Direction direction);

    //! \brief Enumerates belonging of the surface to certain polygon.
    enum class SurfaceType
    {
        Unknown,
        NotASurface,
        BoundaryCondition,
        GlazingCavity,
        FrameCavity,
        Enclosure
    };

    //! \brief Converts a string to a surface type.
    //! \param surfaceType The string to convert.
    //! \return The surface type represented by the string.
    SurfaceType surfaceTypeFromString(std::string_view surfaceType);

    //! \brief Converts a surface type to a string.
    //! \param surfaceType The surface type to convert.
    //! \return The string representation of the surface type.
    std::string surfaceTypeToString(SurfaceType surfaceType);

    enum class ShadeModifier
    {
        None,
        Exterior,
        Interior
    };

    //! \brief Converts a string to a shade modifier.
    //! \param shadeModifier The string to convert.
    //! \return The shade modifier represented by the string.
    ShadeModifier shadeModifierFromString(std::string_view shadeModifier);

    //! \brief Converts a shade modifier to a string.
    //! \param shadeModifier The shade modifier to convert.
    //! \return The string representation of the shade modifier.
    std::string shadeModifierToString(ShadeModifier shadeModifier);

    enum class PolygonType
    {
        Unknown,
        None,
        Material,
        Glass,
        Gap,
        BoundaryCondition,
        Spacer,
        GlassIn,
        GlassOut,
        ShadeGap
    };

    //! \brief Converts a string to a polygon type.
    //! \param polygonType The string to convert.
    //! \return The polygon type represented by the string.
    PolygonType polygonTypeFromString(std::string_view polygonType);

    //! \brief Converts a polygon type to a string.
    //! \param polygonType The polygon type to convert.
    //! \return The string representation of the polygon type.
    std::string polygonTypeToString(PolygonType polygonType);

    //! \brief Enumerates the side of the material that will be used at the boundary condition. This is valid only for
    //! the radiance mode.
    enum class MaterialSide
    {
        Unknown,
        Front,
        Back
    };

    //! \brief Converts a string to a material side.
    //! \param materialSide The string to convert.
    //! \return The material side represented by the string.
    MaterialSide materialSideFromString(std::string_view materialSide);

    //! \brief Converts a material side to a string.
    //! \param materialSide The material side to convert.
    //! \return The string representation of the material side.
    std::string materialSideToString(MaterialSide materialSide);

}   // namespace ThermFile