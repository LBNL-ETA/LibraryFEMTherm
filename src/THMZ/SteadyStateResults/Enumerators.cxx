#include "Enumerators.hxx"

#include <map>

#include "LibraryUtilities/Common.hxx"

namespace ThermFile
{
    const std::map<UValueDimensionType, std::string> uValueDimensionTypeToStringMap = {
      {UValueDimensionType::Unknown, "Unknown"},
      {UValueDimensionType::TotalLength, "Total Length"},
      {UValueDimensionType::XProjected, "Projected X"},
      {UValueDimensionType::YProjected, "Projected Y"},
      {UValueDimensionType::Custom, "Custom Length"},
      {UValueDimensionType::GlassRotationProjected, "Projected in glass plane"},
      {UValueDimensionType::CustomRotationProjected, "Custom Rotation"}};


    UValueDimensionType uValueDimensionTypeFromString(std::string_view uValueDimensionType)
    {
        return LibraryCommon::enumFromString(uValueDimensionType, uValueDimensionTypeToStringMap);
    }

    std::string uValueDimensionTypeToString(UValueDimensionType uValueDimensionType)
    {
        return LibraryCommon::enumToString(uValueDimensionType, uValueDimensionTypeToStringMap);
    }
}