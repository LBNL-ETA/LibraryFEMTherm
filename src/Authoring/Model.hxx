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
namespace ThermFile::Authoring
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

    //! No exchange at all: adiabatic and moisture-tight (flux zero).
    struct Sealed
    {
        [[nodiscard]] friend bool operator==(const Sealed & lhs, const Sealed & rhs) = default;
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

    using Boundary = std::variant<Sealed, Prescribed, Convective>;

    // ------------------------------------------------------------------------------------
    // Geometry
    // ------------------------------------------------------------------------------------

    //! One material polygon: its corners in metres, in drawing order, each given once and
    //! the polygon closed implicitly, and the name of the material filling it. The name is
    //! the caller's to choose; it is looked up in the case's own materials.
    struct Region
    {
        std::string material;
        std::vector<Point> points;

        [[nodiscard]] friend bool operator==(const Region & lhs, const Region & rhs) = default;
    };

    //! One boundary segment: a straight line between two points on a region's outline, the
    //! condition it carries, and the index of the region it bounds. The index matters when
    //! regions touch: the same line can be the edge of two of them, and the condition
    //! belongs to one.
    struct Segment
    {
        Boundary kind{Sealed{}};
        Point start;
        Point end;
        std::size_t region{0U};

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
    // The case
    // ------------------------------------------------------------------------------------

    //! A THERM model stated in code: regions, segments, start, clock, settings, and the
    //! materials the regions name.
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
        //! Keyed by whatever the regions call their materials.
        std::map<std::string, Material> materials;

        [[nodiscard]] friend bool operator==(const ModelCase & lhs, const ModelCase & rhs) = default;
    };

    //! Everything that stops the case from being written, one line each; empty when it can
    //! be. Checked: at least one region; three points per region; every segment's region
    //! index in range; every segment lying along one edge of the region it names (both
    //! endpoints on the same edge, within a nanometre); every material a region names
    //! carried by the case; a positive step and a positive number of steps.
    [[nodiscard]] std::vector<std::string> issues(const ModelCase & modelCase);

    //! The material a region names, or nothing if the case does not carry it.
    [[nodiscard]] std::optional<Material> material(const ModelCase & modelCase, std::string_view name);

    //! The material filling the region at the given index, or nothing if the index is out
    //! of range or the case does not carry that material.
    [[nodiscard]] std::optional<Material> regionMaterial(const ModelCase & modelCase, std::size_t index);

    //! The library record a boundary kind is written as. The same in every generated file,
    //! so a library imported from one archive serves another.
    [[nodiscard]] std::string recordName(const Boundary & boundary);
}   // namespace ThermFile::Authoring
