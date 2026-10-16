#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "BoundaryConditions/BoundaryConditions.hxx"
#include "Materials/Materials.hxx"
#include "THMZ/Properties/Enumerators.hxx"
#include "THMZ/Properties/Properties.hxx"
#include "TimeSeriesData/TimeSeriesData.hxx"

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

    // ------------------------------------------------------------------------------------
    // Boundary kinds
    // ------------------------------------------------------------------------------------

    //! A boundary input: one value that holds for the whole run, or one value per step of
    //! the schedule. A transient case accepts either; a steady-state case, having no clock,
    //! accepts constants only. The writer keeps a constant on the segment's record and
    //! puts a series in a dataset the record reads, so THERM sees the file as one of its own.
    using Input = std::variant<double, std::vector<double>>;

    [[nodiscard]] bool isConstant(const Input & input);

    //! The number of values a series input carries; nothing for a constant.
    [[nodiscard]] std::optional<std::size_t> seriesLength(const Input & input);

    //! No exchange at all: no heat and no moisture crosses the surface (flux zero). Named
    //! as THERM names its built-in record for exactly this, so the word is the same in the
    //! case, in the file and in THERM's dialog.
    struct Adiabatic
    {
        [[nodiscard]] friend bool operator==(const Adiabatic & lhs, const Adiabatic & rhs) = default;
    };

    //! The surface state itself is imposed: a temperature in °C and optionally a relative
    //! humidity as a fraction.
    struct Prescribed
    {
        Input temperature{0.0};
        std::optional<Input> humidity{};

        [[nodiscard]] friend bool operator==(const Prescribed & lhs, const Prescribed & rhs) = default;
    };

    //! Radiative exchange with a surrounding at a radiant temperature in °C through a fixed
    //! coefficient in W/(m² K): the one radiation model the engine reads step by step, and
    //! the one a weather file feeds (its sky temperature is a radiant temperature).
    struct FixedRadiation
    {
        Input temperature{0.0};
        Input coefficient{0.0};

        [[nodiscard]] friend bool operator==(const FixedRadiation & lhs, const FixedRadiation & rhs) = default;
    };

    //! A surface's radiative exchange: fixed radiation with inputs of its own, or one of
    //! THERM's other models as its records state them. A black body must hold constants;
    //! the enclosures are steady-state only, since the engine does not run them through a
    //! clock.
    using Radiation = std::variant<FixedRadiation,
                                   BCLibrary::BlackBodyRadiation,
                                   BCLibrary::AutomaticEnclosure,
                                   BCLibrary::ManualEnclosure>;

    //! Exchange with an environment: air temperature in °C, relative humidity as a fraction,
    //! and a convection model. The fixed model takes its film coefficient in W/(m² K) from
    //! `filmCoefficient`; the correlations compute theirs and read the wind instead, speed
    //! in m/s and direction in degrees, as each correlation requires. `flux` is an imposed
    //! heat flux in W/m² on top of the exchange.
    struct Convective
    {
        Input airTemperature{0.0};
        std::optional<Input> filmCoefficient{};
        Input humidity{0.5};
        BCLibrary::ConvectionModel model{BCLibrary::ConvectionModel::Fixed_Convection_Coefficient};
        std::optional<Input> windSpeed{};
        std::optional<Input> windDirection{};
        std::optional<Input> flux{};
        std::optional<Radiation> radiation{};

        [[nodiscard]] friend bool operator==(const Convective & lhs, const Convective & rhs) = default;
    };

    using Boundary = std::variant<Adiabatic, Prescribed, Convective>;

    //! One input of a boundary kind and the dataset role it is written under.
    struct RoleInput
    {
        TimeSeriesLibrary::SeriesRole role{TimeSeriesLibrary::SeriesRole::AirTemperature};
        Input value{0.0};

        [[nodiscard]] friend bool operator==(const RoleInput & lhs, const RoleInput & rhs) = default;
    };

    //! Every input a boundary kind carries, each under its role, in the order the record
    //! states them. The one place that pairs an input with a role: the writer puts a series
    //! input in a dataset under this role, and issues() checks it against the schedule.
    //! Empty for an adiabatic segment.
    [[nodiscard]] std::vector<RoleInput> inputs(const Boundary & boundary);

    // ------------------------------------------------------------------------------------
    // Geometry
    // ------------------------------------------------------------------------------------

    //! One material polygon: its corners in metres, in drawing order, each given once and
    //! the polygon closed implicitly, and the material filling it -- a record of THERM's
    //! materials library, whether stated from numbers (MaterialsLibrary::fromValues), taken from a
    //! library or read back from a file. Regions that share a material carry the same
    //! record; the archive's library keeps one record per UUID.
    struct Region
    {
        MaterialsLibrary::Material material;
        std::vector<Point> points;

        //! Two regions are the same region when they carry the same material record, by
        //! identity, over the same corners; a record's tables are not compared.
        [[nodiscard]] friend bool operator==(const Region & lhs, const Region & rhs)
        {
            return lhs.material.UUID == rhs.material.UUID && lhs.material.Name == rhs.material.Name
                   && lhs.points == rhs.points;
        }
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
        //! The colour THERM draws the segment in, 0xRRGGBB. Unset, the kind's own colour
        //! (see kindColor).
        std::optional<std::string> color{};
        //! What the caller calls the face, for plots and messages: "outside", "room side".
        //! Not written to the file, whose segment name is the record's.
        std::optional<std::string> name{};
        //! The U-factor tag the segment reports under, as THERM's "Frame" or "Edge": the
        //! solver sums the heat flow over every segment carrying the same tag and reports
        //! one U-factor per tag. Unset, the segment reports under none.
        std::optional<std::string> uFactorTag{};

        [[nodiscard]] friend bool operator==(const Segment & lhs, const Segment & rhs) = default;
    };

    //! The colour a boundary kind is drawn in when a segment states none, 0xRRGGBB:
    //! black for Adiabatic, as THERM draws its own record, and two colours THERM's shipped
    //! records do not use for the other kinds. The writer puts it on the kind's library
    //! record too, so THERM shows the same colours a plot of the case does.
    [[nodiscard]] std::string kindColor(const Boundary & boundary);

    //! The colour a segment is drawn in: its own, or its kind's.
    [[nodiscard]] std::string segmentColor(const Segment & segment);

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

    //! Where the clock starts in the year: a calendar position with no year, as a dataset's
    //! axis states it. 1 January 00:00 unless stated.
    struct Start
    {
        std::size_t month{1U};
        std::size_t day{1U};
        std::size_t hour{0U};
        std::size_t minute{0U};

        [[nodiscard]] friend bool operator==(const Start & lhs, const Start & rhs) = default;
    };

    //! The clock: nSteps steps of dtime seconds, from `start`. Every series input of the
    //! case has one value per step.
    struct Schedule
    {
        double dtime{3600.0};
        std::size_t nSteps{0U};
        Start start{};

        [[nodiscard]] double duration() const;

        [[nodiscard]] friend bool operator==(const Schedule & lhs, const Schedule & rhs) = default;
    };

    //! The schedule as the axis of the datasets the writer makes.
    [[nodiscard]] TimeSeriesLibrary::TimeAxis axisOf(const Schedule & schedule);

    //! The schedule a dataset dictates: its step, its row count and its start, so a case
    //! driven by a weather file runs exactly over the file.
    [[nodiscard]] Schedule scheduleFor(const TimeSeriesLibrary::TimeSeriesData & dataset);

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
    //!
    //! `mode` is the calculation the archive is set up for. A transient case runs through
    //! its clock: `initial` and `schedule` apply, and every condition is read from a dataset.
    //! A steady-state case has no clock: `initial` and `schedule` are not written, each
    //! condition is a constant on its own record, and `meshControl` -- THERM's own error
    //! target and pass budget -- drives the error estimator.
    struct ModelCase
    {
        std::string id{};
        std::string title{};
        std::vector<Region> regions{};
        std::vector<Segment> segments{};
        Initial initial{};
        Schedule schedule{};
        Physics physics{};
        Numerics numerics{};
        ThermFile::CalculationMode mode{ThermFile::CalculationMode::cmTransient};
        ThermFile::MeshControl meshControl{};

        [[nodiscard]] friend bool operator==(const ModelCase & lhs, const ModelCase & rhs) = default;
    };

    //! Everything that stops the case from being written, one line each; empty when it can
    //! be. Checked: at least one region; three points per region; every material named,
    //! and one set of properties per name (the library keeps one record per name); every
    //! segment attached to one region (see segmentRegion); a mode that is transient or steady
    //! state; a convection model given the inputs it reads (a film coefficient for the fixed
    //! model, the wind for the correlations); for a transient case a positive step and a
    //! positive number of steps, every series input as long as the schedule, and radiation
    //! the engine runs through a clock (fixed radiation, or a black body of constants); for a
    //! steady-state case no prescribed segment, since THERM's steady-state library has no
    //! record for a prescribed surface state, and no series input, since it has no datasets.
    //! Segments are not checked while a region has fewer than three points, since it has
    //! no edges to lie on.
    [[nodiscard]] std::vector<std::string> issues(const ModelCase & modelCase);

    //! Whether the case is set up for a steady-state calculation.
    [[nodiscard]] bool isSteadyState(const ModelCase & modelCase);

    //! The region a segment is attached to: the one it names, provided the segment lies
    //! along one of its edges (both endpoints on the same edge, within a nanometre); or,
    //! when it names none, the only region whose edge carries it. Nothing when the named
    //! region does not exist or does not carry the segment, when no region carries it, or
    //! when it names none and more than one region does -- each of which issues() reports.
    [[nodiscard]] std::optional<std::size_t> segmentRegion(const ModelCase & modelCase, std::size_t index);

    //! The adiabatic segments the case leaves unstated: every stretch of a region's edge
    //! that no other region's edge and no stated segment covers lies on the outside of the
    //! model, and a face nothing is said about is one nothing crosses. Each is attached to
    //! its region and runs in the edge's drawing direction. Empty when every outside face
    //! is stated. Regions with fewer than three points have no edges and are skipped.
    [[nodiscard]] std::vector<Segment> outlineGaps(const ModelCase & modelCase);

    //! The case with its outline gaps appended as adiabatic segments, after the stated
    //! ones so their indices hold. This is what the Build namespace writes; a caller states
    //! only the faces that exchange something.
    [[nodiscard]] ModelCase completed(const ModelCase & modelCase);

    //! The material record filling the region at the given index, or nothing if the index
    //! is out of range.
    [[nodiscard]] std::optional<MaterialsLibrary::Material> regionMaterial(const ModelCase & modelCase,
                                                                          std::size_t index);
}   // namespace ThermFile::Model
