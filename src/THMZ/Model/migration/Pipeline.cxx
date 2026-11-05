#include "Pipeline.hxx"

#include <algorithm>
#include <charconv>
#include <string>
#include <utility>

#include "THMZ/Model/migration/BCConsolidation.hxx"
#include "THMZ/Model/migration/ConradTolerance.hxx"
#include "THMZ/Model/migration/FrameCavity.hxx"
#include "THMZ/Model/migration/TimestepFiles.hxx"
#include "THMZ/ZipModule/ZipModule.hxx"

namespace ThermFile::Migration
{
    namespace
    {
        //! The stored version as a number. Anything unreadable counts as the oldest file.
        int parsedVersion(const std::string & version)
        {
            int parsed{0};
            const auto * const end{version.data() + version.size()};
            const auto [position, error]{std::from_chars(version.data(), end, parsed)};
            return error == std::errc{} && position == end ? parsed : 0;
        }
    }   // namespace

    std::map<std::string, std::string> applyAllToEntries(std::map<std::string, std::string> entries)
    {
        // Timestep files of earlier versions become embedded time series datasets.
        entries = TimestepFiles::withConvertedFiles(std::move(entries));

        // New archive conversions go here.

        return entries;
    }

    std::map<std::string, std::string> openArchive(const std::string & zipFileName)
    {
        return applyAllToEntries(ThermZip::unzipFiles(zipFileName));
    }

    ThermFile::ThermModel applyAllToModel(const std::map<std::string, std::string> & entries,
                                          ThermFile::ThermModel model)
    {
        const int fileVersion{parsedVersion(model.version)};

        // Cavity polygons become polygon-level instead of cavity-typed materials.
        if(const auto materialsEntry{ThermZip::findEntry(entries, ThermZip::MaterialsFileName)};
           !materialsEntry.empty())
        {
            model = FrameCavity::applyToModel(FrameCavity::capture(materialsEntry), std::move(model));
        }

        // Boundaries bound by steady record name or transient type record take the unified binding.
        model = BCConsolidation::applyToModel(BCConsolidation::captureFromEntries(entries), std::move(model));

        // Boundaries take the datasets their timestep files became in applyAllToEntries.
        model = TimestepFiles::applyToModel(entries, std::move(model));

        // A stored number has no old shape to recognize, so only the version can say this ran.
        if(fileVersion < ConradTolerance::firstVersionWithConradTolerance)
        {
            model = ConradTolerance::applyToModel(std::move(model));
        }

        // New model conversions go here. A conversion is one file in this folder, with an
        // applyToModel that changes nothing on a model it does not concern; one that cannot
        // tell that from the model is gated on the version here, and the version goes up.

        // A file from a newer build keeps its number: this build has not migrated it to that.
        model.version = std::to_string(std::max(fileVersion, currentModelVersion));

        return model;
    }

    MaterialsLibrary::DB applyAllToMaterials(const std::map<std::string, std::string> & entries,
                                             MaterialsLibrary::DB materials)
    {
        // Cavity and radiation enclosure records are polygon kinds now, not materials.
        const auto legacy{FrameCavity::capture(ThermZip::findEntry(entries, ThermZip::MaterialsFileName))};
        materials = FrameCavity::applyToMaterials(legacy, std::move(materials));

        // New materials conversions go here.

        return materials;
    }

    std::vector<BCLibrary::BoundaryCondition>
      legacyBoundaryConditions(const std::map<std::string, std::string> & entries)
    {
        return BCConsolidation::captureFromEntries(entries).boundaryConditions;
    }
}   // namespace ThermFile::Migration
