#pragma once

#include <string>

namespace ThermFile
{
    enum class RunType
    {
        Unknown,
        UFactor,
        CondensationResistance
    };

    RunType runTypeFromString(std::string_view runType);

    std::string runTypeToString(RunType runType);

    enum class SegmentType
    {
        Unknown,
        Flux,
        Temperature,
        Convection,
        Radiation,
        RadiationEnclosure
    };

    SegmentType segmentTypeFromString(std::string_view segmentType);

    std::string segmentTypeToString(SegmentType segmentType);
}   // namespace ThermFile