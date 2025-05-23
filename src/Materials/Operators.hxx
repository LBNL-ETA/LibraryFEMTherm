#pragma once

#include "Materials.hxx"

namespace MaterialsLibrary
{
    bool operator==(const point & lhs, const point & rhs);
    bool operator!=(const point & lhs, const point & rhs);

    bool operator==(const HygroThermal & lhs, const HygroThermal & rhs);
    bool operator!=(const HygroThermal & lhs, const HygroThermal & rhs);

    bool operator==(const OpticalProperties & lhs, const OpticalProperties & rhs);
    bool operator!=(const OpticalProperties & lhs, const OpticalProperties & rhs);

    bool operator==(const OpticalSide & lhs, const OpticalSide & rhs);
    bool operator!=(const OpticalSide & lhs, const OpticalSide & rhs);

    bool operator==(const OpticalType & lhs, const OpticalType & rhs);
    bool operator!=(const OpticalType & lhs, const OpticalType & rhs);

    bool operator==(const Infrared & lhs, const Infrared & rhs);
    bool operator!=(const Infrared & lhs, const Infrared & rhs);

    bool operator==(const Integrated & lhs, const Integrated & rhs);
    bool operator!=(const Integrated & lhs, const Integrated & rhs);

    bool operator==(const Optical & lhs, const Optical & rhs);
    bool operator!=(const Optical & lhs, const Optical & rhs);

    bool operator==(const Solid & lhs, const Solid & rhs);
    bool operator!=(const Solid & lhs, const Solid & rhs);

    bool operator==(const Cavity & lhs, const Cavity & rhs);
    bool operator!=(const Cavity & lhs, const Cavity & rhs);

    bool operator==(const RadiationEnclosure & lhs, const RadiationEnclosure & rhs);
    bool operator!=(const RadiationEnclosure & lhs, const RadiationEnclosure & rhs);

    bool operator==(const Material & lhs, const Material & rhs);
    bool operator!=(const Material & lhs, const Material & rhs);

    // Comparing only physical properties that will actually have influence on the results of the simulation.
    bool arePhysicalPropertiesEqual(const Material & lhs, const Material & rhs);
}   // namespace MaterialsLibrary
