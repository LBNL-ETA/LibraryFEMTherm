#include "TimestepFiles.hxx"

#include <algorithm>
#include <filesystem>
#include <optional>
#include <utility>

#include "Common/ParseNumber.hxx"
#include "Legacy/Step1/BCTransient/Timestep.hxx"
#include "Legacy/Step1/Converters/Converters.hxx"
#include "THMZ/Model/DB.hxx"
#include "THMZ/ZipModule/ZipModule.hxx"
#include "TimeSeriesData/ContentHash.hxx"
#include "TimeSeriesData/DB.hxx"

namespace ThermFile::Migration::TimestepFiles
{
    namespace
    {
        using Entries = std::map<std::string, std::string>;
        using Dataset = TimeSeriesLibrary::TimeSeriesData;

        //! A value the current model format no longer carries, read from the raw Model.xml.
        std::optional<double> legacyNumber(const std::string & modelText, const std::string & tag)
        {
            const std::string opening{"<" + tag + ">"};
            const auto start{modelText.find(opening)};
            if(start == std::string::npos)
            {
                return std::nullopt;
            }
            const auto * const first{modelText.data() + start + opening.size()};
            // Common::parseDouble, not std::from_chars: older Apple standard libraries have no
            // from_chars for double.
            const auto parsed{Common::parseDouble(first, modelText.data() + modelText.size())};
            return parsed.has_value() ? std::optional{parsed->value} : std::nullopt;
        }

        std::string fileContent(const Entries & entries, const std::string & fileName)
        {
            // Older archives keep the files in the archive root.
            for(const auto & name : {ThermZip::TimestepFilesDir + "/" + fileName, fileName})
            {
                if(const auto found{entries.find(name)}; found != entries.end())
                {
                    return found->second;
                }
            }
            return {};
        }

        void applyModelClock(Dataset & dataset, const std::string & modelText)
        {
            if(const auto step{legacyNumber(modelText, "TimeStep")}; step.has_value() && step.value() > 0.0)
            {
                dataset.axis.stepSeconds = step.value();
            }
            const auto count{legacyNumber(modelText, "NumberOfTimesteps")};
            if(count.has_value() && count.value() >= 1.0)
            {
                const auto rows{static_cast<std::size_t>(count.value())};
                for(auto & series : dataset.series)
                {
                    series.values.resize((std::min)(rows, series.values.size()));
                }
            }
        }

        std::optional<Dataset> convertedFile(const std::string & fileName,
                                             const std::string & content,
                                             const std::string & modelText)
        {
            BCInputFileLibrary::BoundaryConditionTimestep timestep;
            timestep.loadFromXMLString(content);
            if(!timestep.isLoadSuccesful())
            {
                return std::nullopt;
            }
            auto dataset{BCLibrary::environmentFromTimestep(
              timestep, std::filesystem::path(fileName).stem().string())};
            if(dataset.series.empty())
            {
                return std::nullopt;
            }
            dataset.Source = "Timestep file";
            applyModelClock(dataset, modelText);
            dataset.UUID = TimeSeriesLibrary::contentUuid(dataset);
            return dataset;
        }

        //! A plain text search, so ordinary files pay nothing: the tag is written empty on every
        //! boundary of a model that has no timestep files.
        bool namesTimestepFile(const std::string & modelText)
        {
            const std::string opening{"<TimestepFileName>"};
            for(auto position{modelText.find(opening)}; position != std::string::npos;
                position = modelText.find(opening, position + opening.size()))
            {
                const auto next{position + opening.size()};
                if(next < modelText.size() && modelText[next] != '<')
                {
                    return true;
                }
            }
            return false;
        }

        //! The boundary's timestep file, when it is still waiting for a dataset.
        std::optional<std::string> pendingFile(const ThermFile::Boundary & boundary)
        {
            const auto & transient{boundary.transientRecordData};
            if(boundary.timeSeriesUUID.has_value() || !transient.has_value()
               || transient->transientFileName.empty())
            {
                return std::nullopt;
            }
            return transient->transientFileName;
        }

        //! The datasets of a transient model's pending files, by file name. THERM wrote these
        //! files into steady-state models too, where nothing reads them.
        std::map<std::string, Dataset> datasetsByFile(const Entries & entries, const ThermFile::ThermModel & model)
        {
            std::map<std::string, Dataset> datasets;
            if(model.properties.calculationOptions.calculationMode != ThermFile::CalculationMode::cmTransient)
            {
                return datasets;
            }
            const auto modelText{ThermZip::findEntry(entries, ThermZip::ModelFileName)};
            for(const auto & boundary : model.boundaryConditions)
            {
                const auto fileName{pendingFile(boundary)};
                if(!fileName.has_value() || datasets.contains(fileName.value()))
                {
                    continue;
                }
                const auto content{fileContent(entries, fileName.value())};
                if(content.empty())
                {
                    continue;
                }
                if(auto dataset{convertedFile(fileName.value(), content, modelText)}; dataset.has_value())
                {
                    datasets.emplace(fileName.value(), std::move(dataset.value()));
                }
            }
            return datasets;
        }
    }   // namespace

    std::map<std::string, std::string> withConvertedFiles(std::map<std::string, std::string> entries)
    {
        const auto modelText{ThermZip::findEntry(entries, ThermZip::ModelFileName)};
        if(!namesTimestepFile(modelText))
        {
            return entries;
        }
        const auto model{ThermFile::thermModelFromString(modelText)};
        if(!model.has_value())
        {
            return entries;
        }
        for(const auto & [fileName, dataset] : datasetsByFile(entries, model.value()))
        {
            TimeSeriesLibrary::DB single;
            single.add(dataset);
            entries[ThermZip::timeSeriesEntryName(dataset.UUID)] = single.toString();
        }
        return entries;
    }

    ThermFile::ThermModel applyToModel(const std::map<std::string, std::string> & entries,
                                       ThermFile::ThermModel model)
    {
        if(!std::ranges::any_of(model.boundaryConditions,
                                [](const ThermFile::Boundary & boundary) { return pendingFile(boundary).has_value(); }))
        {
            return model;
        }
        const auto datasets{datasetsByFile(entries, model)};
        for(auto & boundary : model.boundaryConditions)
        {
            const auto fileName{pendingFile(boundary)};
            if(!fileName.has_value())
            {
                continue;
            }
            const auto found{datasets.find(fileName.value())};
            if(found != datasets.end() && entries.contains(ThermZip::timeSeriesEntryName(found->second.UUID)))
            {
                boundary.timeSeriesUUID = found->second.UUID;
                boundary.transientRecordData->transientFileName.clear();
            }
        }
        return model;
    }
}   // namespace ThermFile::Migration::TimestepFiles
