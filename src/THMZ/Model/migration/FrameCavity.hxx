#pragma once

#include <map>
#include <set>
#include <string>

#include "THMZ/Model/THMX.hxx"
#include "Materials/DB.hxx"

namespace ThermFile
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

    //! \brief Scan a Materials.xml document for legacy cavity-typed and
    //! radiation-enclosure-typed material entries. Returns empty if no legacy
    //! data is present.
    LegacyMaterialsCapture captureLegacyMaterials(const std::string & materialsXml);

    //! \brief One-shot migration for the legacy frame-cavity representation.
    //!
    //! Specific to this refactor; not a general migration framework. If a future
    //! schema change needs migrating, add a sibling function alongside this one
    //! (and consider a versioned dispatcher once there are two-plus migrations
    //! to abstract over).
    //!
    //! Three passes:
    //! 1. *Materials*: delete legacy non-solid material UUIDs from the loaded DB
    //!    (they may have been parsed as empty Solid records by the flat parser).
    //! 2. *Polygons*: for each polygon referencing a captured legacy material, set its
    //!    PolygonType (FrameCavity or RadiationEnclosure), populate per-polygon CavityData
    //!    (gas + ventilated bit derived from the legacy standard variant), and clear
    //!    materialUUID. Tally per-cavity convection-model usage along the way.
    //! 3. *Project convection model*: majority-vote the tally and apply to
    //!    CalculationOptions::frameCavityProperties.convectionModel. Ties prefer ISO15099.
    //!
    //! Idempotent: passing an empty capture is a safe no-op.
    void applyFrameCavityMigration(const LegacyMaterialsCapture & legacy,
                                   MaterialsLibrary::DB & materials,
                                   ThermFile::ThermModel & model);
}   // namespace ThermFile
