#pragma once

#include <cstddef>
#include <map>
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

    // Every input of a boundary kind is a BCLibrary::Source, as on THERM's own records: a
    // Constant that holds for the whole run, or FromTimeSeries{role}, read step by step
    // from the dataset the segment binds (Segment::dataset). A steady-state case, having no
    // datasets, accepts constants only.

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
        BCLibrary::Source temperature{0.0};
        std::optional<BCLibrary::Source> humidity{};

        [[nodiscard]] friend bool operator==(const Prescribed & lhs, const Prescribed & rhs) = default;
    };

    //! A surface's radiative exchange, as THERM's records state it. Fixed-coefficient
    //! radiation is the one model the engine reads step by step, and the one a weather file
    //! feeds (its sky temperature is a radiant temperature); a black body must hold
    //! constants; the enclosures are steady-state only, since the engine does not run them
    //! through a clock.
    using Radiation = BCLibrary::RadiationOptions;

    //! Exchange with an environment: air temperature in °C, relative humidity as a fraction,
    //! and a convection model. The fixed model takes its film coefficient in W/(m² K) from
    //! `filmCoefficient`; the correlations compute theirs and read the wind instead, speed
    //! in m/s and direction in degrees, as each correlation requires. `flux` is an imposed
    //! heat flux in W/m² on top of the exchange.
    struct Convective
    {
        BCLibrary::Source airTemperature{0.0};
        std::optional<BCLibrary::Source> filmCoefficient{};
        BCLibrary::Source humidity{0.5};
        BCLibrary::ConvectionModel model{BCLibrary::ConvectionModel::Fixed_Convection_Coefficient};
        std::optional<BCLibrary::Source> windSpeed{};
        std::optional<BCLibrary::Source> windDirection{};
        std::optional<BCLibrary::Source> flux{};
        std::optional<Radiation> radiation{};

        [[nodiscard]] friend bool operator==(const Convective & lhs, const Convective & rhs) = default;
    };

    using Boundary = std::variant<Adiabatic, Prescribed, Convective>;

    //! The data of a boundary-condition record: what a boundary kind is in the file.
    using RecordData = decltype(BCLibrary::BoundaryCondition::data);

    //! The boundary kind as THERM's record data: a Convective as a surface exchange, a
    //! Prescribed as a prescribed state, Adiabatic as no exchange. The inputs carry over as
    //! they are, constants and roles alike.
    [[nodiscard]] RecordData recordData(const Boundary & boundary);

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
    //!
    //! `dataset` is what the condition's FromTimeSeries inputs read, as in THERM, where a
    //! segment binds one dataset beside its record. It is written to the archive as it is
    //! given, its own axis included, so the run's clock is the datasets' clock.
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
        std::optional<TimeSeriesLibrary::TimeSeriesData> dataset{};

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

    //! Where the clock starts in the year: a calendar position with no year, as a dataset's
    //! axis states it and counted the same way (TimeSeriesLibrary::TimeAxis): month, day and
    //! hour from 1, hour 1 being 00:00 to 01:00, and minute from 0. 1 January 00:00 unless
    //! stated.
    struct Start
    {
        std::size_t month{1U};
        std::size_t day{1U};
        std::size_t hour{1U};
        std::size_t minute{0U};

        [[nodiscard]] friend bool operator==(const Start & lhs, const Start & rhs) = default;
    };

    //! The clock of a case whose segments bind no dataset: one length in seconds per step,
    //! from `start`. The steps need not be even. A case whose segments bind datasets runs on
    //! the datasets' clock; a schedule stated beside them must agree with them.
    struct Schedule
    {
        Start start{};
        std::vector<double> stepLengths;

        [[nodiscard]] double duration() const;

        [[nodiscard]] friend bool operator==(const Schedule & lhs, const Schedule & rhs) = default;
    };

    //! The usual clock: `nSteps` steps of `dtime` seconds.
    [[nodiscard]] Schedule uniformSchedule(double dtime, std::size_t nSteps, Start start = {});

    [[nodiscard]] std::size_t nSteps(const Schedule & schedule);

    //! The one length every step has, or nothing when the steps differ or there are none.
    [[nodiscard]] std::optional<double> uniformStep(const Schedule & schedule);

    //! The schedule as the axis of the clock dataset the writer makes when no segment binds
    //! one: even when every step has the same length, otherwise with the lengths stated one
    //! by one.
    [[nodiscard]] TimeSeriesLibrary::TimeAxis axisOf(const Schedule & schedule);

    //! The schedule a dataset dictates: its steps and their lengths, and its start, so a
    //! case driven by a weather file runs exactly over the file.
    [[nodiscard]] Schedule scheduleFor(const TimeSeriesLibrary::TimeSeriesData & dataset);

    //! Uniform starting state: temperature in °C, relative humidity as a fraction.
    struct Uniform
    {
        double temperature{20.0};
        double humidity{0.5};

        [[nodiscard]] friend bool operator==(const Uniform & lhs, const Uniform & rhs) = default;
    };

    //! A start solved first: the steady state of the same regions, with the faces named in
    //! `conditions` under those conditions instead of their own (the initializer). Faces not
    //! named keep the condition they carry, read at their first step when it comes from a
    //! dataset. `start` is where the steady iteration begins; none means the engine's own
    //! default. A field the case does not solve keeps its start value.
    struct FromSteadyState
    {
        std::map<std::string, Boundary> conditions{};
        std::optional<Uniform> start{};

        [[nodiscard]] friend bool operator==(const FromSteadyState & lhs, const FromSteadyState & rhs) = default;
    };

    //! A start solved first: the end of a transient run of the same regions over `schedule`,
    //! with the faces named in `conditions` under those conditions instead of their own,
    //! itself started from `start`. Faces reading a dataset read it from its beginning, as
    //! the main run then does again.
    struct FromTransient
    {
        std::map<std::string, Boundary> conditions{};
        Schedule schedule{};
        Uniform start{};

        [[nodiscard]] friend bool operator==(const FromTransient & lhs, const FromTransient & rhs) = default;
    };

    //! How a case starts: uniform, or from a state solved first.
    using Initial = std::variant<Uniform, FromSteadyState, FromTransient>;

    //! The uniform values a start is stated with: the constant itself, or where the
    //! initializer begins (20 °C and 0.5 standing in when a steady start names none). Not
    //! where a solved start's run begins: that is the field the initializer ends with.
    [[nodiscard]] Uniform initialValues(const Initial & initial);

    //! The conditions a solved start swaps in, by face name; none for a uniform start.
    [[nodiscard]] const std::map<std::string, Boundary> * startConditions(const Initial & initial);

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
    //! its clock -- the datasets its segments bind, or `schedule` when they bind none -- from
    //! `initial`, uniform or solved first. A steady-state case has no clock: `schedule` is
    //! not written, `initial` is the uniform start of its iteration, each condition is a
    //! constant on its own record, and `meshControl` -- THERM's own error target and pass
    //! budget -- drives the error estimator.
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

    //! What is wrong with a case, one kind per check issues() makes.
    enum class IssueKind
    {
        NoRegions,                 //!< the case states no region
        TooFewPoints,              //!< `region` has fewer than three corners
        UnnamedMaterial,           //!< `region`'s material has no name
        MaterialNameClash,         //!< `region` and `otherRegion` carry different materials of one name
        RegionMissing,             //!< `segment` names a region that does not exist
        OffRegion,                 //!< `segment` does not lie along an edge of the region it names
        OffEveryRegion,            //!< `segment` lies along no region's edge
        SharedEdge,                //!< `segment` lies on the edge `region` and `otherRegion` share; it must name one
        NoSteps,                   //!< the schedule has no step, and no segment binds a dataset
        NonPositiveStep,           //!< the schedule's `step` is not positive
        ScheduleDisagrees,         //!< the stated schedule differs from the axis of `segment`'s dataset
        ConvectionInputMissing,    //!< `segment`'s convection model reads `input`, which it does not give
        RoleWithoutDataset,        //!< `segment` reads `roles` but binds no dataset
        RoleNotProvided,           //!< `segment`'s dataset provides none of `roles`, which it reads
        EnclosureInTransient,      //!< `segment` radiates into an enclosure in a transient case
        BlackBodyFromDataset,      //!< `segment`'s black body reads a dataset in a transient case
        RoleInSteadyState,         //!< `segment` reads `roles` in a steady-state case
        DatasetInSteadyState,      //!< `segment` binds a dataset in a steady-state case
        SolvedStartInSteadyState,  //!< the start is solved first, which a steady-state case does not do
        StartConditionUnknownName, //!< the start names a face `name` that no segment carries
        StartConditionReadsRole,   //!< the start's condition on `name` reads `roles`; it must be constants
        StartConditionInputMissing,//!< the start's condition on `name` reads `input`, which it does not give
        UnsupportedMode            //!< the mode is neither transient nor steady state
    };

    //! The input a convection model reads.
    enum class ConvectionInput
    {
        FilmCoefficient,
        WindSpeed,
        WindDirection
    };

    //! One thing wrong with a case: its kind and what it is about, as indices into the case
    //! and the roles or input concerned. Only the fields the kind names are set. describe()
    //! words it; a caller wanting its own words, or to point at the region or segment,
    //! reads the fields instead.
    struct Issue
    {
        IssueKind kind{IssueKind::NoRegions};
        std::optional<std::size_t> region{};
        std::optional<std::size_t> otherRegion{};
        std::optional<std::size_t> segment{};
        std::optional<std::size_t> step{};
        std::vector<TimeSeriesLibrary::SeriesRole> roles{};
        std::optional<ConvectionInput> input{};
        std::optional<std::string> name{};

        [[nodiscard]] friend bool operator==(const Issue & lhs, const Issue & rhs) = default;
    };

    //! LibraryFEMTherm's wording of an issue of the case it was found in, as THERM and the
    //! Python bindings show it.
    [[nodiscard]] std::string describe(const ModelCase & modelCase, const Issue & issue);

    //! Everything that stops the case from being written, one issue each; empty when it can
    //! be. Checked: at least one region; three points per region; every material named,
    //! and one set of properties per name (the library keeps one record per name); every
    //! segment attached to one region (see segmentRegion); a mode that is transient or steady
    //! state; a convection model given the inputs it reads (a film coefficient for the fixed
    //! model, the wind for the correlations); for a transient case a dataset on every segment
    //! whose condition reads a role, providing every role it reads (BCLibrary::missingRoles),
    //! a schedule of positive steps when no segment binds a dataset, a schedule that agrees
    //! with every bound dataset's axis when one is stated beside them, and radiation the
    //! engine runs through a clock (fixed coefficient, or a black body of constants), and for
    //! a start solved first every face it names carried by a segment and every condition it
    //! swaps in a constant given the inputs its convection model reads; for a steady-state
    //! case a uniform start, and no role read and no dataset bound, since it has no
    //! datasets. Whether the datasets of a case agree with one another is not checked
    //! here: TimeSeriesLibrary::alignmentIssues says, and HygroTherm refuses a misaligned
    //! archive before it solves. Segments are not checked while a region has fewer than three
    //! points, since it has no edges to lie on.
    [[nodiscard]] std::vector<Issue> issues(const ModelCase & modelCase);

    //! Every issue of the case, each in LibraryFEMTherm's words; none when it can be written.
    [[nodiscard]] std::vector<std::string> describeIssues(const ModelCase & modelCase);

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

    //! The case with the named faces under other conditions: a copy in which every segment
    //! whose name is a key of `conditions` carries that condition instead of its own and
    //! binds no dataset, since a swapped-in condition is constants. Segments not named are
    //! untouched.
    [[nodiscard]] ModelCase withConditions(const ModelCase & modelCase,
                                           const std::map<std::string, Boundary> & conditions);

    //! The case the initializer solves, for a start solved first; nothing for a uniform
    //! start. FromSteadyState: the case under the swapped conditions, steady state, starting
    //! from `start` (or the default start), with no schedule; the id and title say
    //! "initializer". FromTransient: the same, transient, over the start's schedule.
    [[nodiscard]] std::optional<ModelCase> initializerCase(const ModelCase & modelCase);

    //! The material record filling the region at the given index, or nothing if the index
    //! is out of range.
    [[nodiscard]] std::optional<MaterialsLibrary::Material> regionMaterial(const ModelCase & modelCase,
                                                                          std::size_t index);
}   // namespace ThermFile::Model
