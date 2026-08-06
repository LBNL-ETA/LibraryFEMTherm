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

    //! A scalar input is either a constant value or a reference to an environment channel
    //! role. A record whose inputs are all constants is a steady-state boundary condition
    //! and is also usable in transient runs, where its constants apply at every timestep.
    struct Constant
    {
        double value{0};
    };

    struct FromTimeSeries
    {
        TimeSeriesLibrary::ChannelRole role{TimeSeriesLibrary::ChannelRole::AirTemperature};
    };

    using Source = std::variant<Constant, FromTimeSeries>;

    [[nodiscard]] bool isConstant(const Source & source);
    [[nodiscard]] bool isFromTimeSeries(const Source & source);
}   // namespace BCLibrary
