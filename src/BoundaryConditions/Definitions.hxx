#pragma once

#include <variant>

#include "TimeSeriesData/TimeSeriesData.hxx"

namespace BCLibrary
{
    //! Convection correlations are hard-coded engine models; there is deliberately no
    //! user-defined model. The set matches the legacy transient library one-to-one.
    enum class ConvectionModel
    {
        Fixed_Convection_Coefficient,
        ASHRAE_NFRC_Inside,
        ASHRAE_NFRC_Outside,
        TARP,
        Yazdanian_Klems,
        Kimura,
        Montazeri
    };

    //! A scalar input is either a constant value or a reference to an environment series
    //! role. A record whose inputs are all constants is a steady-state boundary condition
    //! and is also usable in transient runs, where its constants apply at every timestep.
    //!
    //! Both convert implicitly, so a Source is written as what it is: `.humidity = 0.8`,
    //! `.airTemperature = SeriesRole::AirTemperature`. Neither conversion can be mistaken for
    //! the other: a number is only ever a Constant, a role only ever a FromTimeSeries.
    struct Constant
    {
        // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
        constexpr Constant(const double constantValue = 0.0) : value{constantValue}
        {}

        double value{0};

        [[nodiscard]] friend bool operator==(const Constant & lhs, const Constant & rhs) = default;
    };

    struct FromTimeSeries
    {
        // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
        constexpr FromTimeSeries(const TimeSeriesLibrary::SeriesRole seriesRole = TimeSeriesLibrary::SeriesRole::AirTemperature) :
            role{seriesRole}
        {}

        TimeSeriesLibrary::SeriesRole role{TimeSeriesLibrary::SeriesRole::AirTemperature};

        [[nodiscard]] friend bool operator==(const FromTimeSeries & lhs, const FromTimeSeries & rhs) = default;
    };

    using Source = std::variant<Constant, FromTimeSeries>;

    [[nodiscard]] bool isConstant(const Source & source);
    [[nodiscard]] bool isFromTimeSeries(const Source & source);
}   // namespace BCLibrary
