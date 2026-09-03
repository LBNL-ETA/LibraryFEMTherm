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

    //! One time series: a role plus its values. Rows are indexed; the dataset's TimeAxis says
    //! where row zero sits in the year and how far apart the rows are.
    struct Series
    {
        SeriesRole role{SeriesRole::AirTemperature};
        std::vector<double> values;

        [[nodiscard]] bool operator==(const Series & other) const = default;
    };

    //! Where the first row sits in the year and how far apart the rows are. The year is
    //! deliberately absent: typical-year weather files stitch every month from a different
    //! source year, so a literal year could never match between two stations, or between a
    //! weather file and an interior generated from it. Two datasets line up when their axes
    //! and their step counts are equal; nothing is ever interpolated. Datasets written before
    //! the axis existed read as 1 January 00:00 with hourly steps, which is what they were.
    struct TimeAxis
    {
        size_t month{1U};    //!< 1..12
        size_t day{1U};      //!< 1..31
        size_t hour{0U};     //!< 0..23, the start of the first interval
        size_t minute{0U};   //!< 0..59
        double stepSeconds{3600.0};

        [[nodiscard]] bool operator==(const TimeAxis & other) const = default;
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
        //! Part of the content hash: the same values a month apart are different data.
        TimeAxis axis;
        std::vector<Series> series;

        [[nodiscard]] bool operator==(const TimeSeriesData & other) const = default;
    };

    //! Number of steps the dataset provides: the shortest series length (series are
    //! expected to be equal length; the minimum is the defensive answer).
    [[nodiscard]] size_t steps(const TimeSeriesData & data);

    //! The nominal year every axis is measured in has no leap day, like the weather files.
    [[nodiscard]] constexpr double secondsPerNominalYear()
    {
        return 365.0 * 86400.0;
    }

    //! Seconds from the start of the nominal year to the given calendar position.
    [[nodiscard]] double
      secondsIntoYear(size_t month, size_t day, size_t hour, size_t minute, size_t second = 0U);

    [[nodiscard]] double startSecondsIntoYear(const TimeAxis & axis);

    //! "1 Jan 00:00"
    [[nodiscard]] std::string startLabel(const TimeAxis & axis);

    //! "1 h", "30 min", "90 s" - whichever unit divides the step exactly.
    [[nodiscard]] std::string stepLabel(const TimeAxis & axis);

    //! "1 Jan 00:00, step 1 h, 8760 rows"
    [[nodiscard]] std::string describeAxis(const TimeSeriesData & data);

    //! Same axis and same step count: the two can drive one simulation.
    [[nodiscard]] bool aligned(const TimeSeriesData & lhs, const TimeSeriesData & rhs);

    //! Empty when every dataset lines up with the first; otherwise one "<name>: <axis>"
    //! line per dataset, so a message can show the whole picture rather than one culprit.
    [[nodiscard]] std::vector<std::string>
      alignmentIssues(const std::vector<TimeSeriesData> & datasets);

    [[nodiscard]] bool hasRole(const TimeSeriesData & data, SeriesRole role);

    [[nodiscard]] std::optional<std::vector<double>> valuesForRole(const TimeSeriesData & data,
                                                                   SeriesRole role);

    [[nodiscard]] std::vector<SeriesRole> providedRoles(const TimeSeriesData & data);
}   // namespace TimeSeriesLibrary
