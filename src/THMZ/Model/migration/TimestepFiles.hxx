#pragma once

#include <map>
#include <string>

#include "THMZ/Model/THMX.hxx"

//! The per-boundary timestep files of earlier THERM versions, turned into time series datasets
//! when a transient archive is opened. Best effort: a file that is missing or cannot be read
//! leaves its boundary as it was, and the simulation start names what is left to assign.
namespace ThermFile::Migration::TimestepFiles
{
    //! \brief The archive's entries, with a "time series/<uuid>" entry added for each timestep
    //! file a boundary of a transient model still needs.
    //!
    //! Runs right after unzipping, so the datasets reach every reader as ordinary embedded
    //! datasets. Rows past the model's number of timesteps are dropped, because the old run
    //! stopped there, and the step comes from the model: the old files state neither.
    //!
    //! This runs on every archive opened, so a model that names no timestep file is passed
    //! straight through: taken by value, an archive is moved in and out, never copied.
    [[nodiscard]] std::map<std::string, std::string>
      withConvertedFiles(std::map<std::string, std::string> entries);

    //! \brief Returns the model with each boundary bound to the dataset of its timestep file.
    //! A boundary whose dataset is not in the entries keeps its legacy file name.
    [[nodiscard]] ThermFile::ThermModel applyToModel(const std::map<std::string, std::string> & entries,
                                                     ThermFile::ThermModel model);
}   // namespace ThermFile::Migration::TimestepFiles
