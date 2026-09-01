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
        std::optional<Source> airTemperature{};
        std::optional<Source> filmCoefficient{};
        std::optional<Source> windSpeed{};
        std::optional<Source> windDirection{};

        [[nodiscard]] friend bool operator==(const Convection & lhs, const Convection & rhs) = default;
    };

    //! The enclosure's radiation temperature is a value of its own, as it has been since
    //! version 7, where the Tr field stays editable for an auto grey body record. It is
    //! optional only so that libraries converted before the field existed keep loading;
    //! absent, the enclosure follows the convection air temperature, which is what every
    //! shipped record does anyway.
    struct AutomaticEnclosure
    {
        Source emissivity{Constant{0.9}};
        std::optional<Source> temperature{};

        [[nodiscard]] friend bool operator==(const AutomaticEnclosure & lhs, const AutomaticEnclosure & rhs) = default;
    };

    struct ManualEnclosure
    {
        [[nodiscard]] friend bool operator==(const ManualEnclosure & lhs, const ManualEnclosure & rhs) = default;
    };

    struct BlackBodyRadiation
    {
        Source temperature{Constant{0.0}};
        Source emissivity{Constant{0.9}};
        double viewFactor{1.0};

        [[nodiscard]] friend bool operator==(const BlackBodyRadiation & lhs, const BlackBodyRadiation & rhs) = default;
    };

    //! Merges the legacy steady-state Linearized radiation and the legacy transient Fixed
    //! radiation coefficient; they are the same physics.
    struct FixedCoefficientRadiation
    {
        Source temperature{Constant{0.0}};
        Source coefficient{Constant{0.0}};

        [[nodiscard]] friend bool operator==(const FixedCoefficientRadiation & lhs, const FixedCoefficientRadiation & rhs) = default;
    };

    using RadiationOptions =
      std::variant<AutomaticEnclosure, ManualEnclosure, BlackBodyRadiation, FixedCoefficientRadiation>;

    struct Solar
    {
        Source irradiance{Constant{0.0}};
        double absorptance{0.6};

        [[nodiscard]] friend bool operator==(const Solar & lhs, const Solar & rhs) = default;
    };

    //! Neumann surface exchange: subsumes the legacy steady Comprehensive and Simplified
    //! records and the legacy transient Neumann type record.
    struct SurfaceExchange
    {
        Source relativeHumidity{Constant{0.5}};
        std::optional<Convection> convection{};
        std::optional<RadiationOptions> radiation{};
        std::optional<Solar> solar{};
        std::optional<Source> flux{};

        [[nodiscard]] friend bool operator==(const SurfaceExchange & lhs, const SurfaceExchange & rhs) = default;
    };

    //! Dirichlet: the surface state itself is imposed. At least one input is expected;
    //! enforced by the loader and the GUI, not by the type.
    struct PrescribedState
    {
        std::optional<Source> temperature{};
        std::optional<Source> relativeHumidity{};

        [[nodiscard]] friend bool operator==(const PrescribedState & lhs, const PrescribedState & rhs) = default;
    };

    struct RadiationSurface
    {
        bool isDefault{false};
        double temperature{0};
        double emissivity{0};

        [[nodiscard]] friend bool operator==(const RadiationSurface & lhs, const RadiationSurface & rhs) = default;
    };

    //! Deliberately no exchange at all: the boundary is adiabatic and moisture-tight.
    //! A dedicated kind (rather than an empty SurfaceExchange) so intent is explicit -
    //! an empty exchange could equally be a half-edited record.
    struct NoExchange
    {
        [[nodiscard]] friend bool operator==(const NoExchange & lhs, const NoExchange & rhs) = default;
    };

    struct BoundaryCondition
    {
        std::string UUID{};
        std::string Name{"Default Name"};
        bool Protected{false};
        std::string Color{"0x0078D7"};
        std::variant<SurfaceExchange, PrescribedState, RadiationSurface, NoExchange> data{};

        std::optional<std::string> ProjectName{};

        //! Carried over from the steady-state library; glazing-system surfaces are treated
        //! specially regardless of calculation mode.
        bool isIGUSurface{false};

        [[nodiscard]] friend bool operator==(const BoundaryCondition & lhs, const BoundaryCondition & rhs) = default;
    };

    //! The set of series roles the record reads from an time series dataset, in stable
    //! first-appearance order. Empty result == steady-state capable.
    [[nodiscard]] std::vector<TimeSeriesLibrary::SeriesRole>
      requiredRoles(const BoundaryCondition & boundaryCondition);

    [[nodiscard]] bool isSteadyCapable(const BoundaryCondition & boundaryCondition);
}   // namespace BCLibrary
