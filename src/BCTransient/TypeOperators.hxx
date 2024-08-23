#pragma once

#include "Types.hxx"

namespace BCTypesLibrary
{
    bool operator==(const ConvectionRecord & lhs, const ConvectionRecord & rhs);
    bool operator!=(const ConvectionRecord & lhs, const ConvectionRecord & rhs);

    bool operator==(const RadiationRecord & lhs, const RadiationRecord & rhs);
    bool operator!=(const RadiationRecord & lhs, const RadiationRecord & rhs);

    bool operator==(const SolarRecord & lhs, const SolarRecord & rhs);
    bool operator!=(const SolarRecord & lhs, const SolarRecord & rhs);

    bool operator==(const TypeRecord & lhs, const TypeRecord & rhs);
    bool operator!=(const TypeRecord & lhs, const TypeRecord & rhs);
}   // namespace BCTypesLibrary
