#pragma once

#include <vector>

#include "BoundaryConditions.hxx"

#include "LibraryUtilities/PropertyDifference.hxx"

namespace BCLibrary
{
    //! True when the two records describe the same physics: the same kind, and every input
    //! equal within the library tolerance (a value read back from XML text may differ in its
    //! last digits from the one written). Identity, name, colour and project are not compared.
    //! Defined as "physicalDifferences is empty", so the two can never disagree.
    [[nodiscard]] bool arePhysicalPropertiesEqual(const BoundaryCondition & lhs, const BoundaryCondition & rhs);

    //! Raw per-field physical differences between two records, for a conflict display. A
    //! different kind is one "Type" row; otherwise one row per differing input, with the
    //! path dash-stacked ("Convection-Film coefficient"). A constant reads as its value, an
    //! input read from a time series as "series: <role>". Empty iff physically equal.
    [[nodiscard]] std::vector<LibraryCommon::PropertyDifference> physicalDifferences(const BoundaryCondition & lhs,
                                                                                     const BoundaryCondition & rhs);
}   // namespace BCLibrary
