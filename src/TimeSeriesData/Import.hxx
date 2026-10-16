#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include <lbnl/expected.hxx>

#include "Epw.hxx"
#include "TimeSeriesData.hxx"

namespace TimeSeriesLibrary::Import
{
    //! Datasets from a file a user points at, by its extension: an EnergyPlus weather file
    //! (.epw), a CSV of role-named columns (.csv), or a zip holding any number of either
    //! (weather files are commonly distributed zipped, an .epw beside its .ddy and .stat
    //! companions). The readers are Epw and Csv; this only decides which one a file gets.

    struct ImportedDataset
    {
        TimeSeriesData data;
        //! CSV columns whose header names no series role, so they were not imported. An
        //! EPW's unmapped fields are the standard's own and are not reported.
        std::vector<std::string> skippedColumns;
    };

    struct ImportOutcome
    {
        std::vector<ImportedDataset> datasets;
        //! Zip entries that looked importable and were not, as "<entry>: <reason>".
        std::vector<std::string> failedEntries;
    };

    //! The extensions importDatasets accepts, lower case with the dot.
    [[nodiscard]] std::vector<std::string> supportedExtensions();

    //! One weather file, with its station: an .epw, or a .zip holding exactly one (the
    //! way weather files are distributed, beside their .ddy and .stat companions). A zip
    //! with no weather file or with several is refused, naming them.
    [[nodiscard]] lbnl::ExpectedExt<Epw::ImportResult, std::string> readWeather(const std::filesystem::path & file);

    //! Every dataset the file holds. A zip with no importable entry, an unreadable file
    //! and an unsupported extension all fail with a message that says so.
    [[nodiscard]] lbnl::ExpectedExt<ImportOutcome, std::string> importDatasets(const std::filesystem::path & file);

    //! The zip case on its contents: entry name -> content, each .epw or .csv entry
    //! imported under its stem, every other entry ignored.
    [[nodiscard]] lbnl::ExpectedExt<ImportOutcome, std::string>
      importDatasetsFromEntries(const std::map<std::string, std::string> & entries);
}   // namespace TimeSeriesLibrary::Import
