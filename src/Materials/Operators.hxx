#pragma once

#include <vector>

#include "Materials.hxx"

#include "LibraryUtilities/PropertyDifference.hxx"

namespace MaterialsLibrary
{
    // operator!= is synthesized from operator== (C++20).
    bool operator==(const point & lhs, const point & rhs);
    bool operator==(const HygroThermal & lhs, const HygroThermal & rhs);
    bool operator==(const OpticalProperties & lhs, const OpticalProperties & rhs);
    bool operator==(const OpticalSide & lhs, const OpticalSide & rhs);
    bool operator==(const OpticalType & lhs, const OpticalType & rhs);
    bool operator==(const Infrared & lhs, const Infrared & rhs);
    bool operator==(const Integrated & lhs, const Integrated & rhs);
    bool operator==(const Optical & lhs, const Optical & rhs);
    bool operator==(const Solid & lhs, const Solid & rhs);
    bool operator==(const Material & lhs, const Material & rhs);

    // Comparing only physical properties that will actually have influence on the results of the simulation.
    bool arePhysicalPropertiesEqual(const Material & lhs, const Material & rhs);

    //! Raw per-field physical differences between two materials, covering exactly the fields
    //! arePhysicalPropertiesEqual compares (both derive from the same field list). The result is
    //! empty iff the records are physically equal, so a conflict always yields at least one row.
    //! Values are raw (number / label / absent) - the caller formats for display.
    std::vector<LibraryCommon::PropertyDifference> physicalDifferences(const Material & lhs, const Material & rhs);
}   // namespace MaterialsLibrary
