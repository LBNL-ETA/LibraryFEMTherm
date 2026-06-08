#pragma once

#include <optional>
#include <vector>

#include "Gases.hxx"

#include "LibraryUtilities/PropertyDifference.hxx"

namespace GasesLibrary
{
    // operator!= is synthesized from operator== (C++20).
    bool operator==(const Coefficients & lhs, const Coefficients & rhs);
    bool operator==(const GasProperties & lhs, const GasProperties & rhs);
    bool operator==(const GasCommon & lhs, const GasCommon & rhs);
    bool operator==(const PureGas & lhs, const PureGas & rhs);
    bool operator==(const Component & lhs, const Component & rhs);
    bool operator==(const Gas & lhs, const Gas & rhs);
    bool operator==(const GasesData & lhs, const GasesData & rhs);

    // The following functions are used to compare the physical properties of gases. These properties will have impact
    // on the results of the simulation and thus important to compare.
    bool arePhysicalPropertiesEqual(const PureGas & lhs, const PureGas & rhs);
    bool arePhysicalPropertiesEqual(const std::optional<PureGas> & lhs, const std::optional<PureGas> & rhs);
    bool arePhysicalPropertiesEqual(const Gas & lhs, const Gas & rhs);
    bool arePhysicalPropertiesEqual(const GasesData & lhs, const GasesData & rhs);

    //! Raw per-field physical differences between two gas records, covering each pure component's
    //! properties (molecular weight, specific heat ratio, conductivity/viscosity/specific-heat
    //! coefficients) plus mixture fractions and component count. Empty iff physically equal; a
    //! conflict always yields at least one row.
    std::vector<LibraryCommon::PropertyDifference> physicalDifferences(const GasesData & lhs, const GasesData & rhs);
}   // namespace GasesLibrary
