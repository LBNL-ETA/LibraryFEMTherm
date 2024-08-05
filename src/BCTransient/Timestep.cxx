#include <fileParse/XMLNodeAdapter.hxx>
#include <fileParse/Vector.hxx>

#include "Timestep.hxx"

#include "TimestepSerializers.hxx"

namespace BCInputFileLibrary
{
    /////////////////////////////////////////////////////////////////////////////////
    // TARPTimestep
    /////////////////////////////////////////////////////////////////////////////////

    TARPTimestep::TARPTimestep(const std::optional<size_t> & timestepID, double temp, double hum) :
        id(timestepID), temperature(temp), humidity(hum)
    {}

    TARPTimestep::TARPTimestep(double temperature, double humidity) : temperature(temperature), humidity(humidity)
    {}

    /////////////////////////////////////////////////////////////////////////////////
    // ASHRAEInside
    /////////////////////////////////////////////////////////////////////////////////

    ASHRAEInside::ASHRAEInside(const std::optional<size_t> & id, double temp, double hum) :
        id(id), temperature(temp), humidity(hum)
    {}

    ASHRAEInside::ASHRAEInside(double temperature, double humidity) : temperature(temperature), humidity(humidity)
    {}

    /////////////////////////////////////////////////////////////////////////////////
    // ASHRAEOutside
    /////////////////////////////////////////////////////////////////////////////////

    ASHRAEOutside::ASHRAEOutside(const std::optional<size_t> & id,
                                 double temperature,
                                 double humidity,
                                 double wind_speed) :
        id(id), temperature(temperature), humidity(humidity), windSpeed(wind_speed)
    {}

    ASHRAEOutside::ASHRAEOutside(double temperature, double humidity, double wind_speed) :
        temperature(temperature), humidity(humidity), windSpeed(wind_speed)
    {}

    /////////////////////////////////////////////////////////////////////////////////
    // FixedFilmCoefficient
    /////////////////////////////////////////////////////////////////////////////////

    FixedFilmCoefficient::FixedFilmCoefficient(const std::optional<size_t> & id,
                                               double temperature,
                                               double humidity,
                                               double fixed_film_coefficient) :
        id(id), temperature(temperature), humidity(humidity), fixedFilmCoefficient(fixed_film_coefficient)
    {}

    FixedFilmCoefficient::FixedFilmCoefficient(double temperature, double humidity, double fixed_film_coefficient) :
        temperature(temperature), humidity(humidity), fixedFilmCoefficient(fixed_film_coefficient)
    {}

    /////////////////////////////////////////////////////////////////////////////////
    // YazdanianKlems
    /////////////////////////////////////////////////////////////////////////////////

    YazdanianKlems::YazdanianKlems(
      const std::optional<size_t> & id, double temperature, double humidity, double wind_speed, double wind_direction) :
        id(id), temperature(temperature), humidity(humidity), windSpeed(wind_speed), windDirection(wind_direction)
    {}

    YazdanianKlems::YazdanianKlems(double temperature, double humidity, double wind_speed, double wind_direction) :
        temperature(temperature), humidity(humidity), windSpeed(wind_speed), windDirection(wind_direction)
    {}

    /////////////////////////////////////////////////////////////////////////////////
    // Kimura
    /////////////////////////////////////////////////////////////////////////////////

    Kimura::Kimura(
      const std::optional<size_t> & id, double temperature, double humidity, double wind_speed, double wind_direction) :
        id(id), temperature(temperature), humidity(humidity), windSpeed(wind_speed), windDirection(wind_direction)
    {}

    Kimura::Kimura(double temperature, double humidity, double wind_speed, double wind_direction) :
        temperature(temperature), humidity(humidity), windSpeed(wind_speed), windDirection(wind_direction)
    {}

    /////////////////////////////////////////////////////////////////////////////////
    // Montazeri
    /////////////////////////////////////////////////////////////////////////////////

    Montazeri::Montazeri(
      const std::optional<size_t> & id, double temperature, double humidity, double wind_speed, double wind_direction) :
        id(id), temperature(temperature), humidity(humidity), windSpeed(wind_speed), windDirection(wind_direction)
    {}

