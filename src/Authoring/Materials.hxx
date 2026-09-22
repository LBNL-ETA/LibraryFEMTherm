#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "Materials/DB.hxx"
#include "Materials/Materials.hxx"

#include "Model.hxx"

//! An authoring material as the record THERM's materials library stores.
//!
//! The library keeps the same information with its own field names, its own table
//! conventions and a UUID per record. This is the one place the two vocabularies meet.
//! Conventions that differ, and how they are reconciled:
//!
//! - Table axes. The library keys the moisture-dependent conductivity and the
//!   moisture-dependent vapour resistance factor by water content; the authoring material
//!   keys its mu curve by humidity. The curve is converted point by point through the
//!   sorption isotherm, exact at every sample.
//! - Linear laws. The authoring material states k(T) and k(w) as a slope; the library
//!   stores tables. A two-point table reproduces a linear law exactly, since the engine
//!   interpolates linearly.
//! - Constants. THERM writes a constant table as a single point; the same is done here so
//!   a generated file reads like one THERM saved.
//! - Liquid transport. The library carries two liquid tables, suction and redistribution;
//!   the authoring material has one curve, written to both.
//! - Identity. The UUID is derived from the material name, so regenerating a case never
//!   produces a second record for the same material.
namespace ThermFile::Authoring
{
    //! The record UUID for a material name, the same on every machine and run.
    [[nodiscard]] std::string materialUuid(std::string_view name);

    //! A display colour for THERM's polygon fill, derived from the name so it is the same
    //! in every archive. THERM's format is 0xRRGGBB; channels are kept off the extremes so
    //! no material is white or black.
    [[nodiscard]] std::string materialColor(std::string_view name);

    //! The isotherm's last value: the water-content axis end of every w-keyed table.
    [[nodiscard]] double maxWaterContent(const Material & material);

    //! The sorption isotherm at the given humidity, linear interpolation, clamped at the
    //! table's ends. Total water, liquid and vapour together, kg/m³.
    [[nodiscard]] double waterContent(const Material & material, double humidity);

    //! The mu(phi) curve re-keyed by water content through the isotherm. Empty when the
    //! material has no curve, in which case the scalar factor stands alone.
    [[nodiscard]] Curve resistanceFactorByWaterContent(const Material & material);

    //! The authoring material as a library record, ready for a materials database.
    [[nodiscard]] MaterialsLibrary::Material libraryMaterial(const Material & material);

    //! A materials library holding the given materials, one record per distinct name.
    [[nodiscard]] MaterialsLibrary::DB materialsDatabase(const std::vector<Material> & materials);
}   // namespace ThermFile::Authoring
