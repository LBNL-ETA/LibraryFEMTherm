#pragma once

#include <map>

#include "Definitions.hxx"

namespace CMALibrary
{
    struct Environment
    {
        // Make default constructor and constructor with arguments
        // explicit to avoid implicit conversion.
        explicit Environment() = default;
        explicit Environment(double tin, double tout);

        double Tin{21.0};     // Celsius
        double Tout{-18.0};   // Celsius
    };

    struct LayerValues
    {
        explicit LayerValues() = default;
        LayerValues(double hcin, double hcout, double emissivityIn, double emissivityOut);

        double hcin{0.0};    // W/m^2-K
        double hcout{0.0};   // W/m^2-K
        double emissivityIn{0.0};
        double emissivityOut{0.0};
    };

    struct LayerOptionsKey
    {
        LayerOptionsKey() = default;
        LayerOptionsKey(Option glazing, Option spacer) : glazing(glazing), spacer(spacer){};

        Option glazing{Option::Low};
        Option spacer{Option::Low};
    };

    bool operator<(const LayerOptionsKey & lhs, const LayerOptionsKey & rhs);

    using LayerOptions = std::map<LayerOptionsKey, LayerValues>;

    struct SingleLayer
    {
        explicit SingleLayer() = default;
        explicit SingleLayer(double thickness, LayerOptions layerOptions);

        double thickness{0.006};   // m
        double conductivity{1.0};  // W/m-K
        LayerOptions layerOptions{{{Option::Low, Option::Low}, {3.415185, 26, 0.026, 0.84}},
                                  {{Option::Low, Option::High}, {3.415185, 26, 0.026, 0.84}},
                                  {{Option::High, Option::Low}, {3.529954, 26, 0.84, 0.84}},
                                  {{Option::High, Option::High}, {3.529954, 26, 0.84, 0.84}}};
    };

    struct DoubleLayer
    {
        explicit DoubleLayer() = default;
        explicit DoubleLayer(double thickness1, double thickness2, LayerOptions layerOptions);

        double thickness1{0.006};   // m
        double thickness2{0.006};   // m
        double conductivity1{1.0};  // W/m-K
        double conductivity2{1.0};  // W/m-K
        LayerOptions layerOptions{{{Option::Low, Option::Low}, {1.854252, 26, 0.84, 0.84}},
                                  {{Option::Low, Option::High}, {1.854252, 26, 0.84, 0.84}},
                                  {{Option::High, Option::Low}, {2.866122, 26, 0.84, 0.84}},
                                  {{Option::High, Option::High}, {2.866122, 26, 0.84, 0.84}}};
    };

    struct CMAData
    {
        std::string version{"1"};
        std::map<Option, double> glazingConductance{
          {Option::Low, 0.498817},   // W/m^2-K
          {Option::High, 5.880546}   // W/m^2-K
        };
        std::map<Option, double> spacerConductivity{
          {Option::Low, 0.01},   // W/m-K
          {Option::High, 10.0}   // W/m-K
        };

        Environment environment;
        SingleLayer singleLayer;
        DoubleLayer doubleLayer;
    };

}   // namespace CMAXML