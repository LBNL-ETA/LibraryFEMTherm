#pragma once

#include <optional>

#include "BoundaryConditions.hxx"

//! Steady-state questions asked of a unified record, mirroring what the legacy
//! steady-state library answered about its own.
//!
//! Every value here is a steady one, so an input reading a time series has no answer and
//! returns nothing. Ask isSteadyCapable first when the distinction between "absent" and
//! "varies" matters: on its own, a missing value cannot tell the two apart.
namespace BCLibrary
{
    [[nodiscard]] bool isRadiationSurface(const BoundaryCondition & bc);
    [[nodiscard]] bool isAdiabatic(const BoundaryCondition & bc);
    [[nodiscard]] bool isConvective(const BoundaryCondition & bc);
    [[nodiscard]] bool isConstantFlux(const BoundaryCondition & bc);

    [[nodiscard]] bool isAutoGreyBodyRadiation(const BoundaryCondition & bc);
    [[nodiscard]] bool isGreyBodyRadiation(const BoundaryCondition & bc);
    [[nodiscard]] bool isBlackBodyRadiation(const BoundaryCondition & bc);
    [[nodiscard]] bool isLinearizedRadiation(const BoundaryCondition & bc);
    [[nodiscard]] bool isRadiationBC(const BoundaryCondition & bc);

    [[nodiscard]] std::optional<double> getRelativeHumidity(const BoundaryCondition & bc);
    [[nodiscard]] std::optional<double> airTemperature(const BoundaryCondition & bc);
    [[nodiscard]] std::optional<double> filmCoefficient(const BoundaryCondition & bc);
    [[nodiscard]] std::optional<double> radiationTemperature(const BoundaryCondition & bc);
    [[nodiscard]] std::optional<double> radiationFilmCoefficient(const BoundaryCondition & bc);
    [[nodiscard]] std::optional<double> emissivity(const BoundaryCondition & bc);
    [[nodiscard]] std::optional<double> constantHeatFlux(const BoundaryCondition & bc);

    //! Reports the black body's EMISSIVITY, not its view factor.
    //!
    //! That is what the legacy accessor did, and the solver input has been built from it
    //! ever since, so this reproduces it rather than correcting it. Every shipped record
    //! sets the two to one, which is why the difference has never shown. Correcting it
    //! would change results and belongs in its own change.
    [[nodiscard]] std::optional<double> viewFactor(const BoundaryCondition & bc);
}   // namespace BCLibrary
