#pragma once

#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

//! A THERM model stated in physical terms, for writing an archive from code.
//!
//! The types under ThermFile mirror the file entry by entry: page millimetres, records by
//! UUID, inputs as sources reading dataset roles. Stating a model through them means
//! knowing the file. This vocabulary sits above that: regions in metres filled with a
//! material named by the caller, boundary segments carrying a physical condition, a uniform
//! start, a clock, and the calculation switches. Archive.hxx turns one of these into the
//! archive THERM would have saved.
//!
//! Nothing here assumes a shape. A one-dimensional case is a model whose regions happen
//! to be thin. Nothing here supplies materials either: a case carries the ones it uses.
namespace ThermFile::Model
{
    //! A position in metres, in the case's own coordinates. Only relative positions matter:
    //! a region and the segments bordering it must agree, but where the origin sits does not
    //! affect the solution.
    struct Point
    {
        double x{0.0};
        double y{0.0};

        [[nodiscard]] friend bool operator==(const Point & lhs, const Point & rhs) = default;
    };

    //! A table of (x, y) samples, interpolated linearly by whoever reads it.
    using Curve = std::vector<std::pair<double, double>>;

    // ------------------------------------------------------------------------------------
    // Boundary kinds
    // ------------------------------------------------------------------------------------

    //! No exchange at all: no heat and no moisture crosses the surface (flux zero). Named
    //! as THERM names its built-in record for exactly this, so the word is the same in the
    //! case, in the file and in THERM's dialog.
    struct Adiabatic
    {
        [[nodiscard]] friend bool operator==(const Adiabatic & lhs, const Adiabatic & rhs) = default;
    };

    //! The surface state itself is imposed: a temperature in °C and optionally a relative
    //! humidity as a fraction. Constant in time.
    struct Prescribed
    {
        double temperature{0.0};
        std::optional<double> humidity{};

        [[nodiscard]] friend bool operator==(const Prescribed & lhs, const Prescribed & rhs) = default;
    };

    //! Exchange with an environment: air temperature in °C, film coefficient in W/(m² K),
    //! relative humidity as a fraction. Constant in time.
    struct Convective
    {
        double airTemperature{0.0};
        double filmCoefficient{0.0};
        double humidity{0.5};

        [[nodiscard]] friend bool operator==(const Convective & lhs, const Convective & rhs) = default;
    };

    using Boundary = std::variant<Adiabatic, Prescribed, Convective>;

    // ------------------------------------------------------------------------------------
    // Material
    // ------------------------------------------------------------------------------------

    //! One material's thermal and hygric properties, as a caller states them. The only
    //! required fields are a name, a vapour resistance factor and a sorption isotherm; the
    //! thermal properties default to zero so a moisture-only case need not invent them.
    //! Materials.hxx converts one of these into the library record THERM stores.
    struct Material
    {
        std::string name;
        //! mu [-], the factor by which the material resists vapour diffusion relative to
        //! still air. Stands alone unless muCurve is given.
        double diffusionResistanceFactor{0.0};
        //! (phi, w) with w in kg/m³: total water content against relative humidity.
        Curve sorptionCurve;
        //! (w, D_l) with D_l in m²/s. Empty: no liquid transport.
        Curve liquidTransportCurve;
        //! (phi, mu). Empty: the scalar factor above. Given: mu varies with humidity.
        Curve muCurve;
        double density{0.0};                             //!< dry, kg/m³
        double heatCapacity{0.0};                        //!< dry, J/(kg K)
        double thermalConductivity{0.0};                 //!< dry, W/(m K)
        //! k(T) = thermalConductivity (1 + beta T). Zero keeps it constant.
        double thermalConductivityBeta{0.0};
        //! k(w) = thermalConductivity + slope w. Zero keeps it constant.
        double thermalConductivityMoistureSlope{0.0};
        double porosity{0.0};                            //!< total, [-]

        [[nodiscard]] friend bool operator==(const Material & lhs, const Material & rhs) = default;
    };

    // ------------------------------------------------------------------------------------
    // Geometry
    // ------------------------------------------------------------------------------------

    //! One material polygon: its corners in metres, in drawing order, each given once and
    //! the polygon closed implicitly, and the material filling it. Regions that share a
    //! material state the same one; the archive's library keeps one record per material
    //! name.
    struct Region
    {
        Material material;
        std::vector<Point> points;

