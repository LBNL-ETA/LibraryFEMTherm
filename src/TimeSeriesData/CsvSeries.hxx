#pragma once

#include <string>
#include <vector>

#include <lbnl/expected.hxx>

#include "TimeSeriesData.hxx"

namespace TimeSeriesLibrary::Csv
{
    //! CSV is the interchange format for time series datasets: import with header
    //! matching and export back, so a dataset is never trapped in the library format.
    //!
    //! The behavior follows the interactive mockup's csv_series.py: a column is
    //! imported when its header names a channel role - either the stored spelling
    //! (AirTemperature) or the spelled-out one (Air Temperature), with case and
    //! spacing ignored - and is otherwise ignored and reported. No keyword guessing:
    //! a header either is a role name or it is not. A recognized calendar time
    //! column is acknowledged but skipped, because datasets carry no time axis of
    //! their own (rows are implicitly indexed; step duration is a simulation
    //! setting in the model file).
    struct ImportResult
    {
        TimeSeriesData data;
        std::vector<std::string> ignoredHeaders;
        bool hadTimeColumn{false};
    };

    //! Spelled-out spelling of a role (AirTemperature -> "Air Temperature"): the
    //! header written on export and the second spelling accepted on import.
    [[nodiscard]] std::string displayHeaderForRole(ChannelRole role);

    //! The imported dataset's UUID is the deterministic content hash, so importing
    //! the same file twice converges to one dataset. Blank cells mid-run carry the
    //! last good reading forward, as the weather reader does - a gap is not a zero.
    [[nodiscard]] lbnl::ExpectedExt<ImportResult, std::string>
      readFromString(const std::string & content, const std::string & datasetName);

    //! Reads the file and imports it under its stem as the dataset name.
    [[nodiscard]] lbnl::ExpectedExt<ImportResult, std::string> read(const std::string & fileName);

    //! Export in the shape read() accepts: a calendar time column on a nominal
    //! hourly axis, then one column per role under its spelled-out name.
    [[nodiscard]] std::string writeToString(const TimeSeriesData & data);
    void writeToFile(const TimeSeriesData & data, const std::string & fileName);
}   // namespace TimeSeriesLibrary::Csv
