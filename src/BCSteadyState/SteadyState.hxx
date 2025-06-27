#pragma once

#include <variant>
#include <optional>
#include <string>

namespace BCSteadyStateLibrary
{
    struct Convection
    {
        double temperature{0};
        double filmCoefficient{0};
    };

    struct ConstantFlux
    {
        double flux{0};
    };

    struct AutomaticEnclosure
    {
        double temperature{0};
        double emissivity{0};
    };

    struct ManualEnclosure
    {};

    struct BlackBodyRadiation
    {
        double temperature{0};
        double emissivity{0};
        double viewFactor{0};
    };

    struct LinearizedRadiation
    {
        double temperature{0};
        double filmCoefficient{0};
    };

    using RadiationOptions = std::variant<AutomaticEnclosure, ManualEnclosure, BlackBodyRadiation, LinearizedRadiation>;

    struct Radiation
    {
        std::optional<RadiationOptions> radiation;
    };

    struct Comprehensive
    {
        double relativeHumidity{0.5};
        std::optional<Convection> convection;
        std::optional<ConstantFlux> constantFlux;
        std::optional<Radiation> radiation;
    };

    struct Simplified
    {
        double temperature{0};
        double filmCoefficient{0};
        double relativeHumidity{0.5};
    };

    struct RadiationSurface
    {
        bool isDefault{false};
        double temperature{0};
        double emissivity{0};
    };

    struct BoundaryCondition
    {
        std::string UUID;
        std::string Name{"Default Name"};
        bool Protected{false};
        std::string Color{"0x0078D7"};
        std::variant<Comprehensive, Simplified, RadiationSurface> data;

        std::string ProjectName;

        //! Unfortunately, the design of current code base requires this flag to be used. All other boundary conditions
        //! are treated in the same way except boundary condition IGUs.
        bool isIGUSurface{false};
    };
}   // namespace BCSteadyStateLibrary