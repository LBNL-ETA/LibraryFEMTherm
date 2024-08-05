#pragma once

#include <fileParse/Enum.hxx>

#include "Enumerators.hxx"

namespace ThermFile
{
    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::Direction & direction)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::Direction>(
          node, direction, ThermFile::directionFromString);
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::Direction & direction)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::Direction>(
          node, direction, ThermFile::directionToString);
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::SurfaceType & surfaceType)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::SurfaceType>(
          node, surfaceType, ThermFile::surfaceTypeFromString);
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::SurfaceType & surfaceType)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::SurfaceType>(
          node, surfaceType, ThermFile::surfaceTypeToString);
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::ShadeModifier & shadeModifier)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::ShadeModifier>(
          node, shadeModifier, ThermFile::shadeModifierFromString);
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::ShadeModifier & shadeModifier)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::ShadeModifier>(
          node, shadeModifier, ThermFile::shadeModifierToString);
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::PolygonType & polygonType)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::PolygonType>(
          node, polygonType, ThermFile::polygonTypeFromString);
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::PolygonType & polygonType)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::PolygonType>(
          node, polygonType, ThermFile::polygonTypeToString);
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::MaterialSide & materialSide)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::MaterialSide>(
          node, materialSide, ThermFile::materialSideFromString);
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::MaterialSide & materialSide)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::MaterialSide>(
          node, materialSide, ThermFile::materialSideToString);
    }
}   // namespace ThermFile