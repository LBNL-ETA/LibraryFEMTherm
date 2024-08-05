#pragma once

#include <map>
#include <optional>
#include <vector>

#include "Enumerators.hxx"

namespace ThermFile
{
    struct EnvironmentCoefficients
    {
        double h{0.0};
        double hc{0.0};
        double hr{0.0};
        double temperature{0.0};
    };

    struct GlazingSystemProperties
    {
        EnvironmentalCondition environmentalCondition{EnvironmentalCondition::Unknown};
        BoundaryConditionStyle boundaryConditionStyle{BoundaryConditionStyle::Unknown};
        double uValue{0.0};
        std::map<Position, EnvironmentCoefficients> coefficients;
        double solar{0.0};
    };

    struct IGUSurface
    {
        double temperature{0.0};
        double emissivity{0.0};
    };

    struct IGULayer
    {
        size_t ID{0u};
        std::string name;
        double thickness{0.0};
        double conductivity{0.0};
        IGUSurface surface1;
        IGUSurface surface2;
    };

    struct IGUGasCoefficients
    {
        double A{0.0};
        double B{0.0};
        double C{0.0};
    };

    struct IGUGasProperties
    {
        std::string name;
        double molecularWeight{0.0};
        IGUGasCoefficients conductivity;
        IGUGasCoefficients viscosity;
        IGUGasCoefficients specificHeat;
    };

    struct IGUPureGas
    {
        double fraction{0.0};
        IGUGasProperties properties;
    };

    struct IGUGas
    {
        size_t ID{0u};
        std::string name;
        double thickness{0.0};
        double conductivity{0.0};
        std::vector<IGUPureGas> components;
    };

    struct ShadeProperties
    {
        double dBottom{0.0};
        double dTop{0.0};
        double dLeft{0.0};
        double dRight{0.0};
        double gapKeff{0.0};
    };

    struct IGUShadeLayer
    {
        Position position{Position::Unknown};
        double emissivityRatio{0.0};
        double hcRatio{0.0};
        double thickness{0.0};
        double gapThickness{0.0};
        double modeledHeight{0.0};
        ShadeProperties properties;
    };

    struct GlazingSystem
    {
        size_t ID{0u};
        size_t index{0u};
        std::string name;
        std::vector<GlazingSystemProperties> properties;
        std::vector<IGULayer> layers;
        std::vector<IGUGas> gases;
        std::optional<IGUShadeLayer> shadeLayer;
        double actualHeight{0.0};
        size_t orientation{0u};
        bool isNominalThickness{false};
        std::string interiorBC;
        std::string exteriorBC;
        bool optionFlag{false};
        std::string source;
        double siteLineHeight{0.0};
        double eogHeight{0.0};
        double sightLineToShadeEdge{0.0};
        bool isDeflected{false};
        std::string edgeGlassTag;
        std::vector<size_t> glazingOptionIDs;
    };

}   // namespace ThermFile
