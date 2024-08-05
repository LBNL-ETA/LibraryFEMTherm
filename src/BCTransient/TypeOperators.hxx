#pragma once

namespace BCTypesLibrary
{
    struct ConvectionRecord;
    bool operator==(const ConvectionRecord & lhs, const ConvectionRecord & rhs);
    bool operator!=(const ConvectionRecord & lhs, const ConvectionRecord & rhs);

    struct RadiationRecord;
    bool operator==(const RadiationRecord & lhs, const RadiationRecord & rhs);
    bool operator!=(const RadiationRecord & lhs, const RadiationRecord & rhs);

    struct SolarRecord;
    bool operator==(const SolarRecord & lhs, const SolarRecord & rhs);
    bool operator!=(const SolarRecord & lhs, const SolarRecord & rhs);

    struct TypeRecord;
    bool operator==(const TypeRecord & lhs, const TypeRecord & rhs);
    bool operator!=(const TypeRecord & lhs, const TypeRecord & rhs);
}   // namespace BCTypesLibrary
