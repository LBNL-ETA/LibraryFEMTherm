#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <variant>

#include "Enumerators.hxx"

namespace ThermFile
{
    using CMABCContainer = std::unordered_map<CMABCPosition, std::string>;

    struct CMAGapSpacer
    {
        CMACase glazingCase{CMACase::None};
        CMACase spacerCase{CMACase::None};

        double gapConductance{0.0};
        double spacerConductivity{0.0};
    };

    struct CMALayerInput
    {
        CMACase glazingCase{CMACase::None};
        CMACase spacerCase{CMACase::None};
        double hcin{0.0};
        double hcout{0.0};
        double emissivityIn{0.0};
        double emissivityOut{0.0};
    };

    struct CMASingleLayer
    {
        double thickness{0.0};
        double conductivity{0.0};
        std::vector<CMALayerInput> option;
    };

    struct CMADoubleLayer
    {
        double thicknessIn{0.0};
        double thicknessOut{0.0};
        double conductivityIn{0.0};
        double conductivityOut{0.0};
        std::vector<CMALayerInput> option;
    };

    struct CMAData
    {
        CMABCContainer cmaBCPositions;
        CMAIGUType iguType{CMAIGUType::Unknown};
        std::variant<CMASingleLayer, CMADoubleLayer> layerInput;
        std::vector<CMAGapSpacer> gapSpacerInput;
    };
}