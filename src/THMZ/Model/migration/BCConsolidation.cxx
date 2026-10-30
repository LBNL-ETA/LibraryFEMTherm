#include <lbnl/algorithm.hxx>

#include "BCConsolidation.hxx"

#include "Legacy/Step1/BCSteadyState/DB.hxx"
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

        //! A transient face names its type record by UUID, and that UUID is the record's
        //! identity in the unified library too, whether the archive embedded the record or
        //! the record is one THERM ships ("Fixed film coefficient - Indoor" and the like):
        //! the binding is carried over as it is, and resolves wherever the record is.
        void bindSegment(const LegacyBCCapture & legacy, ThermFile::Boundary & segment)
        {
            if(segment.bcUUID.has_value())
            {
                return;
            }
            const auto & transient{segment.transientRecordData};
            if(transient.has_value())
            {
                if(!transient->typeRecordUUID.empty())
                {
                    segment.bcUUID = transient->typeRecordUUID;
                }
            }
            else if(const auto found{legacy.bcUUIDByName.find(segment.name)}; found != legacy.bcUUIDByName.end())
            {
                segment.bcUUID = found->second;
            }
        }
    }   // namespace

    bool isEmpty(const LegacyBCCapture & capture)
    {
        return capture.boundaryConditions.empty();
    }

    LegacyBCCapture capture(const std::string & steadyStateXml, const std::string & typeRecordsXml)
    {
        LegacyBCCapture result;
        captureSteadyState(steadyStateXml, result);
        captureTypeRecords(typeRecordsXml, result);
        return result;
    }

    LegacyBCCapture captureFromEntries(const std::map<std::string, std::string> & entries)
    {
        return capture(ThermZip::findEntry(entries, ThermZip::SteadyStateBCFileName),
                       ThermZip::findEntry(entries, ThermZip::TransientTypeBCFileName));
    }

    LegacyBCCapture captureFromArchive(const std::string & zipFileName)
    {
        return captureFromEntries(ThermZip::unzipFiles(zipFileName));
    }

    ThermFile::ThermModel applyToModel(const LegacyBCCapture & legacy, ThermFile::ThermModel model)
    {
        for(auto & segment : model.boundaryConditions)
        {
            bindSegment(legacy, segment);
        }

        return model;
    }
}   // namespace ThermFile::Migration::BCConsolidation
