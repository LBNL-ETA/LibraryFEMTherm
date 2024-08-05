#include "Operators.hxx"

#include "Materials.hxx"
#include "LibraryUtilities/Common.hxx"

namespace MaterialsLibrary
{
    bool operator==(const point & lhs, const point & rhs)
    {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }

    bool operator!=(const point & lhs, const point & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const HygroThermal & lhs, const HygroThermal & rhs)
    {
        constexpr auto tolerance{1e-6};
        return LibraryCommon::isEqual(lhs.DefaultThickness, rhs.DefaultThickness, tolerance)
               && lhs.MaterialInformation == rhs.MaterialInformation
               && LibraryCommon::isEqual(lhs.BulkDensity, rhs.BulkDensity, tolerance)
               && LibraryCommon::isEqual(lhs.Porosity, rhs.Porosity, tolerance)
               && LibraryCommon::isEqual(lhs.SpecificHeatCapacityDry, rhs.SpecificHeatCapacityDry, tolerance)
               && LibraryCommon::isEqual(lhs.ThermalConductivityDry, rhs.ThermalConductivityDry, tolerance)
               && lhs.Roughness == rhs.Roughness
               && LibraryCommon::isEqual(
                 lhs.WaterVaporDiffusionResistanceFactor, rhs.WaterVaporDiffusionResistanceFactor, tolerance)
               && lhs.MoistureStorageFunction == rhs.MoistureStorageFunction
               && lhs.LiquidTransportationCoefficientSuction == rhs.LiquidTransportationCoefficientSuction
               && lhs.LiquidTransportationCoefficientRedistribution == rhs.LiquidTransportationCoefficientRedistribution
               && lhs.ThermalConductivityMoistureDependent == rhs.ThermalConductivityMoistureDependent
               && lhs.ThermalConductivityTemperatureDependent == rhs.ThermalConductivityTemperatureDependent;
    }

    bool operator!=(const HygroThermal & lhs, const HygroThermal & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const Cavity & lhs, const Cavity & rhs)
    {
        constexpr auto tolerance{1e-6};
        return LibraryCommon::isEqual(lhs.EmissivitySide1, rhs.EmissivitySide1, tolerance)
               && LibraryCommon::isEqual(lhs.EmissivitySide2, rhs.EmissivitySide2, tolerance)
               && lhs.cavityStandard == rhs.cavityStandard && lhs.GasName == rhs.GasName;
    }

    bool operator!=(const Cavity & lhs, const Cavity & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const OpticalProperties & lhs, const OpticalProperties & rhs)
    {
        return lhs.Transmittance == rhs.Transmittance && lhs.Reflectance == rhs.Reflectance;
    }

    bool operator!=(const OpticalProperties & lhs, const OpticalProperties & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const OpticalSide & lhs, const OpticalSide & rhs)
    {
        return lhs.Front == rhs.Front && lhs.Back == rhs.Back;
    }

    bool operator!=(const OpticalSide & lhs, const OpticalSide & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const OpticalType & lhs, const OpticalType & rhs)
    {
        return lhs.Direct == rhs.Direct && lhs.Diffuse == rhs.Diffuse;
    }

    bool operator!=(const OpticalType & lhs, const OpticalType & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const Integrated & lhs, const Integrated & rhs)
    {
        return lhs.IR == rhs.IR && lhs.Solar == rhs.Solar && lhs.Visible == rhs.Visible;
    }

    bool operator!=(const Integrated & lhs, const Integrated & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const Infrared & lhs, const Infrared & rhs)
    {
        constexpr auto tolerance{1e-6};
        return LibraryCommon::isEqual(lhs.Transmittance, rhs.Transmittance, tolerance)
               && LibraryCommon::isEqual(lhs.EmissivityFront, rhs.EmissivityFront, tolerance)
               && LibraryCommon::isEqual(lhs.EmissivityBack, rhs.EmissivityBack, tolerance);
    }

    bool operator!=(const Infrared & lhs, const Infrared & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const Optical & lhs, const Optical & rhs)
    {
        return lhs.integrated == rhs.integrated;
    }

    bool operator!=(const Optical & lhs, const Optical & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const Material & lhs, const Material & rhs)
    {
        return lhs.UUID == rhs.UUID && lhs.Name == rhs.Name && lhs.Protected == rhs.Protected && lhs.Color == rhs.Color
               && lhs.data == rhs.data;
    }

    bool operator!=(const Material & lhs, const Material & rhs)
    {
        return !(lhs == rhs);
    }

    bool arePhysicalPropertiesEqual(const Material & lhs, const Material & rhs)
    {
        return lhs.data == rhs.data;
    }

    bool operator==(const Solid & lhs, const Solid & rhs)
    {
        return lhs.hygroThermal == rhs.hygroThermal && lhs.optical == rhs.optical;
    }

    bool operator!=(const Solid & lhs, const Solid & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const RadiationEnclosure & lhs, const RadiationEnclosure & rhs)
    {
        constexpr auto tolerance{1e-6};
        return LibraryCommon::isEqual(lhs.emissivityDefault, rhs.emissivityDefault, tolerance);
    }

    bool operator!=(const RadiationEnclosure & lhs, const RadiationEnclosure & rhs)
    {
        return !(lhs == rhs);
    }

}   // namespace MaterialsLibrary
