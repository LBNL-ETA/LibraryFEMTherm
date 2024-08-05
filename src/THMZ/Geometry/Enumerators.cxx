#include "Enumerators.hxx"

#include <map>

#include "LibraryUtilities/Common.hxx"

namespace ThermFile
{
    std::map<Direction, std::string> heatFlowDirectionStringMap = {
      {Direction::Left, "Left"}, {Direction::Right, "Right"}, {Direction::Up, "Up"}, {Direction::Down, "Down"}};

    Direction directionFromString(std::string_view direction)
    {
        return LibraryCommon::enumFromString(direction, heatFlowDirectionStringMap);
    }

    std::string directionToString(Direction direction)
    {
        return LibraryCommon::enumToString(direction, heatFlowDirectionStringMap);
    }

    std::map<SurfaceType, std::string> surfaceTypeStringMap = {{SurfaceType::NotASurface, "Not A Surface"},
                                                               {SurfaceType::BoundaryCondition, "Boundary Condition"},
                                                               {SurfaceType::GlazingCavity, "Glazing Cavity"},
                                                               {SurfaceType::FrameCavity, "Frame Cavity"},
                                                               {SurfaceType::Enclosure, "Enclosure"}};

    SurfaceType surfaceTypeFromString(std::string_view surfaceType)
    {
        return LibraryCommon::enumFromString(surfaceType, surfaceTypeStringMap);
    }

    std::string surfaceTypeToString(SurfaceType surfaceType)
    {
        return LibraryCommon::enumToString(surfaceType, surfaceTypeStringMap);
    }

    std::map<ShadeModifier, std::string> shadeModifierStringMap = {
      {ShadeModifier::None, "None"}, {ShadeModifier::Exterior, "Exterior"}, {ShadeModifier::Interior, "Interior"}};

    ShadeModifier shadeModifierFromString(std::string_view shadeModifier)
    {
        return LibraryCommon::enumFromString(shadeModifier, shadeModifierStringMap);
    }

    std::string shadeModifierToString(ShadeModifier shadeModifier)
    {
        return LibraryCommon::enumToString(shadeModifier, shadeModifierStringMap);
    }

    std::map<PolygonType, std::string> polygonTypeStringMap = {{PolygonType::None, "None"},
                                                               {PolygonType::Material, "Material"},
                                                               {PolygonType::Glass, "Glass"},
                                                               {PolygonType::Gap, "Gap"},
                                                               {PolygonType::BoundaryCondition, "Boundary Condition"},
                                                               {PolygonType::Spacer, "Spacer"},
                                                               {PolygonType::GlassIn, "Glass In"},
                                                               {PolygonType::GlassOut, "Glass Out"},
                                                               {PolygonType::ShadeGap, "Shade Gap"}};

    PolygonType polygonTypeFromString(std::string_view polygonType)
    {
        return LibraryCommon::enumFromString(polygonType, polygonTypeStringMap);
    }

    std::string polygonTypeToString(PolygonType polygonType)
    {
        return LibraryCommon::enumToString(polygonType, polygonTypeStringMap);
    }

    std::map<MaterialSide, std::string> materialSideStringMap = {{MaterialSide::Front, "Front"},
                                                                 {MaterialSide::Back, "Back"}};

    MaterialSide materialSideFromString(std::string_view materialSide)
    {
        return LibraryCommon::enumFromString(materialSide, materialSideStringMap);
    }

    std::string materialSideToString(MaterialSide materialSide)
    {
        return LibraryCommon::enumToString(materialSide, materialSideStringMap);
    }
}   // namespace ThermFile
