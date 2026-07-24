#pragma once

#include <optional>
#include <string>
#include <vector>

namespace EnvironmentDataLibrary
{
    //! Fixed vocabulary of channel roles. The list is deliberately closed: both a boundary
    //! condition input and a dataset channel name the same role, which is what makes their
    //! pairing automatic. New roles ship together with the engine model that consumes them.
    enum class ChannelRole
    {
        AirTemperature,
        RelativeHumidity,
        WindSpeed,
        WindDirection,
        SolarIrradiance,
        HeatFlux,
        ConvectiveCoefficient,
        RadiantTemperature,
        RadiativeCoefficient,
        Emissivity,
        PrescribedTemperature,
        PrescribedHumidity
    };

    //! One time series: a role plus its values. Rows are implicitly indexed; step duration is
    //! a simulation setting in the model file, not a dataset property.
    struct Channel
    {
        ChannelRole role{ChannelRole::AirTemperature};
        std::vector<double> values;
    };

    struct EnvironmentData
    {
        std::string UUID;
        std::string Name{"Default Name"};
        bool Protected{false};
        std::optional<std::string> Description;
        std::vector<Channel> channels;
    };

    //! Number of steps the dataset provides: the shortest channel length (channels are
    //! expected to be equal length; the minimum is the defensive answer).
    [[nodiscard]] size_t steps(const EnvironmentData & data);

    [[nodiscard]] bool hasRole(const EnvironmentData & data, ChannelRole role);

    [[nodiscard]] std::optional<std::vector<double>> valuesForRole(const EnvironmentData & data,
                                                                   ChannelRole role);

    [[nodiscard]] std::vector<ChannelRole> providedRoles(const EnvironmentData & data);
}   // namespace EnvironmentDataLibrary
