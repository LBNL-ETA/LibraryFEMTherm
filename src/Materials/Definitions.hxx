#pragma once

#include <vector>

namespace MaterialsLibrary
{
    enum class MaterialType
    {
        Solid,
        Cavity,
        RadiationEnclosure
    };

    enum class MaterialRoughness
    {
        VeryRough,
        Rough,
        MediumRough,
        MediumSmooth,
        Smooth,
        VerySmooth
    };

    enum class CavityStandard
    {
        NFRC,
        CEN,
        CENVentilated,
        NFRCWithUserDimensions,
        ISO15099,
        ISO15099Ventilated
    };

    // Used for looping over the CavityStandard enumeration
    static const std::vector<CavityStandard> enumCavityStandard{CavityStandard::NFRC,
                                                                CavityStandard::CEN,
                                                                CavityStandard::CENVentilated,
                                                                CavityStandard::NFRCWithUserDimensions,
                                                                CavityStandard::ISO15099,
                                                                CavityStandard::ISO15099Ventilated};
}   // namespace MaterialsLibrary
