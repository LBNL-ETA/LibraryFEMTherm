#include <set>

#include <lbnl/algorithm.hxx>

#include "EntryPlan.hxx"

#include "ContentHash.hxx"

#include "THMZ/ZipModule/ZipModule.hxx"

namespace TimeSeriesLibrary
{
    namespace
    {
        bool needsWrite(const std::vector<std::string> & existingEntryNames,
                        const std::string & entryName,
                        const std::map<std::string, std::uint64_t> & knownFingerprints,
                        const TimeSeriesData & dataset)
        {
            if(!lbnl::contains(existingEntryNames, entryName))
            {
                return true;
            }

            const auto known{knownFingerprints.find(dataset.UUID)};
            return known == knownFingerprints.end()
                   || known->second != datasetFingerprint(dataset);
        }
    }   // namespace

    EntryPlan planEntryUpdates(const std::vector<std::string> & existingEntryNames,
                               const std::vector<TimeSeriesData> & datasets,
                               const std::map<std::string, std::uint64_t> & knownFingerprints,
                               FileParse::FileFormat format)
    {
        EntryPlan plan;
        std::set<std::string> wantedNames;

        for(const auto & dataset : datasets)
        {
            const auto entryName{ThermZip::timeSeriesEntryName(dataset.UUID, format)};
            wantedNames.insert(entryName);

            if(needsWrite(existingEntryNames, entryName, knownFingerprints, dataset))
            {
                plan.toWrite.push_back(dataset.UUID);
            }
        }

        plan.toErase = lbnl::filter(existingEntryNames, [&wantedNames](const std::string & name) {
            return !wantedNames.contains(name);
        });

        return plan;
    }
}   // namespace TimeSeriesLibrary
