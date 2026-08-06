#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "Definitions.hxx"

namespace BCLibrary
{
    class Tags
    {
    public:
        [[nodiscard]] std::string boundaryConditions() const;
        [[nodiscard]] std::string boundaryCondition() const;
        [[nodiscard]] std::string version() const;

        [[nodiscard]] std::string uuid() const;
        [[nodiscard]] std::string name() const;
        [[nodiscard]] std::string projectName() const;
        [[nodiscard]] std::string protectedTag() const;
        [[nodiscard]] std::string color() const;
        [[nodiscard]] std::string isIGUSurface() const;

        [[nodiscard]] std::string surfaceExchange() const;
        [[nodiscard]] std::string prescribedState() const;
        [[nodiscard]] std::string radiationSurface() const;

        [[nodiscard]] std::string constant() const;
        [[nodiscard]] std::string fromTimeSeries() const;

        [[nodiscard]] std::string model() const;
        [[nodiscard]] std::string convection() const;
        [[nodiscard]] std::string radiation() const;
        [[nodiscard]] std::string solar() const;
        [[nodiscard]] std::string flux() const;

        [[nodiscard]] std::string automaticEnclosure() const;
        [[nodiscard]] std::string manualEnclosure() const;
        [[nodiscard]] std::string blackBodyRadiation() const;
        [[nodiscard]] std::string fixedCoefficientRadiation() const;

        [[nodiscard]] std::string airTemperature() const;
        [[nodiscard]] std::string filmCoefficient() const;
        [[nodiscard]] std::string windSpeed() const;
        [[nodiscard]] std::string windDirection() const;
        [[nodiscard]] std::string temperature() const;
        [[nodiscard]] std::string emissivity() const;
        [[nodiscard]] std::string viewFactor() const;
        [[nodiscard]] std::string coefficient() const;
        [[nodiscard]] std::string relativeHumidity() const;
        [[nodiscard]] std::string irradiance() const;
        [[nodiscard]] std::string absorptance() const;
        [[nodiscard]] std::string isDefault() const;

    private:
        const std::string boundaryConditions_{"BoundaryConditions"};
        const std::string boundaryCondition_{"BoundaryCondition"};
        const std::string version_{"Version"};

        const std::string uuid_{"UUID"};
        const std::string name_{"Name"};
        const std::string projectName_{"ProjectName"};
        const std::string protectedTag_{"Protected"};
        const std::string color_{"Color"};
        const std::string isIGUSurface_{"IGUSurface"};

        const std::string surfaceExchange_{"SurfaceExchange"};
        const std::string prescribedState_{"PrescribedState"};
        const std::string radiationSurface_{"RadiationSurface"};

        const std::string constant_{"Constant"};
        const std::string fromTimeSeries_{"FromTimeSeries"};

        const std::string model_{"Model"};
        const std::string convection_{"Convection"};
        const std::string radiation_{"Radiation"};
        const std::string solar_{"Solar"};
        const std::string flux_{"Flux"};

        const std::string automaticEnclosure_{"AutomaticEnclosure"};
        const std::string manualEnclosure_{"ManualEnclosure"};
        const std::string blackBodyRadiation_{"BlackBody"};
        const std::string fixedCoefficientRadiation_{"FixedCoefficient"};

        const std::string airTemperature_{"AirTemperature"};
        const std::string filmCoefficient_{"FilmCoefficient"};
        const std::string windSpeed_{"WindSpeed"};
        const std::string windDirection_{"WindDirection"};
        const std::string temperature_{"Temperature"};
        const std::string emissivity_{"Emissivity"};
        const std::string viewFactor_{"ViewFactor"};
        const std::string coefficient_{"Coefficient"};
        const std::string relativeHumidity_{"RelativeHumidity"};
        const std::string irradiance_{"Irradiance"};
        const std::string absorptance_{"Absorptance"};
        const std::string isDefault_{"Default"};
    };

    [[nodiscard]] std::string convectionModelToString(ConvectionModel model);
    [[nodiscard]] ConvectionModel convectionModelFromString(std::string_view value);
    [[nodiscard]] std::vector<std::string> convectionModelStrings();
}   // namespace BCLibrary
