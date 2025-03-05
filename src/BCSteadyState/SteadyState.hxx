#pragma once

#include <variant>
#include <optional>
#include <string>

namespace BCSteadyStateLibrary
{
    struct Convection
    {
        Convection() = default;
        Convection(double temperature, double filmCoefficient);

        double temperature{0};
        double filmCoefficient{0};
    };

    struct ConstantFlux
    {
        ConstantFlux() = default;
        explicit ConstantFlux(double flux);

        double flux{0};
    };

    struct AutomaticEnclosure
    {
        AutomaticEnclosure() = default;
        AutomaticEnclosure(double temperature, double emissivity);

        double temperature{0};
        double emissivity{0};
    };

    struct ManualEnclosure
    {};

    struct BlackBodyRadiation
    {
        BlackBodyRadiation() = default;
        explicit BlackBodyRadiation(double temperature, double emissivity);

        double temperature{0};
        double emissivity{0};
    };

    struct LinearizedRadiation
    {
        LinearizedRadiation() = default;
        explicit LinearizedRadiation(double temperature, double filmCoefficient);

        double temperature{0};
        double filmCoefficient{0};
    };

    using RadiationOptions = std::variant<AutomaticEnclosure, ManualEnclosure, BlackBodyRadiation, LinearizedRadiation>;

    struct Radiation
    {
        Radiation() = default;
        explicit Radiation(const AutomaticEnclosure & automaticEnclosure);
        explicit Radiation(const ManualEnclosure & manualEnclosure);
        explicit Radiation(const BlackBodyRadiation & blackBodyRadiation);
        explicit Radiation(const LinearizedRadiation & linearizedRadiation);

        std::optional<RadiationOptions> radiation;
    };

    struct Comprehensive
    {
        Comprehensive() = default;
        Comprehensive(double relativeHumidity,
                      std::optional<Convection> convection,
                      std::optional<ConstantFlux> constantFlux,
                      std::optional<Radiation> radiation);

        double relativeHumidity{0.5};
        std::optional<Convection> convection;
        std::optional<ConstantFlux> constantFlux;
        std::optional<Radiation> radiation;
    };

    struct Simplified
    {
        Simplified() = default;
        Simplified(double temperature, double filmCoefficient, double relativeHumidity);

        double temperature{0};
        double filmCoefficient{0};
        double relativeHumidity{0.5};
    };

    struct RadiationSurface
    {
        RadiationSurface() = default;
        RadiationSurface(double temperature, double emissivity, bool isDefault = false);

        bool isDefault{false};
        double temperature{0};
        double emissivity{0};
    };

    struct BoundaryCondition
    {
        BoundaryCondition() = default;
        BoundaryCondition(std::string UUID,
                          std::string Name,
                          bool Protected,
                          std::string Color,
                          std::variant<Comprehensive, Simplified, RadiationSurface> data,
                          std::string ProjectName,
                          bool isIGUSurface = false);

        std::string UUID;
        std::string Name;
        bool Protected{false};
        std::string Color;
        std::variant<Comprehensive, Simplified, RadiationSurface> data;

        std::string ProjectName;

        //! Unfortunately, the design of current code base requires this flag to be used. All other boundary conditions
        //! are treated in the same way except boundary condition IGUs.
        bool isIGUSurface{false};
    };
}   // namespace BCSteadyStateLibrary