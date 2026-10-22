#include <algorithm>
#include <cctype>
#include <stdexcept>

#include "THMZ/ZipModule/ZipModule.hxx"

#include "CsvSeries.hxx"
#include "Epw.hxx"
#include "Import.hxx"

namespace TimeSeriesLibrary::Import
{
    namespace
    {
        constexpr auto weatherExtension{".epw"};
        constexpr auto csvExtension{".csv"};
        constexpr auto zipExtension{".zip"};

        std::string lowercaseExtension(const std::filesystem::path & path)
        {
            auto extension{path.extension().string()};
            std::ranges::transform(extension, extension.begin(), [](const unsigned char character) {
                return static_cast<char>(std::tolower(character));
            });
            return extension;
        }

        lbnl::ExpectedExt<ImportedDataset, std::string> importContent(const std::string & content,
                                                                      const std::string & extension,
                                                                      const std::string & datasetName,
                                                                      const Csv::StepChoice & stepChoice)
        {
            if(extension == weatherExtension)
            {
                return Epw::readFromString(content, datasetName).transform([](const Epw::ImportResult & result) {
                    return ImportedDataset{.data = result.data, .skippedColumns = {}};
                });
            }
            if(extension == csvExtension)
            {
                return Csv::readFromString(content, datasetName, stepChoice)
                  .transform([](const Csv::ImportResult & result) {
                      return ImportedDataset{.data = result.data, .skippedColumns = result.ignoredHeaders};
                  });
            }
            return lbnl::Unexpected<std::string>{"Unsupported entry type: " + datasetName + extension};
        }

        lbnl::ExpectedExt<ImportedDataset, std::string> importFile(const std::filesystem::path & file,
                                                                   const std::string & extension,
                                                                   const Csv::StepChoice & stepChoice)
        {
            if(extension == weatherExtension)
            {
                return Epw::read(file.string()).transform([](const Epw::ImportResult & result) {
                    return ImportedDataset{.data = result.data, .skippedColumns = {}};
                });
            }
            return Csv::read(file.string(), stepChoice).transform([](const Csv::ImportResult & result) {
                return ImportedDataset{.data = result.data, .skippedColumns = result.ignoredHeaders};
            });
        }

        lbnl::ExpectedExt<ImportOutcome, std::string> importZip(const std::filesystem::path & file,
                                                                const Csv::StepChoice & stepChoice)
        {
            try
            {
                return importDatasetsFromEntries(ThermZip::unzipFiles(file.string()), stepChoice);
            }
            catch(const std::runtime_error &)
            {
                return lbnl::Unexpected<std::string>{"The archive could not be read."};
            }
        }
    }   // namespace

    std::vector<std::string> supportedExtensions()
    {
        return {weatherExtension, csvExtension, zipExtension};
    }

    namespace
    {
        std::vector<std::string> weatherEntries(const std::map<std::string, std::string> & entries)
        {
            std::vector<std::string> names;
            for(const auto & [entryName, content] : entries)
            {
                if(lowercaseExtension(entryName) == weatherExtension)
                {
                    names.push_back(entryName);
                }
            }
            return names;
        }

        lbnl::ExpectedExt<Epw::ImportResult, std::string> readWeatherFromZip(const std::filesystem::path & file)
        {
            std::map<std::string, std::string> entries;
            try
            {
                entries = ThermZip::unzipFiles(file.string());
            }
            catch(const std::runtime_error &)
            {
                return lbnl::Unexpected<std::string>{"The archive could not be read."};
            }
            const auto names{weatherEntries(entries)};
            if(names.size() != 1U)
            {
                std::string listed;
                for(const auto & name : names)
                {
                    listed += (listed.empty() ? "" : ", ") + name;
                }
                return lbnl::Unexpected<std::string>{names.empty()
                                                       ? std::string{"The archive holds no EnergyPlus weather (*.epw) entry."}
                                                       : "The archive holds more than one weather file: " + listed};
            }
            return Epw::readFromString(entries.at(names.front()), std::filesystem::path{names.front()}.stem().string());
        }
    }   // namespace

    lbnl::ExpectedExt<Epw::ImportResult, std::string> readWeather(const std::filesystem::path & file)
    {
        const auto extension{lowercaseExtension(file)};
        if(extension == zipExtension)
        {
            return readWeatherFromZip(file);
        }
        if(extension == weatherExtension)
        {
            return Epw::read(file.string());
        }
        return lbnl::Unexpected<std::string>{"A weather file is an EnergyPlus weather file (*.epw) or a zip holding one."};
    }

    lbnl::ExpectedExt<ImportOutcome, std::string> importDatasets(const std::filesystem::path & file,
                                                                 const Csv::StepChoice & stepChoice)
    {
        const auto extension{lowercaseExtension(file)};
        if(extension == zipExtension)
        {
            return importZip(file, stepChoice);
        }
        if(extension == weatherExtension || extension == csvExtension)
        {
            return importFile(file, extension, stepChoice).transform([](ImportedDataset imported) {
                return ImportOutcome{.datasets = {std::move(imported)}, .failedEntries = {}};
            });
        }
        return lbnl::Unexpected<std::string>{"Unsupported file type: time series data is imported from "
                                             "EnergyPlus weather (*.epw), CSV (*.csv) or zipped weather "
                                             "(*.zip) files."};
    }

    lbnl::ExpectedExt<ImportOutcome, std::string>
      importDatasetsFromEntries(const std::map<std::string, std::string> & entries, const Csv::StepChoice & stepChoice)
    {
        ImportOutcome outcome;
        for(const auto & [entryName, content] : entries)
        {
            const std::filesystem::path entryPath{entryName};
            const auto extension{lowercaseExtension(entryPath)};
            if(extension != weatherExtension && extension != csvExtension)
            {
                continue;
            }
            const auto imported{importContent(content, extension, entryPath.stem().string(), stepChoice)};
            if(imported.has_value())
            {
                outcome.datasets.push_back(imported.value());
            }
            else
            {
                outcome.failedEntries.push_back(entryName + ": " + imported.error());
            }
        }
        if(outcome.datasets.empty())
        {
            return lbnl::Unexpected<std::string>{
              outcome.failedEntries.empty()
                ? std::string{"The archive contains no EnergyPlus weather (*.epw) or CSV (*.csv) entries."}
                : outcome.failedEntries.front()};
        }
        return outcome;
    }
}   // namespace TimeSeriesLibrary::Import
