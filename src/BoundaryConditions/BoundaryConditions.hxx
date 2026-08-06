#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "Definitions.hxx"

namespace BCLibrary
{
    //! Convection component. Which inputs are meaningful depends on the model: the fixed
    //! model uses air temperature and film coefficient; correlation models use air
    //! temperature plus wind speed and direction as the correlation requires.
    struct Convection
    {
        ConvectionModel model{ConvectionModel::Fixed_Convection_Coefficient};
        std::optional<Source> airTemperature;
        std::optional<Source> filmCoefficient;
        std::optional<Source> windSpeed;
        std::optional<Source> windDirection;
    };

    struct AutomaticEnclosure
    {
        Source emissivity{Constant{0.9}};
    };

    struct ManualEnclosure
    {};

    struct BlackBodyRadiation
    {
        Source temperature{Constant{0.0}};
        Source emissivity{Constant{0.9}};
        double viewFactor{1.0};
    };

    //! Merges the legacy steady-state Linearized radiation and the legacy transient Fixed
    //! radiation coefficient; they are the same physics.
    struct FixedCoefficientRadiation
    {
        Source temperature{Constant{0.0}};
        Source coefficient{Constant{0.0}};
    };

    using RadiationOptions =
      std::variant<AutomaticEnclosure, ManualEnclosure, BlackBodyRadiation, FixedCoefficientRadiation>;

    struct Solar
    {
        Source irradiance{Constant{0.0}};
        double absorptance{0.6};
    };

    //! Neumann surface exchange: subsumes the legacy steady Comprehensive and Simplified
    //! records and the legacy transient Neumann type record.
    struct SurfaceExchange
    {
        Source relativeHumidity{Constant{0.5}};
        std::optional<Convection> convection;
        std::optional<RadiationOptions> radiation;
        std::optional<Solar> solar;
        std::optional<Source> flux;
    };

    //! Dirichlet: the surface state itself is imposed. At least one input is expected;
    //! enforced by the loader and the GUI, not by the type.
    struct PrescribedState
    {
        std::optional<Source> temperature;
        std::optional<Source> relativeHumidity;
    };

    struct RadiationSurface
    {
        bool isDefault{false};
        double temperature{0};
        double emissivity{0};
    };

    struct BoundaryCondition
    {
        std::string UUID;
        std::string Name{"Default Name"};
        bool Protected{false};
        std::string Color{"0x0078D7"};
        std::variant<SurfaceExchange, PrescribedState, RadiationSurface> data;

        std::optional<std::string> ProjectName;

        //! Carried over from the steady-state library; glazing-system surfaces are treated
        //! specially regardless of calculation mode.
        bool isIGUSurface{false};
    };

    //! The set of channel roles the record reads from an time series dataset, in stable
    //! first-appearance order. Empty result == steady-state capable.
    [[nodiscard]] std::vector<TimeSeriesLibrary::ChannelRole>
      requiredRoles(const BoundaryCondition & boundaryCondition);

    [[nodiscard]] bool isSteadyCapable(const BoundaryCondition & boundaryCondition);
}   // namespace BCLibrary
