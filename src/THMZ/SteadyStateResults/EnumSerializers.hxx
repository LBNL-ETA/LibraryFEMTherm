#pragma once

#include <fileParse/Enum.hxx>

#include "Enumerators.hxx"

namespace ThermFile
{
    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::UValueDimensionType & uValueDimensionType)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::UValueDimensionType>(
          node, uValueDimensionType, ThermFile::uValueDimensionTypeFromString);
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::UValueDimensionType & uValueDimensionType)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::UValueDimensionType>(
          node, uValueDimensionType, ThermFile::uValueDimensionTypeToString);
    }
}   // namespace ThermFile