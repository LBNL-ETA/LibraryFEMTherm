#include "SteadyState.hxx"

namespace BCSteadyStateLibrary
{
    Convection::Convection(double temperature, double filmCoefficient) :
        temperature(temperature),
        filmCoefficient(filmCoefficient)
    {}

    ConstantFlux::ConstantFlux(double flux) : flux(flux)
    {}

    AutomaticEnclosure::AutomaticEnclosure(double temperature, double emissivity) :
        temperature(temperature),
        emissivity(emissivity)
    {}

    BlackBodyRadiation::BlackBodyRadiation(double temperature, double emissivity) :
        temperature(temperature),
        emissivity(emissivity)
    {}

    LinearizedRadiation::LinearizedRadiation(double temperature, double filmCoefficient) :
        temperature(temperature),
        filmCoefficient(filmCoefficient)
    {}

    Radiation::Radiation(const AutomaticEnclosure & automaticEnclosure) : radiation(automaticEnclosure)
    {}

    Radiation::Radiation(const ManualEnclosure & manualEnclosure) : radiation(manualEnclosure)
    {}

    Radiation::Radiation(const BlackBodyRadiation & blackBodyRadiation) : radiation(blackBodyRadiation)
    {}

    Radiation::Radiation(const LinearizedRadiation & linearizedRadiation) : radiation(linearizedRadiation)
    {}

    Simplified::Simplified(double temperature, double filmCoefficient, double relativeHumidity) :
        temperature(temperature),
        filmCoefficient(filmCoefficient),
        relativeHumidity(relativeHumidity)
    {}

    RadiationSurface::RadiationSurface(double temperature, double emissivity, bool isDefault) :
        isDefault(isDefault),
        temperature(temperature),
        emissivity(emissivity)
    {}

}   // namespace BCSteadyState