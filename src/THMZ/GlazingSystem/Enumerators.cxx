#include "Enumerators.hxx"

#include <map>

#include "LibraryUtilities/Common.hxx"

namespace ThermFile
{
    const std::map<EnvironmentalCondition, std::string> envCondToStringMap = {
      {EnvironmentalCondition::Winter, "Winter"}, {EnvironmentalCondition::Summer, "Summer"}};

    EnvironmentalCondition environmentalConditionsFromString(std::string_view environmentalConditions)
    {
        return LibraryCommon::enumFromString(environmentalConditions.data(), envCondToStringMap);
    }

    std::string environmentalConditionsToString(EnvironmentalCondition environmentalConditions)
    {
        return LibraryCommon::enumToString(environmentalConditions, envCondToStringMap);
    }

    const std::map<Position, std::string> positionToStringMap = {{Position::Interior, "Interior"},
                                                                 {Position::Exterior, "Exterior"}};

    Position positionFromString(std::string_view position)
    {
        return LibraryCommon::enumFromString(position, positionToStringMap);
    }

    std::string positionToString(Position position)
    {
        return LibraryCommon::enumToString(position, positionToStringMap);
    }

    const std::map<BoundaryConditionStyle, std::string> boundaryConditionStyleToStringMap = {
      {BoundaryConditionStyle::Combined, "Combined"},
      {BoundaryConditionStyle::Split, "Split"},
      {BoundaryConditionStyle::BlackBody, "BlackBody"},
      {BoundaryConditionStyle::Enclosure, "Enclosure"},
      {BoundaryConditionStyle::Existing, "Existing"}};

    std::string boundaryConditionStyleToString(BoundaryConditionStyle boundaryConditionStyle)
    {
        return LibraryCommon::enumToString(boundaryConditionStyle, boundaryConditionStyleToStringMap);
    }

    BoundaryConditionStyle boundaryConditionStyleFromString(std::string_view boundaryConditionStyle)
    {
        return LibraryCommon::enumFromString(boundaryConditionStyle.data(), boundaryConditionStyleToStringMap);
    }
}   // namespace ThermFile
