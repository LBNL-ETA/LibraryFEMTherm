#include "Pipeline.hxx"

#include <stdexcept>
#include <string>
#include <utility>

#include "THMZ/Model/migration/BCConsolidation.hxx"
#include "THMZ/Model/migration/FrameCavity.hxx"
#include "THMZ/ZipModule/ZipModule.hxx"

namespace ThermFile::Migration
{
    ThermFile::ThermModel applyAllToModel(const std::map<std::string, std::string> & entries,
                                          ThermFile::ThermModel model)
    {
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

        // --- Future migrations go below this line ----------------------------
        // Pattern: add a nested namespace under ThermFile::Migration (sibling
        // to FrameCavity), expose `capture` + `applyToModel`, then call them
        // here; read legacy hints from the entries map. Chain via
        // `model = X::applyToModel(legacy, std::move(model));`.

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
