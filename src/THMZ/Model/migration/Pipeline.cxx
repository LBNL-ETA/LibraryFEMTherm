#include "Pipeline.hxx"

#include <stdexcept>
#include <string>
#include <utility>

#include "THMZ/Model/migration/FrameCavity.hxx"
#include "THMZ/ZipModule/ZipModule.hxx"

namespace ThermFile::Migration
{
    ThermFile::ThermModel applyAllToModel(const std::string & zipFileName, ThermFile::ThermModel model)
    {
        // --- Frame-cavity migration -----------------------------------------
        // Legacy archives express cavity polygons as <Type>Material</Type>
        // referencing a cavity-typed material in Materials.xml. The new schema
        // makes cavity flavour polygon-level. Capture the legacy hints from
        // Materials.xml and rewrite the polygons.
        try
        {
            const auto materialsXml = ThermZip::unzipFile(zipFileName, ThermZip::MaterialsFileName);
            const auto legacy = FrameCavity::capture(materialsXml);
            model = FrameCavity::applyToModel(legacy, std::move(model));
        }
        catch(const std::runtime_error &)
        {
            // Materials.xml not present in the archive -- nothing to migrate.
        }

        // --- Future migrations go below this line ----------------------------
        // Pattern: add a nested namespace under ThermFile::Migration (sibling
        // to FrameCavity), expose `capture` + `applyToModel`, then call them
        // here in their own try/catch so a missing-file in one doesn't block
        // the others. Chain via `model = X::applyToModel(legacy, std::move(model));`.

        return model;
    }
}   // namespace ThermFile::Migration