    Montazeri::Montazeri(double temperature, double humidity, double wind_speed, double wind_direction) :
        temperature(temperature), humidity(humidity), windSpeed(wind_speed), windDirection(wind_direction)
    {}

    /////////////////////////////////////////////////////////////////////////////////
    // FixedRadiationCoefficient
    /////////////////////////////////////////////////////////////////////////////////

    FixedRadiation::FixedRadiation(const std::optional<size_t> & id, double temperature, double hr) :
        id(id), temperature(temperature), hr(hr)
    {}

    FixedRadiation::FixedRadiation(double temperature, double hr) : temperature(temperature), hr(hr)
    {}

    /////////////////////////////////////////////////////////////////////////////////
    // BlackBodyRadiation
    /////////////////////////////////////////////////////////////////////////////////

    BlackBodyRadiation::BlackBodyRadiation(const std::optional<size_t> & id, double temperature, double emissivity) :
        id(id), temperature(temperature), emissivity(emissivity)
    {}

    BlackBodyRadiation::BlackBodyRadiation(double temperature, double emissivity) :
        temperature(temperature), emissivity(emissivity)
    {}

    /////////////////////////////////////////////////////////////////////////////////
    // HeatFlux
    /////////////////////////////////////////////////////////////////////////////////

    HeatFlux::HeatFlux(const std::optional<size_t> & id, double heat_flux) : id(id), heatFlux(heat_flux)
    {}

    HeatFlux::HeatFlux(double heat_flux) : heatFlux(heat_flux)
    {}

    /////////////////////////////////////////////////////////////////////////////////
    // SolarRadiation
    /////////////////////////////////////////////////////////////////////////////////
    SolarRadiation::SolarRadiation(const std::optional<size_t> & id, double solar_radiation) :
        id(id), solarRadiation(solar_radiation)
    {}

    SolarRadiation::SolarRadiation(double solar_radiation) : solarRadiation(solar_radiation)
    {}

    /////////////////////////////////////////////////////////////////////////////////
    // FixedTemperature
    /////////////////////////////////////////////////////////////////////////////////

    FixedTemperature::FixedTemperature(const std::optional<size_t> & id, double temperature) :
        id(id), temperature(temperature)
    {}

    FixedTemperature::FixedTemperature(double temperature) : temperature(temperature)
    {}

    /////////////////////////////////////////////////////////////////////////////////
    // FixedHumidity
    /////////////////////////////////////////////////////////////////////////////////

    FixedHumidity::FixedHumidity(const std::optional<size_t> & id, double humidity) : id(id), humidity(humidity)
    {}

    FixedHumidity::FixedHumidity(double humidity) : humidity(humidity)
    {}

    void BoundaryConditionTimestep::loadFromXMLFile(const std::string & fileName)
    {
        const auto xInputDataNode{getTopNodeFromFile(fileName, "InputBoundaryConditionsData")};

        if(xInputDataNode.has_value())
        {
            xInputDataNode.value() >> FileParse::Child{"Version", version};
            xInputDataNode.value() >> FileParse::Child{"Name", name};

            auto optTransient{xInputDataNode->getFirstChildByName("BoundaryConditionTransient")};
            if(optTransient)
            {
                XMLNodeAdapter xBCTransient{optTransient.value()};
                xBCTransient >> convection;
                xBCTransient >> radiation;
                xBCTransient >> FileParse::Child{"TemperatureTimestep", temperature};
                xBCTransient >> FileParse::Child{"HumidityTimestep", humidity};
                xBCTransient >> FileParse::Child{"HeatFluxTimestep", heatFlux};
                xBCTransient >> FileParse::Child{"SolarRadiationTimestep", solarRadiation};
            }
        }
    }

    bool BoundaryConditionTimestep::isLoadSuccesful() const
    {
        return true;
    }

    std::string BoundaryConditionTimestep::errorMessage() const
    {
        return "";
    }

}   // namespace BCInputFileLibrary
