#pragma once

namespace ThermFile
{
    enum class CMABCPosition
    {
        None,
        Interior,
        Exterior
    };

    CMABCPosition cmaBCPositionFromString(std::string_view cmaPosition);
    std::string cmaBCPositionToString(CMABCPosition cmaPosition);

    enum class CMAIGUType
    {
        Unknown,
        SingleLayer,
        DoubleLayer
    };

    CMAIGUType iguTypeFromString(std::string_view iguType);
    std::string iguTypeToString(CMAIGUType iguType);

    enum class CMACase
    {
        None,
        Low,
        High
    };

    CMACase cmaCaseFromString(std::string_view cmaCase);
    std::string cmaCaseToString(CMACase cmaCase);
}   // namespace ThermFile