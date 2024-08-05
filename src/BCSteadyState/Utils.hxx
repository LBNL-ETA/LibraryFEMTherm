#pragma once

#include <optional>

namespace BCSteadyStateLibrary
{
    struct BoundaryCondition;
    bool isGreyBodyRadiation(const BoundaryCondition& bc);
    bool isAutoGreyBodyRadiation(const BoundaryCondition& bc);
    bool isBlackBodyRadiation(const BoundaryCondition& bc);
    bool isLinearizedRadiation(const BCSteadyStateLibrary::BoundaryCondition & bc);
    bool isRadiationBC(const BoundaryCondition& bc);
    bool isRadiationSurface(const BoundaryCondition& bc);
    bool isSimplified(const BoundaryCondition& bc);
    bool isComprehensive(const BoundaryCondition& bc);
    bool isAdiabatic(const BoundaryCondition& bc);
    bool isConvective(const BoundaryCondition& bc);
    bool isConstantFlux(const BoundaryCondition& bc);
    bool isComprehensiveWithConvection(const BoundaryCondition& bc);
    std::optional<double> getRelativeHumidity(const BoundaryCondition& bc);
    std::optional<double> airTemperature(const BoundaryCondition& bc);
    std::optional<double> filmCoefficient(const BoundaryCondition& bc);
    std::optional<double> radiationTemperature(const BoundaryCondition& bc);
    std::optional<double> viewFactor(const BoundaryCondition& bc);
    std::optional<double> radiationFilmCoefficient(const BoundaryCondition& bc);
    std::optional<double> emissivity(const BoundaryCondition& bc);
    std::optional<double> constantHeatFlux(const BoundaryCondition& bc);

    enum class BoundaryConditionType
    {
        None,
        Comprehensive,
        Simplified,
        RadiationSurface
    };

    enum class RadiationType
    {
        None,
        AutoEnclosure,
        MaunalEnclosure,
        BlackBody,
        Linearized
    };

    BoundaryConditionType getBoundaryConditionType(const BoundaryCondition& bc);
}
