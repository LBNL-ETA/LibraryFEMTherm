#pragma once

#include <fileParse/Enum.hxx>

#include "Enumerators.hxx"

namespace ThermFile
{
    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::UnitSystem & unitSystem)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::UnitSystem>(
          node, unitSystem, ThermFile::unitSystemFromString);
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::UnitSystem & unitSystem)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::UnitSystem>(
          node, unitSystem, ThermFile::unitSystemToString);
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::ConductivityUnits & conductivityUnit)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::ConductivityUnits>(
          node, conductivityUnit, ThermFile::conductivityUnitsFromString);
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::ConductivityUnits & conductivityUnit)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::ConductivityUnits>(
          node, conductivityUnit, ThermFile::conductivityUnitsToString);
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::UValueBasisType & uValueBasisType)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::UValueBasisType>(
          node, uValueBasisType, ThermFile::uValueBasisTypeFromString);
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::UValueBasisType & uValueBasisType)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::UValueBasisType>(
          node, uValueBasisType, ThermFile::uValueBasisTypeToString);
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::UnderlayType & underlayType)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::UnderlayType>(
          node, underlayType, ThermFile::underlayTypeFromString);
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::UnderlayType & underlayType)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::UnderlayType>(
          node, underlayType, ThermFile::underlayTypeToString);
    }
}   // namespace ThermFile