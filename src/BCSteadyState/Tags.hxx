#pragma once

#include <string>

namespace BCSteadyStateLibrary
{
    struct Comprehensive;
    struct Simplified;
    struct RadiationSurface;

    class Tags
    {
    public:
        [[nodiscard]] std::string uuid() const;
        [[nodiscard]] std::string name() const;
        [[nodiscard]] std::string protectedTag() const;
        [[nodiscard]] std::string color() const;

        [[nodiscard]] std::string boundaryCondition() const;
        [[nodiscard]] std::string boundaryConditions() const;

        [[nodiscard]] std::string comprehensive() const;
        [[nodiscard]] std::string simplified() const;
        [[nodiscard]] std::string radiationSurface() const;

        [[nodiscard]] std::string convection() const;
        [[nodiscard]] std::string constantFlux() const;
        [[nodiscard]] std::string radiation() const;
        [[nodiscard]] std::string automaticEnclosure() const;
        [[nodiscard]] std::string manualEnclosure() const;
        [[nodiscard]] std::string blackBodyRadiation() const;
        [[nodiscard]] std::string linearizedRadiation() const;

        [[nodiscard]] std::string temperature() const;
        [[nodiscard]] std::string filmCoefficient() const;
        [[nodiscard]] std::string emissivity() const;
        [[nodiscard]] std::string viewFactor() const;
        [[nodiscard]] std::string flux() const;
        [[nodiscard]] std::string relativeHumidity() const;
        [[nodiscard]] std::string isDefault() const;
        [[nodiscard]] std::string isIGUSurface() const;

    private:
        const std::string uuid_{"UUID"};
        const std::string name_{"Name"};
        const std::string protectedTag_{"Protected"};
        const std::string color_{"Color"};

        const std::string boundaryCondition_{"BoundaryCondition"};
        const std::string boundaryConditions_{"BoundaryConditions"};

        const std::string comprehensive_{"Comprehensive"};
        const std::string simplified_{"Simplified"};
        const std::string radiationSurface_{"RadiationSurface"};

        const std::string convection_{"Convection"};
        const std::string constantFlux_{"ConstantFlux"};
        const std::string radiation_{"Radiation"};
        const std::string automaticEnclosure_{"AutomaticEnclosure"};
        const std::string manualEnclosure_{"ManualEnclosure"};
        const std::string blackBodyRadiation_{"BlackBodyRadiation"};
        const std::string linearizedRadiation_{"Linear"};

        const std::string temperature_{"Temperature"};
        const std::string filmCoefficient_{"FilmCoefficient"};
        const std::string emissivity_{"Emissivity"};
        const std::string viewFactor_{"ViewFactor"};
        const std::string flux_{"Flux"};
        const std::string relativeHumidity_{"RelativeHumidity"};
        const std::string isDefault_{"Default"};
        const std::string isIGUSurface_{"IGUSurface"};
    };

    struct BoundaryConditionTagVisitor
    {
        std::string operator()(const Comprehensive & c) const;

        std::string operator()(const Simplified & s) const;

        std::string operator()(const RadiationSurface & r) const;
    };

}   // namespace BCSteadyState