        [[nodiscard]] friend bool operator==(const Region & lhs, const Region & rhs) = default;
    };

    //! One boundary segment: a straight line between two points on a region's outline, the
    //! condition it carries, and optionally the index of the region it bounds. Left unset,
    //! the region is found: a segment on the model's outline lies on exactly one region's
    //! edge. It must be stated where regions touch, since the same line is then the edge
    //! of two of them and the condition belongs to one.
    struct Segment
    {
        Boundary kind{Adiabatic{}};
        Point start;
        Point end;
        std::optional<std::size_t> region{};

        [[nodiscard]] friend bool operator==(const Segment & lhs, const Segment & rhs) = default;
    };

    // ------------------------------------------------------------------------------------
    // Settings
    // ------------------------------------------------------------------------------------

    //! Uniform initial state: temperature in °C, relative humidity as a fraction.
    struct Initial
    {
        double temperature{20.0};
        double humidity{0.5};

        [[nodiscard]] friend bool operator==(const Initial & lhs, const Initial & rhs) = default;
    };

    //! The clock: nSteps steps of dtime seconds.
    struct Schedule
    {
        double dtime{3600.0};
        std::size_t nSteps{0U};

        [[nodiscard]] double duration() const;

        [[nodiscard]] friend bool operator==(const Schedule & lhs, const Schedule & rhs) = default;
    };

    //! Which terms are simulated, stated positively. Each switch is written to the file's
    //! calculation options; what the engine does with each is the engine's documentation.
    struct Physics
    {
        bool thermal{true};
        bool moisture{true};
        bool liquidTransport{true};
        bool heatOfEvaporation{true};
        bool capillaryConduction{true};
        bool vaporDiffusionConduction{true};
        bool latentHeatOfFusion{true};
        //! k(w, T) from the material tables. Written to both of the file's
        //! conductivity-dependence flags.
        bool conductivityDependent{true};

        [[nodiscard]] friend bool operator==(const Physics & lhs, const Physics & rhs) = default;
    };

    //! The nonlinear iteration's settings, as the file's engine parameters.
    struct Numerics
    {
        double convergenceTolerance{1e-5};
        double relaxation{1.0};
        int maxIterations{25};

        [[nodiscard]] friend bool operator==(const Numerics & lhs, const Numerics & rhs) = default;
    };

    // ------------------------------------------------------------------------------------
    // The case
    // ------------------------------------------------------------------------------------

    //! A THERM model stated in code: regions, each carrying its material, segments, start,
    //! clock and settings.
    struct ModelCase
    {
        std::string id;
        std::string title;
        std::vector<Region> regions;
        std::vector<Segment> segments;
        Initial initial;
        Schedule schedule;
        Physics physics;
        Numerics numerics;

        [[nodiscard]] friend bool operator==(const ModelCase & lhs, const ModelCase & rhs) = default;
    };

    //! Everything that stops the case from being written, one line each; empty when it can
    //! be. Checked: at least one region; three points per region; every material named,
    //! and one set of properties per name (the library keeps one record per name); every
    //! segment attached to one region (see segmentRegion); a positive step and a positive
    //! number of steps. Segments are not checked while a region has fewer than three
    //! points, since it has no edges to lie on.
    [[nodiscard]] std::vector<std::string> issues(const ModelCase & modelCase);

    //! The region a segment is attached to: the one it names, provided the segment lies
    //! along one of its edges (both endpoints on the same edge, within a nanometre); or,
    //! when it names none, the only region whose edge carries it. Nothing when the named
    //! region does not exist or does not carry the segment, when no region carries it, or
    //! when it names none and more than one region does -- each of which issues() reports.
    [[nodiscard]] std::optional<std::size_t> segmentRegion(const ModelCase & modelCase, std::size_t index);

    //! The material filling the region at the given index, or nothing if the index is out
    //! of range.
    [[nodiscard]] std::optional<Material> regionMaterial(const ModelCase & modelCase, std::size_t index);

    //! The library record a boundary kind is written as. The same in every generated file,
    //! so a library imported from one archive serves another. An adiabatic segment is
    //! written as THERM's own built-in "Adiabatic" record rather than a record of its own.
    [[nodiscard]] std::string recordName(const Boundary & boundary);
}   // namespace ThermFile::Model
