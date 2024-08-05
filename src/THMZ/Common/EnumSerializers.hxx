#pragma once

#include <fileParse/Enum.hxx>

#include "Enumerators.hxx"

namespace ThermFile
{
    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::RunType & runType)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::RunType>(node, runType, ThermFile::runTypeFromString);
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::RunType & runType)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::RunType>(node, runType, ThermFile::runTypeToString);
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::SegmentType & segmentType)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::SegmentType>(
          node, segmentType, ThermFile::segmentTypeFromString);
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::SegmentType & segmentType)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::SegmentType>(
          node, segmentType, ThermFile::segmentTypeToString);
    }
}   // namespace ThermFile