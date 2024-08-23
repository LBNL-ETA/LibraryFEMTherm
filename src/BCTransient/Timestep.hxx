#pragma once

#include <string>
#include <optional>
#include <vector>

namespace BCInputFileLibrary
{
    /////////////////////////////////////////////////////////////////////////////////
    // TARPTimestep
    /////////////////////////////////////////////////////////////////////////////////

    struct TARPTimestep
    {
        TARPTimestep() = default;
        TARPTimestep(const std::optional<size_t> & id, double temperature, double humidity);

        TARPTimestep(double temperature, double humidity);

        std::optional<size_t> id;
        double temperature{0};
        double humidity{0};
    };

    /////////////////////////////////////////////////////////////////////////////////
    // ASHRAEInside
    /////////////////////////////////////////////////////////////////////////////////

    struct ASHRAEInside
    {
        ASHRAEInside() = default;
        ASHRAEInside(const std::optional<size_t> & id, double temperature, double humidity);
        ASHRAEInside(double temperature, double humidity);

        std::optional<size_t> id;
        double temperature{0};
        double humidity{0};
    };

    /////////////////////////////////////////////////////////////////////////////////
    // ASHRAEOutside
    /////////////////////////////////////////////////////////////////////////////////

    struct ASHRAEOutside
    {
        ASHRAEOutside() = default;
        ASHRAEOutside(const std::optional<size_t> & id, double temperature, double humidity, double wind_speed);
        ASHRAEOutside(double temperature, double humidity, double wind_speed);

        std::optional<size_t> id;
        double temperature{0};
        double humidity{0};
        double windSpeed{0};
    };

    /////////////////////////////////////////////////////////////////////////////////
    // FixedFilmCoefficient
    /////////////////////////////////////////////////////////////////////////////////

    struct FixedFilmCoefficient
    {
        FixedFilmCoefficient() = default;
        FixedFilmCoefficient(const std::optional<size_t> & id,
                             double temperature,
                             double humidity,
                             double fixed_film_coefficient);
        FixedFilmCoefficient(double temperature, double humidity, double fixed_film_coefficient);

        std::optional<size_t> id;
        double temperature{0};
        double humidity{0};
        double fixedFilmCoefficient{0};
    };

    /////////////////////////////////////////////////////////////////////////////////
    // YazdanianKlems
    /////////////////////////////////////////////////////////////////////////////////

    struct YazdanianKlems
    {
        YazdanianKlems() = default;
        YazdanianKlems(const std::optional<size_t> & id,
                       double temperature,
                       double humidity,
                       double wind_speed,
                       double wind_direction);
        YazdanianKlems(double temperature, double humidity, double wind_speed, double wind_direction);

        std::optional<size_t> id;
        double temperature{0};
        double humidity{0};
        double windSpeed{0};
        double windDirection{0};
    };

    /////////////////////////////////////////////////////////////////////////////////
    // Kimura
    /////////////////////////////////////////////////////////////////////////////////

    struct Kimura
    {
        Kimura() = default;
        Kimura(const std::optional<size_t> & id,
               double temperature,
               double humidity,
               double wind_speed,
               double wind_direction);
        Kimura(double temperature, double humidity, double wind_speed, double wind_direction);

        std::optional<size_t> id;
        double temperature{0};
        double humidity{0};
        double windSpeed{0};
        double windDirection{0};
    };

    /////////////////////////////////////////////////////////////////////////////////
    // Montazeri
    /////////////////////////////////////////////////////////////////////////////////

    struct Montazeri
    {
        Montazeri() = default;
        Montazeri(const std::optional<size_t> & id,
                  double temperature,
                  double humidity,
                  double wind_speed,
                  double wind_direction);
        Montazeri(double temperature, double humidity, double wind_speed, double wind_direction);

        std::optional<size_t> id;
        double temperature{0};
        double humidity{0};
        double windSpeed{0};
        double windDirection{0};
    };

    /////////////////////////////////////////////////////////////////////////////////
    // ConvectionTimesteps
    /////////////////////////////////////////////////////////////////////////////////

