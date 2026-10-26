#pragma once

#include <optional>
#include <string>
#include <variant>
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
    //!
    //! The steps are one length in seconds for every step, or one length per step, never an
    //! empty list. The file writes the single length, or the first of the list, as
    //! StepSeconds and the list, when there is one, as StepLengths: an evenly stepped axis
    //! is stored and hashed exactly as it was before uneven steps existed.
    //!
    //! Month, day and hour count from 1, as EnergyPlus's weather files count them; they are
    //! the one place the library does not count from 0. Hour h is the hour from (h - 1):00 to
    //! h:00, so hour 1 is the first hour of the day, and the first step starts `minute`
    //! minutes into it: hour 1, minute 0 is 00:00. Minute counts from 0.
    using StepSeconds = std::variant<double, std::vector<double>>;

    //! The months by name, numbered as TimeAxis::month counts them, from 1: a caller may state
    //! a month either way.
    enum class Month : size_t
    {
        January = 1U,
        February,
        March,
        April,
        May,
        June,
        July,
        August,
        September,
        October,
        November,
        December
    };

    [[nodiscard]] constexpr size_t monthNumber(const Month month)
    {
        return static_cast<size_t>(month);
    }

    struct TimeAxis
    {
        size_t month{1U};    //!< 1..12
        size_t day{1U};      //!< 1..31
        size_t hour{1U};     //!< 1..24, the hour as EnergyPlus numbers it: hour 1 is 00:00 to 01:00
        size_t minute{0U};   //!< 0..59, into that hour
        StepSeconds stepSeconds{3600.0};

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

    //! Seconds from the start of the nominal year to the given calendar position. The hour
    //! here is the CLOCK hour, 0..23, as a stamp reads; an axis's hour is that plus one.
    [[nodiscard]] double
      secondsIntoYear(size_t month, size_t day, size_t clockHour, size_t minute, size_t second = 0U);

    //! The clock hour, 0..23, at which the axis's hour begins: hour 1 begins at 0. An hour
    //! outside 1..24, as a file may state it, is clamped.
    [[nodiscard]] size_t clockHour(const TimeAxis & axis);

    [[nodiscard]] double startSecondsIntoYear(const TimeAxis & axis);

    //! Why a calendar position cannot be where an axis starts, or nothing when it can: month
    //! 1..12, day within its month of the nominal year (so no 29 February), hour 1..24 and
    //! minute 0..59. The library's own readers clamp what a file states; this is for a
    //! position stated by hand, which is refused rather than clamped.
    [[nodiscard]] std::optional<std::string>
      calendarPositionError(size_t month, size_t day, size_t hour, size_t minute);

    //! The calendar position `secondsIntoYear` into the nominal year, wrapping at either end
    //! and rounded to the minute, which is as fine as an axis is stated. The step is hourly.
    [[nodiscard]] TimeAxis axisAt(double secondsIntoYear);

    //! Whether the axis states one length for every step.
    [[nodiscard]] bool evenlyStepped(const TimeAxis & axis);

    //! One length per step, `steps` of them, even or not. An uneven axis stating fewer
    //! lengths than steps repeats its last one.
    [[nodiscard]] std::vector<double> stepLengths(const TimeAxis & axis, size_t steps);

    //! The length of step `index`; step 0's is what the file writes as StepSeconds.
    [[nodiscard]] double stepSecondsAt(const TimeAxis & axis, size_t index);

    //! The steps as an axis states them: one length when every length is the same, the list
    //! otherwise. Lengths within half a second count as the same; no lengths, the hourly
    //! default.
    [[nodiscard]] StepSeconds stepSecondsOf(const std::vector<double> & lengths);

    //! The axis `steps` rows later: the start moved through the first `steps` steps of the
    //! nominal year, wrapping at its end, the remaining steps kept. Rounded to the minute.
    [[nodiscard]] TimeAxis axisAfter(const TimeAxis & axis, size_t steps);

    //! "1 Jan 00:00"
    [[nodiscard]] std::string startLabel(const TimeAxis & axis);

    //! "1 h", "30 min", "90 s" - whichever unit divides the step exactly; "15 min to 2 h"
    //! for uneven steps.
    [[nodiscard]] std::string stepLabel(const TimeAxis & axis);

    //! "1 Jan 00:00, step 1 h, 8760 rows", or "..., steps 15 min to 2 h, ..." when uneven.
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

    //! What a dataset holds that a run does not refuse but that may not be what was meant.
    //! Nothing is changed and the run goes ahead, taking as many steps as the shortest
    //! series has values.
    enum class DatasetWarningKind
    {
        SeriesLengthsDiffer,   //!< the series have different numbers of values; the shortest sets the steps
        StepLengthsDiffer      //!< the axis lists a different number of step lengths than there are steps
    };

    //! One warning, as facts: `steps` is what a run takes, `stated` the count that differs
    //! from it (the longest series, or the number of step lengths listed). describeWarning
    //! words it.
    struct DatasetWarning
    {
        DatasetWarningKind kind{DatasetWarningKind::SeriesLengthsDiffer};
        size_t steps{0U};
        size_t stated{0U};

        [[nodiscard]] bool operator==(const DatasetWarning & other) const = default;
    };

    //! Every warning the dataset raises, none when it is as a run reads it.
    [[nodiscard]] std::vector<DatasetWarning> datasetWarnings(const TimeSeriesData & data);

    //! LibraryFEMTherm's wording of one warning, the default; a caller wanting other words
    //! builds them from the warning's fields.
    [[nodiscard]] std::string describeWarning(const TimeSeriesData & data, const DatasetWarning & warning);

    //! Every warning the dataset raises, each in LibraryFEMTherm's words; none when there is
    //! nothing to say.
    [[nodiscard]] std::vector<std::string> describeWarnings(const TimeSeriesData & data);
}   // namespace TimeSeriesLibrary
