#pragma once

#include <fileParse/Enum.hxx>

#include "Enumerators.hxx"

namespace ThermFile
{
    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node,
                                   ThermFile::EnvironmentalCondition & environmentalConditions)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::EnvironmentalCondition>(
          node, environmentalConditions, ThermFile::environmentalConditionsFromString);
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::EnvironmentalCondition & environmentalConditions)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::EnvironmentalCondition>(
          node, environmentalConditions, ThermFile::environmentalConditionsToString);
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::Position & position)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::Position>(
          node, position, ThermFile::positionFromString);
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::Position & position)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::Position>(node, position, ThermFile::positionToString);
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::BoundaryConditionStyle & boundaryConditionStyle)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::BoundaryConditionStyle>(
          node, boundaryConditionStyle, ThermFile::boundaryConditionStyleFromString);
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::BoundaryConditionStyle & boundaryConditionStyle)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::BoundaryConditionStyle>(
          node, boundaryConditionStyle, ThermFile::boundaryConditionStyleToString);
    }
}   // namespace ThermFile
