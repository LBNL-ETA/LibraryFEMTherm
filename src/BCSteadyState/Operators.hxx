#pragma once

#include "SteadyState.hxx"

namespace BCSteadyStateLibrary
{
    bool operator==(const Convection & lhs, const Convection & rhs);
    bool operator!=(const Convection & lhs, const Convection & rhs);

    bool operator==(const ConstantFlux & lhs, const ConstantFlux & rhs);
    bool operator!=(const ConstantFlux & lhs, const ConstantFlux & rhs);

    bool operator==(const AutomaticEnclosure & lhs, const AutomaticEnclosure & rhs);
    bool operator!=(const AutomaticEnclosure & lhs, const AutomaticEnclosure & rhs);

    bool operator==(const ManualEnclosure & lhs, const ManualEnclosure & rhs);
    bool operator!=(const ManualEnclosure & lhs, const ManualEnclosure & rhs);

    bool operator==(const BlackBodyRadiation & lhs, const BlackBodyRadiation & rhs);
    bool operator!=(const BlackBodyRadiation & lhs, const BlackBodyRadiation & rhs);

    bool operator==(const LinearizedRadiation & lhs, const LinearizedRadiation & rhs);
    bool operator!=(const LinearizedRadiation & lhs, const LinearizedRadiation & rhs);

    bool operator==(const Radiation & lhs, const Radiation & rhs);
    bool operator!=(const Radiation & lhs, const Radiation & rhs);

    bool operator==(const Comprehensive & lhs, const Comprehensive & rhs);
    bool operator!=(const Comprehensive & lhs, const Comprehensive & rhs);

    bool operator==(const Simplified & lhs, const Simplified & rhs);
    bool operator!=(const Simplified & lhs, const Simplified & rhs);

    bool operator==(const RadiationSurface & lhs, const RadiationSurface & rhs);
    bool operator!=(const RadiationSurface & lhs, const RadiationSurface & rhs);

    bool operator==(const BoundaryCondition & lhs, const BoundaryCondition & rhs);
    bool operator!=(const BoundaryCondition & lhs, const BoundaryCondition & rhs);

    bool arePhysicalPropertiesEqual(const BoundaryCondition & lhs, const BoundaryCondition & rhs);
}