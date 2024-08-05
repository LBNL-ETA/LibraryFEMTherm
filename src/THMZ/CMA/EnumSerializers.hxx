#pragma once

#include <fileParse/Enum.hxx>

#include "Enumerators.hxx"

namespace ThermFile
{
    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::CMABCPosition & cmabcPosition)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::CMABCPosition>(
          node, cmabcPosition, ThermFile::cmaBCPositionFromString);
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::CMABCPosition & cmabcPosition)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::CMABCPosition>(
          node, cmabcPosition, ThermFile::cmaBCPositionToString);
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::CMAIGUType & iguType)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::CMAIGUType>(
          node, iguType, ThermFile::iguTypeFromString);
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::CMAIGUType & iguType)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::CMAIGUType>(node, iguType, ThermFile::iguTypeToString);
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::CMACase & cmaCase)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::CMACase>(node, cmaCase, ThermFile::cmaCaseFromString);
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::CMACase & cmaCase)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::CMACase>(node, cmaCase, ThermFile::cmaCaseToString);
    }
}   // namespace ThermFile