#pragma once

#include <optional>
#include <string>
#include <vector>

namespace TimeSeriesLibrary
{
    //! Fixed vocabulary of series roles. The list is deliberately closed: both a boundary
    //! condition input and a dataset series name the same role, which is what makes their
    //! pairing automatic. New roles ship together with the engine model that consumes them.
    enum class SeriesRole
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
    struct Series
    {
        SeriesRole role{SeriesRole::AirTemperature};
        std::vector<double> values;

        [[nodiscard]] bool operator==(const Series & other) const = default;
    };

    struct TimeSeriesData
    {
        std::string UUID;
        std::string Name{"Default Name"};
        //! Set only on temporary records that entered the library from a project file during
        //! reconciliation; permanent library records leave it empty. Not part of the content
        //! hash — reconciliation metadata must not change the UUID.
        std::optional<std::string> ProjectName;
        bool Protected{false};
        std::optional<std::string> Description;
        //! Boundaries are drawn in the color of the series they read, not of the record
        //! they use (design decision: avoids duplicating records just to recolor). Not part
        //! of the content hash, like every other piece of display metadata.
        std::string Color{"0x1F6FB4"};
        //! Provenance in words, shown by the library list ("Imported",
        //! "Standard - interior"); absent on datasets made before the label existed, and
        //! the display falls back to "Imported". Metadata like Color - not part of the
        //! content hash.
        std::optional<std::string> Source;
        std::vector<Series> series;

        [[nodiscard]] bool operator==(const TimeSeriesData & other) const = default;
    };

    //! Number of steps the dataset provides: the shortest series length (series are
    //! expected to be equal length; the minimum is the defensive answer).
    [[nodiscard]] size_t steps(const TimeSeriesData & data);

    [[nodiscard]] bool hasRole(const TimeSeriesData & data, SeriesRole role);

    [[nodiscard]] std::optional<std::vector<double>> valuesForRole(const TimeSeriesData & data,
                                                                   SeriesRole role);

    [[nodiscard]] std::vector<SeriesRole> providedRoles(const TimeSeriesData & data);
}   // namespace TimeSeriesLibrary
