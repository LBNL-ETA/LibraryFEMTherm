#include "TypeOperators.hxx"
#include "Types.hxx"

namespace BCTypesLibrary
{
    bool operator==(const ConvectionRecord & lhs, const ConvectionRecord & rhs)
    {
        return lhs.Model == rhs.Model && lhs.Temperature == rhs.Temperature
               && lhs.FilmCoefficient == rhs.FilmCoefficient && lhs.WindSpeed == rhs.WindSpeed
               && lhs.WindDirection == rhs.WindDirection;
    }

    bool operator!=(const ConvectionRecord & lhs, const ConvectionRecord & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const RadiationRecord & lhs, const RadiationRecord & rhs)
    {
        return lhs.Model == rhs.Model && lhs.Temperature == rhs.Temperature
               && lhs.SurfaceEmissivity == rhs.SurfaceEmissivity && lhs.ViewFactor == rhs.ViewFactor
               && lhs.FilmCoefficient == rhs.FilmCoefficient;
    }

    bool operator!=(const RadiationRecord & lhs, const RadiationRecord & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const SolarRecord & lhs, const SolarRecord & rhs)
    {
        return lhs.SolarRadiation == rhs.SolarRadiation && lhs.Absorptance == rhs.Absorptance;
    }

    bool operator!=(const SolarRecord & lhs, const SolarRecord & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const TypeRecord & lhs, const TypeRecord & rhs)
    {
        return lhs.UUID == rhs.UUID && lhs.Name == rhs.Name && lhs.Protected == rhs.Protected
               && lhs.BCType == rhs.BCType && lhs.BCModel == rhs.BCModel && lhs.ConvectionBc == rhs.ConvectionBc
               && lhs.RadiationBc == rhs.RadiationBc && lhs.SolarBc == rhs.SolarBc && lhs.UseHeatFlux == rhs.UseHeatFlux
               && lhs.HeatFlux == rhs.HeatFlux && lhs.UseTemperature == rhs.UseTemperature
               && lhs.Temperature == rhs.Temperature && lhs.UseHumidity == rhs.UseHumidity
               && lhs.Humidity == rhs.Humidity && lhs.Color == rhs.Color;
    }

    bool operator!=(const TypeRecord & lhs, const TypeRecord & rhs)
    {
        return !(lhs == rhs);
    }
}   // namespace BCTypesLibrary
