#include <array>
#include <string>

#include "Enumerators.hxx"

#include "LibraryUtilities/Common.hxx"

namespace ThermFile
{
    const std::array<std::string, 3> CMABCPositionStrings = {"None", "Interior", "Exterior"};

    CMABCPosition cmaBCPositionFromString(std::string_view cmaPosition)
    {
        return LibraryCommon::enumFromString(cmaPosition, CMABCPositionStrings, CMABCPosition::None);
    }

    std::string cmaBCPositionToString(CMABCPosition cmaPosition)
    {
        return LibraryCommon::enumToString(cmaPosition, CMABCPositionStrings);
    }

    const std::array<std::string, 3> IGUTypeStrings = {"Unknown", "SingleLayer", "DoubleLayer"};

    CMAIGUType iguTypeFromString(std::string_view iguType)
    {
        return LibraryCommon::enumFromString(iguType, IGUTypeStrings, CMAIGUType::Unknown);
    }

    std::string iguTypeToString(CMAIGUType iguType)
    {
        return LibraryCommon::enumToString(iguType, IGUTypeStrings);
    }

    const std::array<std::string, 3> CMACaseStrings = {"None", "Low", "High"};

    CMACase cmaCaseFromString(std::string_view cmaCase)
    {
        return LibraryCommon::enumFromString(cmaCase, CMACaseStrings, CMACase::None);
    }

    std::string cmaCaseToString(CMACase cmaCase)
    {
        return LibraryCommon::enumToString(cmaCase, CMACaseStrings);
    }
}   // namespace ThermFile