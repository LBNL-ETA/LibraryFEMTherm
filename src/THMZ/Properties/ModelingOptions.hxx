#pragma once

namespace ThermFile
{
    ////////////////////////////////////////////////////////////////////////////////////
    /// ModelingOptions
    ////////////////////////////////////////////////////////////////////////////////////

    //! \brief Certain parts of equations can be excluded from calculation.
    //!
    //! @param excludeWaterLiquidTransportation Defines whether or not water liquid transportation
    //! will be excluded from mass transfer equation
    //! @param excludeHeatOfEvaporation Defines whether or not heat of evaporation will be
    //! excluded from heat transfer equation
    //! @param excludeCapillaryConduction Defines whether or not capillary conduction will be
    //! excluded from heat transfer equation
    //! @param excludeVaporDiffusionConduction Defines whether or not vapor diffusion conduction
    //! will be excluded from heat transfer equation
    //! @param excludeLatentHeatOfFusion Defines whether or not latent heat of fusion will be
    //! excluded from heat transfer equation
    //! @param excludeThermalConductivityMoistureDependent Defines if thermal conductivity will be
    //! constant or moisture dependent
    //! @param excludeThermalConductivityTemperatureDependent Defines if thermal conductivity will be
    //! constant or temperature dependent
    struct ModelingOptions
    {
        bool excludeWaterLiquidTransportation{true};
        bool excludeHeatOfEvaporation{true};
        bool excludeCapillaryConduction{true};
        bool excludeVaporDiffusionConduction{true};
        bool excludeLatentHeatOfFusion{true};
        bool excludeThermalConductivityMoistureDependent{true};
        bool excludeThermalConductivityTemperatureDependent{true};

        [[nodiscard]] friend bool operator==(const ModelingOptions & lhs, const ModelingOptions & rhs) = default;
    };
}   // namespace ThermFile
