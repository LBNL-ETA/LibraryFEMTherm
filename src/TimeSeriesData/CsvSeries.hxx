#pragma once

#include <optional>
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
    //! How rows become steps is the caller's choice (see StepChoice); stated as nothing,
    //! AutoStep, the file's columns decide. Two clock columns are recognised: a
    //! calendar Time column (Time, Date, Datetime, Timestamp - the year is read and
    //! dropped) and a Timestep column (Timestep, Step, Step Seconds, dt) of lengths in
    //! seconds. A stamp is the END of its row's step under every rule. Steps may be
    //! uneven; the dataset's axis then states them one by one.

    //! Which column states the steps.
    enum class StepColumn
    {
        //! The Time column: every row is a step ending at its time, as EnergyPlus reads a
        //! weather file, and n rows make n steps. The stamps give every length but the
        //! first, which is the distance from the stated start, else the first entry of a
        //! Timestep column that agrees with the stamps everywhere else, else, assumed, the
        //! gap to the second row. ImportResult::firstStep says which.
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

    //! The columns decide: the Time column when the file has one, else the Timestep column,
    //! else every row one step of FixedStep's default, an hour. The choice when none is
    //! stated.
    struct AutoStep
    {
        [[nodiscard]] bool operator==(const AutoStep & other) const = default;
    };

    //! AutoStep first, so a choice made without a value is the automatic one.
    using StepChoice = std::variant<AutoStep, FixedStep, StepColumn>;

    //! Where the length of the first step came from: the caller (a stated start, or a
    //! fixed step), the file's Timestep column, or an assumption, the gap between the
    //! first two stamps. Only the Time rule ever assumes.
    enum class FirstStep
    {
        Stated,
        FromColumn,
        Assumed
    };

    struct ImportResult
    {
        TimeSeriesData data;
        std::vector<std::string> ignoredHeaders;
        bool hadTimeColumn{false};
        bool hadTimestepColumn{false};
        FirstStep firstStep{FirstStep::FromColumn};
    };

    //! One line on the first step, in LibraryFEMTherm's words: its length and where the
    //! length came from, so a reader can see what the file did not state.
    [[nodiscard]] std::string describeFirstStep(const ImportResult & result);

    //! Spelled-out spelling of a role (AirTemperature -> "Air Temperature"): the
    //! header written on export and the second spelling accepted on import.
    [[nodiscard]] std::string displayHeaderForRole(SeriesRole role);

    //! Every role as a table, a heading row and one row per role: the role's name and the
    //! CSV header that writes it out, columns aligned. A CSV file may use either spelling.
    [[nodiscard]] std::string describeRoles();

    //! The imported dataset's UUID is the deterministic content hash, so importing the
    //! same file twice converges to one dataset. Blank cells mid-run carry the last good
    //! reading forward, as the weather reader does - a gap is not a zero. A column the
    //! choice asks for and the file lacks is an error, as is a time that does not follow
    //! the previous one or a timestep that is not positive.
    //!
    //! `start` is when the run begins, written as the Time column writes a time
    //! ("2026-01-01 00:00"). It is read where the file does not say: under the Time rule
    //! it sets the first step's length, and without a Time column it places the run,
    //! which otherwise starts on 1 January 00:00. Beside a Time column, the Timestep and
    //! fixed-step rules place the start from the first stamp and do not read it. The
    //! year is dropped, as in the stamps: a start later in the year than the first stamp
    //! is the year before, so "2025-12-31 23:00" begins a file whose first stamp is
    //! 1 January 00:00.
    [[nodiscard]] lbnl::ExpectedExt<ImportResult, std::string>
      readFromString(const std::string & content,
                     const std::string & datasetName,
                     const StepChoice & stepChoice = {},
                     const std::optional<std::string> & start = std::nullopt);

    //! Reads the file and imports it under its stem as the dataset name.
    [[nodiscard]] lbnl::ExpectedExt<ImportResult, std::string>
      read(const std::string & fileName,
           const StepChoice & stepChoice = {},
           const std::optional<std::string> & start = std::nullopt);

    //! Export in the shape read() accepts: a Time column stamping the end of each step in
    //! a fixed nominal year, a Timestep column of the step lengths in seconds, then one
    //! column per role under its spelled-out name. Read back under either column it is
    //! the same dataset: under the Time rule the Timestep column, agreeing with the
    //! stamps, supplies the first step's length.
    [[nodiscard]] std::string writeToString(const TimeSeriesData & data);
    void writeToFile(const TimeSeriesData & data, const std::string & fileName);
}   // namespace TimeSeriesLibrary::Csv
