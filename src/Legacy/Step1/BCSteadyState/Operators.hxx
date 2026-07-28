#pragma once

#include <vector>

#include "SteadyState.hxx"

#include "LibraryUtilities/PropertyDifference.hxx"

namespace BCSteadyStateLibrary
{
    // operator!= is synthesized from operator== (C++20).
    bool operator==(const Convection & lhs, const Convection & rhs);
    bool operator==(const ConstantFlux & lhs, const ConstantFlux & rhs);
    bool operator==(const AutomaticEnclosure & lhs, const AutomaticEnclosure & rhs);
    bool operator==(const ManualEnclosure & lhs, const ManualEnclosure & rhs);
    bool operator==(const BlackBodyRadiation & lhs, const BlackBodyRadiation & rhs);
    bool operator==(const LinearizedRadiation & lhs, const LinearizedRadiation & rhs);
    bool operator==(const Radiation & lhs, const Radiation & rhs);
    bool operator==(const Comprehensive & lhs, const Comprehensive & rhs);
    bool operator==(const Simplified & lhs, const Simplified & rhs);
    bool operator==(const RadiationSurface & lhs, const RadiationSurface & rhs);
    bool operator==(const BoundaryCondition & lhs, const BoundaryCondition & rhs);

    bool arePhysicalPropertiesEqual(const BoundaryCondition & lhs, const BoundaryCondition & rhs);

    //! Raw per-field physical differences between two boundary conditions, covering every field
    //! arePhysicalPropertiesEqual compares (including the radiation sub-variant and convection/
    //! flux presence). Empty iff physically equal; a conflict always yields at least one row.
    std::vector<LibraryCommon::PropertyDifference> physicalDifferences(const BoundaryCondition & lhs,
                                                                       const BoundaryCondition & rhs);
}
