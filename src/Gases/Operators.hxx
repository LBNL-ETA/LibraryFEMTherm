#pragma once

#include <optional>

#include "Gases.hxx"

namespace GasesLibrary
{
    bool operator==(const Coefficients & lhs, const Coefficients & rhs);
    bool operator!=(const Coefficients & lhs, const Coefficients & rhs);

    bool operator==(const GasProperties & lhs, const GasProperties & rhs);
    bool operator!=(const GasProperties & lhs, const GasProperties & rhs);

    bool operator==(const GasCommon & lhs, const GasCommon & rhs);
    bool operator!=(const GasCommon & lhs, const GasCommon & rhs);

    bool operator==(const PureGas & lhs, const PureGas & rhs);
    bool operator!=(const PureGas & lhs, const PureGas & rhs);

    bool operator==(const Component & lhs, const Component & rhs);
    bool operator!=(const Component & lhs, const Component & rhs);

    bool operator==(const Gas & lhs, const Gas & rhs);
    bool operator!=(const Gas & lhs, const Gas & rhs);

    bool operator==(const GasesData & lhs, const GasesData & rhs);
    bool operator!=(const GasesData & lhs, const GasesData & rhs);

    // The following functions are used to compare the physical properties of gases. These properties will have impact
    // on the results of the simulation and thus important to compare.
    bool arePhysicalPropertiesEqual(const PureGas & lhs, const PureGas & rhs);
    bool arePhysicalPropertiesEqual(const std::optional<PureGas> & lhs, const std::optional<PureGas> & rhs);
    bool arePhysicalPropertiesEqual(const Gas & lhs, const Gas & rhs);
    bool arePhysicalPropertiesEqual(const GasesData & lhs, const GasesData & rhs);
}   // namespace GasesLibrary
