#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Materials/DB.hxx"
#include "Materials/Materials.hxx"

//! A material stated in physical terms, made into the record THERM's materials library
//! stores. The record is the only material type a model carries; fromValues() is the
//! shorthand for building one from a few numbers, so a case need not spell out THERM's
//! tables by hand. A record taken from THERM's library, or read back from a file, goes on
//! a region as it is.
//!
//! Conventions that differ between the physical statement and the record, and how they
//! are reconciled:
//!
//! - Table axes. The library keys the moisture-dependent conductivity and the
//!   moisture-dependent vapour resistance factor by water content; the statement keys its
//!   mu curve by humidity. The curve is converted point by point through the sorption
//!   isotherm, exact at every sample.
//! - Linear laws. The statement gives k(T) and k(w) as a slope; the library stores tables.
//!   A two-point table reproduces a linear law exactly, since the engine interpolates
//!   linearly.
//! - Constants. THERM writes a constant table as a single point; the same is done here so
//!   a generated file reads like one THERM saved.
//! - Liquid transport. The library carries two liquid tables, suction and redistribution;
//!   the statement has one curve, written to both.
//! - Identity. The UUID is derived from the material name, so regenerating a case never
//!   produces a second record for the same material.
namespace MaterialsLibrary
{
    //! A table of (x, y) samples, interpolated linearly by whoever reads it.
    using Curve = std::vector<std::pair<double, double>>;

    //! The record UUID for a material name, the same on every machine and run.
    [[nodiscard]] std::string materialUuid(std::string_view name);

    //! A display colour for THERM's polygon fill, derived from the name so it is the same
    //! in every archive. THERM's format is 0xRRGGBB; channels are kept off the extremes so
    //! no material is white or black.
    [[nodiscard]] std::string materialColor(std::string_view name);

    //! The isotherm's last value: the water-content axis end of every w-keyed table.
    [[nodiscard]] double maxWaterContent(const Curve & sorptionCurve);

    //! The sorption isotherm at the given humidity, linear interpolation, clamped at the
    //! table's ends. Total water, liquid and vapour together, kg/m³.
    [[nodiscard]] double waterContent(const Curve & sorptionCurve, double humidity);

    //! A mu(phi) curve re-keyed by water content through the isotherm. Empty when the
    //! curve is empty, in which case the scalar factor stands alone.
    [[nodiscard]] Curve resistanceFactorByWaterContent(const Curve & muCurve, const Curve & sorptionCurve);

    //! The library record for a material stated in physical terms, ready for a region or
    //! a materials database. Required: a name, the vapour resistance factor mu [-] and the
    //! sorption isotherm as (phi, w) with w in kg/m³. Optional: the liquid transport curve
    //! (w, D_l) with D_l in m²/s, empty for none; a (phi, mu) curve, empty to keep the
    //! scalar factor; dry density kg/m³, heat capacity J/(kg K) and conductivity W/(m K),
    //! zero so a moisture-only case need not invent them; k(T) = k (1 + beta T) and
    //! k(w) = k + slope w, zero for constant; total porosity [-]; and THERM's fill colour
    //! 0xRRGGBB, unset for one derived from the name.
    [[nodiscard]] Material fromValues(std::string_view name,
                                                      double diffusionResistanceFactor,
                                                      const Curve & sorptionCurve,
                                                      const Curve & liquidTransportCurve = {},
                                                      const Curve & muCurve = {},
                                                      double density = 0.0,
                                                      double heatCapacity = 0.0,
                                                      double thermalConductivity = 0.0,
                                                      double thermalConductivityBeta = 0.0,
                                                      double thermalConductivityMoistureSlope = 0.0,
                                                      double porosity = 0.0,
                                                      const std::optional<std::string> & color = std::nullopt);

    //! A materials library holding the given records, one per distinct UUID.
    [[nodiscard]] DB materialsDatabase(const std::vector<Material> & records);
}   // namespace MaterialsLibrary
