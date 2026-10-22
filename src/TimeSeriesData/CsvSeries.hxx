#pragma once

#include <string>
#include <variant>
#include <vector>

#include <lbnl/expected.hxx>

#include "TimeSeriesData.hxx"

namespace TimeSeriesLibrary::Csv
{
    //! CSV is the interchange format for time series datasets: import with header
    //! matching and export back, so a dataset is never trapped in the library format.
    //!
    //! A column is imported when its header names a series role - either the stored
    //! spelling (AirTemperature) or the spelled-out one (Air Temperature), with case and
    //! spacing ignored - and is otherwise ignored and reported. No keyword guessing: a
    //! header either is a role name or it is not.
    //!
    //! How rows become steps is the caller's choice, stated on every read, because the
    //! file alone cannot say (see StepChoice). Two clock columns are recognised: a
    //! calendar Time column (Time, Date, Datetime, Timestamp - the year is read and
    //! dropped) and a Timestep column (Timestep, Step, Step Seconds, dt) of lengths in
    //! seconds. A stamp is the END of its row's step under every rule. Steps may be
    //! uneven; the dataset's axis then states them one by one.

    //! Which column states the steps.
    enum class StepColumn
    {
        //! The Time column: a row's value belongs to the step ending at its time, so the
        //! first row is only the start of the run and n rows make n-1 steps. This mirrors
        //! how EnergyPlus reads a weather file; it is the rule most likely to be revisited.
        Time,
        //! The Timestep column: row i is step i, of the length the row states. A Time
        //! column, if present, places the start one step before the first stamp.
        Timestep
    };

    //! Every row is one step of this many seconds. A Time column, if present, places the
    //! start one step before the first stamp, as under StepColumn::Timestep.
    struct FixedStep
    {
        double seconds{3600.0};

        [[nodiscard]] bool operator==(const FixedStep & other) const = default;
    };

    using StepChoice = std::variant<FixedStep, StepColumn>;

    struct ImportResult
    {
        TimeSeriesData data;
        std::vector<std::string> ignoredHeaders;
        bool hadTimeColumn{false};
        bool hadTimestepColumn{false};
        //! Under StepColumn::Time the first row was the start, not a step: its values
        //! were dropped.
        bool firstRowWasStart{false};
    };

    //! Spelled-out spelling of a role (AirTemperature -> "Air Temperature"): the
    //! header written on export and the second spelling accepted on import.
    [[nodiscard]] std::string displayHeaderForRole(SeriesRole role);

    //! The imported dataset's UUID is the deterministic content hash, so importing the
    //! same file twice converges to one dataset. Blank cells mid-run carry the last good
    //! reading forward, as the weather reader does - a gap is not a zero. A column the
    //! choice asks for and the file lacks is an error, as is a time that does not follow
    //! the previous one or a timestep that is not positive.
    [[nodiscard]] lbnl::ExpectedExt<ImportResult, std::string>
      readFromString(const std::string & content, const std::string & datasetName, const StepChoice & stepChoice);

    //! Reads the file and imports it under its stem as the dataset name.
    [[nodiscard]] lbnl::ExpectedExt<ImportResult, std::string> read(const std::string & fileName,
                                                                    const StepChoice & stepChoice);

    //! Export in the shape read() accepts: a Time column stamping the end of each step in
    //! a fixed nominal year, a Timestep column of the step lengths in seconds, then one
    //! column per role under its spelled-out name. Read back under StepColumn::Timestep
    //! it is the same dataset; under StepColumn::Time the first row becomes the start and
    //! the dataset is one step shorter.
    [[nodiscard]] std::string writeToString(const TimeSeriesData & data);
    void writeToFile(const TimeSeriesData & data, const std::string & fileName);
}   // namespace TimeSeriesLibrary::Csv
