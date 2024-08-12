#pragma once

#include <vector>
#include <unordered_map>
#include <string>

#include "Enumerators.hxx"

namespace ThermFile
{
    using CMABCContainer = std::unordered_map<CMABCPosition, std::string>;

    struct CMABestWorstValues
    {
        CMACase glazingCase{CMACase::None};
        CMACase spacerCase{CMACase::None};

        double gapConductance{0.0};
        double spacerConductivity{0.0};
    };

    struct CMAData
    {
        CMABCContainer cmaBCPositions;
        CMAIGUType iguType{CMAIGUType::Unknown};
        std::vector<CMABestWorstValues> bestWorstValues;
    };
}