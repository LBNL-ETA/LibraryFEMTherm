#include "Pipeline.hxx"

#include <algorithm>
#include <charconv>
#include <stdexcept>
#include <string>
#include <utility>

#include "THMZ/Model/migration/BCConsolidation.hxx"
#include "THMZ/Model/migration/ConradTolerance.hxx"
#include "THMZ/Model/migration/FrameCavity.hxx"
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

    ThermFile::ThermModel applyAllToModel(const std::map<std::string, std::string> & entries,
                                          ThermFile::ThermModel model)
    {
        const int fileVersion{parsedVersion(model.version)};

        // --- Frame-cavity migration -----------------------------------------
        // Legacy archives express cavity polygons as <Type>Material</Type>
        // referencing a cavity-typed material in the materials entry. The new
        // schema makes cavity flavour polygon-level. Capture the legacy hints
        // and rewrite the polygons.
        if(const auto materialsEntry{ThermZip::findEntry(entries, ThermZip::MaterialsFileName)};
           !materialsEntry.empty())
        {
            const auto legacy{FrameCavity::capture(materialsEntry)};
            model = FrameCavity::applyToModel(legacy, std::move(model));
        }

        // --- BC consolidation migration ---------------------------------------
        // Pre-consolidation archives bind segments by steady record name or by
        // transient type-record UUID. Capture the embedded legacy BC records,
        // convert them to the unified form, and fill each segment's bcUUID binding.
        // Legacy per-boundary timestep files are not migrated.
        const auto legacyBCs{BCConsolidation::captureFromEntries(entries)};
        model = BCConsolidation::applyToModel(legacyBCs, std::move(model));

        // --- Conrad tolerance migration ----------------------------------------
        // Version-gated: a stored number has no legacy shape to recognize, so only the
        // file's version can tell whether this already ran.
        model = ConradTolerance::applyToModel(fileVersion, std::move(model));

        // --- Future migrations go below this line ----------------------------
        // Pattern: add a nested namespace under ThermFile::Migration (sibling
        // to FrameCavity), expose `capture` + `applyToModel`, then call them
        // here; read legacy hints from the entries map. Chain via
        // `model = X::applyToModel(legacy, std::move(model));`.

        // A file from a newer build keeps its number: this build has not migrated it to that.
        model.version = std::to_string(std::max(fileVersion, currentModelVersion));

        return model;
    }

    ThermFile::ThermModel applyAllToModel(const std::string & zipFileName, ThermFile::ThermModel model)
    {
        try
        {
            return applyAllToModel(ThermZip::unzipFiles(zipFileName), std::move(model));
        }
        catch(const std::runtime_error &)
        {
            // Archive unreadable for migration hints -- nothing to migrate.
            return model;
        }
    }
}   // namespace ThermFile::Migration
