#pragma once

#include "THMX.hxx"
#include "Materials/DB.hxx"

namespace ThermFile
{
    //! \brief One-shot migration for the legacy frame-cavity representation.
    //!
    //! Specific to this refactor; not a general migration framework. If a future
    //! schema change needs migrating, add a sibling function alongside this one
    //! (and consider a versioned dispatcher once there are two-plus migrations
    //! to abstract over).
    //!
    //! Three passes:
    //! 1. *Materials*: capture cavity-typed and radiation-enclosure-typed materials by UUID
    //!    along with their CavityStandard/Gas/ventilation, then drop them from the library.
    //! 2. *Polygons*: for each polygon referencing a captured legacy material, set its
    //!    PolygonType (FrameCavity or RadiationEnclosure), populate per-polygon CavityData,
    //!    and clear materialUUID. Tally per-cavity (standard, gas, ventilated) usage.
    //! 3. *Project defaults*: majority-vote the tally and apply to
    //!    CalculationOptions::frameCavityProperties. Ties prefer ISO15099 / "Air" / false.
    //!
    //! Idempotent: safe to call on already-migrated data (no captured materials means no
    //! polygon rewrites and no project-default promotion).
    void applyFrameCavityMigration(MaterialsLibrary::DB & materials, ThermFile::ThermModel & model);
}   // namespace ThermFile