    struct ConvectionTimesteps
    {
        std::vector<TARPTimestep> tarp;
        std::vector<ASHRAEInside> ashraeInside;
        std::vector<ASHRAEOutside> ashraeOutside;
        std::vector<FixedFilmCoefficient> fixedFilmCoefficient;
        std::vector<YazdanianKlems> yazdanianKlems;
        std::vector<Kimura> kimura;
        std::vector<Montazeri> montazeri;
    };

    /////////////////////////////////////////////////////////////////////////////////
    // FixedRadiationCoefficient
    /////////////////////////////////////////////////////////////////////////////////

    struct FixedRadiation
    {
        FixedRadiation() = default;
        FixedRadiation(const std::optional<size_t> & id, double temperature, double hr);
        FixedRadiation(double temperature, double hr);

        std::optional<size_t> id;
        double temperature{0};
        double hr{0};
    };

    /////////////////////////////////////////////////////////////////////////////////
    // BlackBodyRadiation
    /////////////////////////////////////////////////////////////////////////////////

    struct BlackBodyRadiation
    {
        BlackBodyRadiation() = default;
        BlackBodyRadiation(const std::optional<size_t> & id, double temperature, double emissivity);
        BlackBodyRadiation(double temperature, double emissivity);

        std::optional<size_t> id;
        double temperature{0};
        double emissivity{0};
    };

    /////////////////////////////////////////////////////////////////////////////////
    // RadiationTimesteps
    /////////////////////////////////////////////////////////////////////////////////

    struct RadiationTimesteps
    {
        std::vector<FixedRadiation> fixedRadiation;
        std::vector<BlackBodyRadiation> blackBodyRadiation;
    };

    /////////////////////////////////////////////////////////////////////////////////
    // HeatFlux
    /////////////////////////////////////////////////////////////////////////////////

    struct HeatFlux
    {
        HeatFlux() = default;
        HeatFlux(const std::optional<size_t> & id, double heat_flux);
        explicit HeatFlux(double heat_flux);

        std::optional<size_t> id;
        double heatFlux{0};
    };

    /////////////////////////////////////////////////////////////////////////////////
    // SolarRadiation
    /////////////////////////////////////////////////////////////////////////////////

    struct SolarRadiation
    {
        SolarRadiation() = default;
        SolarRadiation(const std::optional<size_t> & id, double solar_radiation);
        explicit SolarRadiation(double solar_radiation);

        std::optional<size_t> id;
        double solarRadiation{0};
    };

    /////////////////////////////////////////////////////////////////////////////////
    // FixedTemperature
    /////////////////////////////////////////////////////////////////////////////////

    struct FixedTemperature
    {
        FixedTemperature() = default;
        FixedTemperature(const std::optional<size_t> & id, double temperature);
        explicit FixedTemperature(double temperature);

        std::optional<size_t> id;
        double temperature{0};
    };

    /////////////////////////////////////////////////////////////////////////////////
    // FixedHumidity
    /////////////////////////////////////////////////////////////////////////////////

    struct FixedHumidity
    {
        FixedHumidity() = default;
        FixedHumidity(const std::optional<size_t> & id, double humidity);
        explicit FixedHumidity(double humidity);

        std::optional<size_t> id;
        double humidity{0};
    };

    /////////////////////////////////////////////////////////////////////////////////
    // BoundaryConditionTimestep
    /////////////////////////////////////////////////////////////////////////////////
    struct BoundaryConditionTimestep
    {
        BoundaryConditionTimestep() = default;

        void loadFromXMLFile(const std::string & fileName);
        void loadFromXMLString(const std::string & xmlString);

        bool isLoadSuccesful() const;

        //! \brief In case loading failed this is showing error message
        [[nodiscard]] std::string errorMessage() const;

        std::string version;
        std::string name;

        ConvectionTimesteps convection;
        RadiationTimesteps radiation;

        std::vector<HeatFlux> heatFlux;
        std::vector<SolarRadiation> solarRadiation;
        std::vector<FixedTemperature> temperature;
        std::vector<FixedHumidity> humidity;

    private:
        std::string m_ErrorMessage;
    };

}   // namespace BCInputFileLibrary
