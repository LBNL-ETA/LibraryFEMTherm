#pragma once

#include "THMX.hxx"
#include "Materials/DB.hxx"

namespace ThermFile
{
    //! \brief Migrates a legacy-shape THMZ (Model + Materials) to the new shape in place.
    //!
    //! Three passes:
    //! 1. *Materials*: capture cavity-typed and radiation-enclosure-typed materials by UUID
    //!    along with their CavityStandard/Gas/ventilation, then drop them from the library.
    //! 2. *Polygons*: for each polygon referencing a captured legacy material, set its
    //!    PolygonType (FrameCavity or RadiationEnclosure), populate per-polygon CavityData,
    //!    clear materialUUID, and drop legacy cavityUUID. Tally per-cavity
    //!    (standard, gas, ventilated) usage.
    //! 3. *Project defaults*: majority-vote the tally and apply to
    //!    CalculationOptions::frameCavityProperties. Ties prefer ISO15099 / "Air" / false.
    //!
    //! Safe to call on already-migrated data -- no captured materials means no polygon
    //! rewrites and no project-default promotion.
    void applyMigrationShim(MaterialsLibrary::DB & materials, ThermFile::ThermModel & model);
}   // namespace ThermFile
