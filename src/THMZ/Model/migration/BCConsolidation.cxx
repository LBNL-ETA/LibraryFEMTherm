#include <filesystem>

#include <lbnl/algorithm.hxx>

#include "BCConsolidation.hxx"

#include "Legacy/Step1/BCSteadyState/DB.hxx"
#include "Legacy/Step1/BCTransient/Timestep.hxx"
#include "Legacy/Step1/BCTransient/Types.hxx"
#include "Legacy/Step1/Converters/Converters.hxx"
#include "THMZ/ZipModule/ZipModule.hxx"

namespace ThermFile::Migration::BCConsolidation
{
    namespace
    {
        void captureSteadyState(const std::string & steadyStateXml, LegacyBCCapture & result)
        {
            if(steadyStateXml.empty())
            {
                return;
            }

            BCSteadyStateLibrary::DB legacyDB;
            legacyDB.loadFromString(steadyStateXml);
            for(const auto & record : legacyDB.getBoundaryConditions())
            {
                result.boundaryConditions.push_back(BCLibrary::fromSteadyState(record));
                result.bcUUIDByName[record.Name] = record.UUID;
            }
        }

        void captureTypeRecords(const std::string & typeRecordsXml, LegacyBCCapture & result)
        {
            if(typeRecordsXml.empty())
            {
                return;
            }

            BCTypesLibrary::DB legacyDB;
            legacyDB.loadFromString(typeRecordsXml);
            for(const auto & record : legacyDB.getBoundaryConditions())
            {
                result.boundaryConditions.push_back(BCLibrary::fromTypeRecord(record));
            }
        }

        void captureTimestepFile(const std::string & fileName, const std::string & content, LegacyBCCapture & result)
        {
            BCInputFileLibrary::BoundaryConditionTimestep timestep;
            timestep.loadFromXMLString(content);
            if(!timestep.isLoadSuccesful())
            {
                return;   // best-effort transient migration: unreadable file, segment keeps its legacy binding
            }

            const auto datasetName{std::filesystem::path(fileName).stem().string()};
            const auto dataset{BCLibrary::environmentFromTimestep(timestep, datasetName)};
            result.datasetUUIDByFileName[fileName] = dataset.UUID;

            const auto known{lbnl::find_element(result.datasets,
                                                [&dataset](const TimeSeriesLibrary::TimeSeriesData & existing) {
                                                    return existing.UUID == dataset.UUID;
                                                })};
            if(!known.has_value())
            {
                result.datasets.push_back(dataset);
            }
        }

        void bindSegment(const LegacyBCCapture & legacy, ThermFile::Boundary & segment)
        {
            const auto & transient{segment.transientRecordData};

            if(!segment.bcUUID.has_value())
            {
                if(transient.has_value())
                {
                    const auto captured{lbnl::find_element(legacy.boundaryConditions,
                                                           [&transient](const BCLibrary::BoundaryCondition & record) {
                                                               return record.UUID == transient->typeRecordUUID;
                                                           })};
                    if(captured.has_value())
                    {
                        segment.bcUUID = transient->typeRecordUUID;
                    }
                }
                else if(const auto found{legacy.bcUUIDByName.find(segment.name)}; found != legacy.bcUUIDByName.end())
                {
                    segment.bcUUID = found->second;
                }
            }

            if(!segment.timeSeriesUUID.has_value() && transient.has_value())
            {
                const auto found{legacy.datasetUUIDByFileName.find(transient->transientFileName)};
                if(found != legacy.datasetUUIDByFileName.end())
                {
                    segment.timeSeriesUUID = found->second;
                }
            }
        }
    }   // namespace

    bool isEmpty(const LegacyBCCapture & capture)
    {
        return capture.boundaryConditions.empty() && capture.datasets.empty();
    }

    LegacyBCCapture capture(const std::string & steadyStateXml,
                            const std::string & typeRecordsXml,
                            const std::map<std::string, std::string> & timestepFilesByName)
    {
        LegacyBCCapture result;
        captureSteadyState(steadyStateXml, result);
        captureTypeRecords(typeRecordsXml, result);
        for(const auto & [fileName, content] : timestepFilesByName)
        {
            captureTimestepFile(fileName, content, result);
        }

        return result;
    }

    LegacyBCCapture captureFromEntries(const std::map<std::string, std::string> & entries)
    {
        const std::string timestepPrefix{ThermZip::TimestepFilesDir + "/"};
        std::map<std::string, std::string> timestepFiles;
        for(const auto & [entryName, content] : entries)
        {
            if(entryName.starts_with(timestepPrefix))
            {
                timestepFiles[entryName.substr(timestepPrefix.size())] = content;
            }
        }

        return capture(ThermZip::findEntry(entries, ThermZip::SteadyStateBCFileName),
                       ThermZip::findEntry(entries, ThermZip::TransientTypeBCFileName),
                       timestepFiles);
    }

    LegacyBCCapture captureFromArchive(const std::string & zipFileName)
    {
        return captureFromEntries(ThermZip::unzipFiles(zipFileName));
    }

    ThermFile::ThermModel applyToModel(const LegacyBCCapture & legacy, ThermFile::ThermModel model)
    {
        if(isEmpty(legacy))
        {
            return model;
        }

        for(auto & segment : model.boundaryConditions)
        {
            bindSegment(legacy, segment);
        }

        return model;
    }
}   // namespace ThermFile::Migration::BCConsolidation
