#include "Enumerators.hxx"

#include <map>

#include "LibraryUtilities/Common.hxx"

namespace ThermFile
{
    const std::map<RunType, std::string> runTypeToStringMap = {
      {RunType::Unknown, "Unknown"},
      {RunType::UFactor, "U-factor"},
      {RunType::CondensationResistance, "Condensation Resistance"}};

    RunType runTypeFromString(std::string_view runType)
    {
        return LibraryCommon::enumFromString(runType, runTypeToStringMap);
    }

    std::string runTypeToString(RunType runType)
    {
        return LibraryCommon::enumToString(runType, runTypeToStringMap);
    }

    const std::map<SegmentType, std::string> segmentTypeToStringMap = {
      {SegmentType::Unknown, "Unknown"},
      {SegmentType::Flux, "Flux"},
      {SegmentType::Temperature, "Temperature"},
      {SegmentType::Convection, "Convection"},
      {SegmentType::Radiation, "Radiation"},
      {SegmentType::RadiationEnclosure, "Radiation Enclosure"}};


    SegmentType segmentTypeFromString(std::string_view segmentType)
    {
        return LibraryCommon::enumFromString(segmentType, segmentTypeToStringMap);
    }

    std::string segmentTypeToString(SegmentType segmentType)
    {
        return LibraryCommon::enumToString(segmentType, segmentTypeToStringMap);
    }
}   // namespace ThermFile
