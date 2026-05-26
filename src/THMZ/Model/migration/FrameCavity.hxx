#pragma once

#include <map>
#include <set>
#include <string>

#include "THMZ/Model/THMX.hxx"
#include "Materials/DB.hxx"

namespace ThermFile::Migration::FrameCavity
{
    //! \brief One per legacy frame-cavity-typed material entry: the cavity standard
    //! string (raw, e.g. "ISO15099Ventilated") and the gas name.
    //! The standard is kept opaque here; normalization happens inside the migration.
    struct LegacyCavityEntry
    {
        std::string cavityStandard;
        std::string gas;
    };

    //! \brief Side-channel capture of legacy non-solid material entries that the
    //! current flat Material parser silently drops.
    struct LegacyMaterialsCapture
    {
        std::map<std::string, LegacyCavityEntry> cavities;          // material UUID -> cavity data
        std::set<std::string> radiationEnclosures;                  // material UUIDs
    };

    //! \brief Return type for applyAll: the migrated materials DB and model paired
    //! together, so callers can destructure or move out of a single value.
    struct MigrationResult
    {
        MaterialsLibrary::DB materials;
        ThermFile::ThermModel model;
    };

    //! \brief Scan a Materials.xml document for legacy cavity-typed and
    //! radiation-enclosure-typed material entries. Returns empty if no legacy
    //! data is present.
    [[nodiscard]] LegacyMaterialsCapture capture(const std::string & materialsXml);

    //! \brief Materials-side legacy migration: returns a copy of the input DB
    //! with the now-repurposed cavity-typed and radiation-enclosure-typed
    //! material UUIDs stripped. The flat Material parser leaves empty Solid
    //! records behind for those entries; this removes them. Idempotent: empty
    //! capture is a no-op (returns the input unchanged).
    [[nodiscard]] MaterialsLibrary::DB applyToMaterials(const LegacyMaterialsCapture & legacy,
                                                       MaterialsLibrary::DB materials);

    //! \brief Model-side legacy migration: returns a copy of the input model
    //! with each polygon referencing a captured legacy material rewritten --
    //! PolygonType set (FrameCavity or RadiationEnclosure), per-polygon
    //! CavityData populated (gas + ventilated bit derived from the legacy
    //! standard variant), materialUUID cleared. Tallies per-cavity
    //! convection-model usage and majority-votes it onto
    //! CalculationOptions::frameCavityProperties.convectionModel (ties prefer
    //! ISO15099). Idempotent: empty capture is a no-op.
    [[nodiscard]] ThermFile::ThermModel applyToModel(const LegacyMaterialsCapture & legacy,
                                                    ThermFile::ThermModel model);

    //! \brief Convenience wrapper that runs both applyToMaterials and
    //! applyToModel. Use this when the caller holds both the materials DB and
    //! the model. Loaders that only see one side can call the relevant half
    //! directly.
    [[nodiscard]] MigrationResult applyAll(const LegacyMaterialsCapture & legacy,
                                           MaterialsLibrary::DB materials,
                                           ThermFile::ThermModel model);
}   // namespace ThermFile::Migration::FrameCavity
