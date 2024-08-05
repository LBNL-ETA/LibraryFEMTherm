#pragma once

#include <unordered_map>
#include <string>

#include "Enumerators.hxx"

namespace ThermFile
{
    using CMABCContainer = std::unordered_map<CMABCPosition, std::string>;

    struct CMAData
    {
        CMABCContainer cmaBCPositions;
        CMAIGUType iguType{CMAIGUType::Unknown};
    };
}