#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include <lbnl/algorithm.hxx>

#include "Model/Archive.hxx"
#include "Model/Case.hxx"

#include "SourceLike.hxx"

namespace py = pybind11;

using namespace ThermFile::Model;
using namespace ThermFile::Build;
using SourceBinding::SourceLike;
using SourceBinding::sourceRepr;
using SourceBinding::toSource;

namespace
{
    //! Every vocabulary type compares by value, so a case can be checked against another.
    template<typename Type>
    void addEquality(py::class_<Type> & cls)
    {
        cls.def(
             "__eq__",
             [](const Type & lhs, const Type & rhs) { return lhs == rhs; },
             py::is_operator())
          .def(
            "__ne__",
            [](const Type & lhs, const Type & rhs) { return !(lhs == rhs); },
            py::is_operator());
    }

    //! Python's repr of a bound value, so a nested value prints as it does on its own:
    //! a string quoted, a bool as True/False, a point or a boundary kind by its own repr.
    template<typename Type>
    std::string pythonRepr(const Type & value)
    {
        return py::repr(py::cast(value)).template cast<std::string>();
    }

    //! ", label=value" for a field that is set, nothing for one that is not.
    template<typename Type>
    std::string optionalField(const std::string_view label, const std::optional<Type> & value)
    {
        return value.has_value() ? std::format(", {}={}", label, pythonRepr(*value)) : std::string{};
    }

    std::string optionalInput(const std::string_view label, const std::optional<BCLibrary::Source> & value)
    {
        return value.has_value() ? std::format(", {}={}", label, sourceRepr(*value)) : std::string{};
    }

    //! A point as a constructor takes it: a Point or a 2-tuple of metres. Spelled out as a
    //! variant so the signature, and the stub generated from it, names both.
    using PointLike = std::variant<Point, std::pair<double, double>>;

    Point toPoint(const PointLike & given)
    {
        if(const auto * pair{std::get_if<std::pair<double, double>>(&given)})
        {
            const auto & [xValue, yValue]{*pair};
            return Point{xValue, yValue};
        }
        return std::get<Point>(given);
    }

    std::string regionRepr(const Region & region)
    {
        return std::format(
          "Region(material={}, points={})", pythonRepr(region.material.Name), pythonRepr(region.points));
    }

    std::string segmentRepr(const Segment & segment)
    {
        return std::format("Segment(kind={}, start={}, end={}{}{}{}{}{})",
                           pythonRepr(segment.kind),
                           pythonRepr(segment.start),
                           pythonRepr(segment.end),
                           optionalField("region", segment.region),
                           optionalField("color", segment.color),
                           optionalField("name", segment.name),
                           optionalField("u_factor_tag", segment.uFactorTag),
                           optionalField("dataset", segment.dataset));
    }

    std::string convectiveRepr(const Convective & kind)
    {
        const std::string modelField{kind.model == BCLibrary::ConvectionModel::Fixed_Convection_Coefficient
                                       ? std::string{}
                                       : std::format(", model={}", py::str(py::cast(kind.model)).cast<std::string>())};
        return std::format("Convective(air_temperature={}{}, humidity={}{}{}{}{}{})",
                           sourceRepr(kind.airTemperature),
                           optionalInput("film_coefficient", kind.filmCoefficient),
                           sourceRepr(kind.humidity),
                           modelField,
                           optionalInput("wind_speed", kind.windSpeed),
                           optionalInput("wind_direction", kind.windDirection),
                           optionalInput("flux", kind.flux),
                           optionalField("radiation", kind.radiation));
    }

    std::string physicsRepr(const Physics & physics)
    {
        return std::format("Physics(thermal={}, moisture={}, liquid_transport={}, heat_of_evaporation={}, "
                           "capillary_conduction={}, vapor_diffusion_conduction={}, latent_heat_of_fusion={}, "
                           "conductivity_dependent={})",
                           pythonRepr(physics.thermal),
                           pythonRepr(physics.moisture),
                           pythonRepr(physics.liquidTransport),
                           pythonRepr(physics.heatOfEvaporation),
                           pythonRepr(physics.capillaryConduction),
                           pythonRepr(physics.vaporDiffusionConduction),
                           pythonRepr(physics.latentHeatOfFusion),
                           pythonRepr(physics.conductivityDependent));
    }

    void bindGeometry(py::module_ & model)
    {
        auto point{py::class_<Point>(model,
                                     "Point",
                                     "A position in metres, in the case's own coordinates. A 2-tuple is "
                                     "accepted wherever a Point is expected.")
                     .def(py::init<>())
                     .def(py::init([](const double x, const double y) { return Point{x, y}; }),
                          py::arg("x"),
                          py::arg("y"))
                     .def(py::init([](const std::pair<double, double> & xy) { return Point{xy.first, xy.second}; }),
                          py::arg("xy"))
                     .def_readwrite("x", &Point::x)
                     .def_readwrite("y", &Point::y)
                     .def("__repr__", [](const Point & point) { return std::format("Point({}, {})", point.x, point.y); })};
        addEquality(point);
        py::implicitly_convertible<py::tuple, Point>();

        auto region{py::class_<Region>(model,
                                       "Region",
                                       "One material polygon: its corners in metres, in drawing order, each given "
                                       "once and the polygon closed implicitly, and the material filling it: a "
                                       "pylibraryfemtherm.Material record, built by keyword, taken from a library or "
                                       "a file. Regions that share a material carry the same record.")
                      .def(py::init([](MaterialsLibrary::Material material, const std::vector<PointLike> & points) {
                               return Region{std::move(material), lbnl::transform_to_vector(points, toPoint)};
                           }),
                           py::arg("material"),
                           py::arg("points"))
                      .def_readwrite("material", &Region::material)
                      .def_readwrite("points", &Region::points)
                      .def("__repr__", &regionRepr)};
        addEquality(region);

        auto segment{py::class_<Segment>(model,
                                         "Segment",
                                         "One boundary segment: a straight line between two points on a region's "
                                         "outline, the condition it carries (one of the `bc` kinds), and "
                                         "optionally the index of the region it bounds. Left None, the region is "
                                         "found: a segment on the outline lies on exactly one region's edge. It must "
                                         "be given where two regions share the edge. `color` is what THERM draws the "
                                         "segment in, 0xRRGGBB; left None, the kind's own colour. `name` is what the "
                                         "caller calls the face, for plots and messages; it is not written to the file. "
                                         "`u_factor_tag` is the U-factor tag the segment reports under, as THERM's "
                                         "\"Frame\" or \"Edge\": one U-factor per tag, over every segment carrying it. "
                                         "`dataset` is what the condition's role inputs read, step by step, as in THERM, "
                                         "where a segment binds one dataset beside its record; it is written to the "
                                         "archive as given, its own time axis included.")
                       .def(py::init([](Boundary kind,
                                        const PointLike & start,
                                        const PointLike & end,
                                        const std::optional<std::size_t> region,
                                        std::optional<std::string> color,
                                        std::optional<std::string> name,
                                        std::optional<std::string> uFactorTag,
                                        std::optional<TimeSeriesLibrary::TimeSeriesData> dataset) {
                                return Segment{.kind = std::move(kind),
                                               .start = toPoint(start),
                                               .end = toPoint(end),
                                               .region = region,
                                               .color = std::move(color),
                                               .name = std::move(name),
                                               .uFactorTag = std::move(uFactorTag),
                                               .dataset = std::move(dataset)};
                            }),
                            py::arg("kind"),
                            py::arg("start"),
                            py::arg("end"),
                            py::arg("region") = py::none(),
                            py::arg("color") = py::none(),
                            py::arg("name") = py::none(),
                            py::arg("u_factor_tag") = py::none(),
                            py::arg("dataset") = py::none())
                       .def_readwrite("kind", &Segment::kind)
                       .def_readwrite("start", &Segment::start)
                       .def_readwrite("end", &Segment::end)
                       .def_readwrite("region", &Segment::region)
                       .def_readwrite("color", &Segment::color)
                       .def_readwrite("name", &Segment::name)
                       .def_readwrite("u_factor_tag", &Segment::uFactorTag)
                       .def_readwrite("dataset", &Segment::dataset)
                       .def("__repr__", &segmentRepr)};
        addEquality(segment);
    }

    void bindBoundaryKinds(py::module_ & model)
    {
        auto bc{model.def_submodule("bc",
                                    "The boundary conditions a segment can carry: Adiabatic, Prescribed or "
                                    "Convective. Every input is a number, held for the whole run, or a SeriesRole, "
                                    "read step by step from the dataset the segment binds.")};

        auto adiabatic{py::class_<Adiabatic>(bc,
                                             "Adiabatic",
                                             "No exchange at all: no heat and no moisture crosses the surface. "
                                             "Written as THERM's own built-in Adiabatic record.")
                         .def(py::init<>())
                         .def("__repr__", [](const Adiabatic &) { return std::string{"Adiabatic()"}; })};
        addEquality(adiabatic);

        auto prescribed{
          py::class_<Prescribed>(bc,
                                 "Prescribed",
                                 "The surface state itself is imposed: a temperature in degrees C and optionally a "
                                 "relative humidity as a fraction. Each is a number, or a SeriesRole read from the "
                                 "segment's dataset.")
            .def(py::init([](const SourceLike & temperature, const std::optional<SourceLike> & humidity) {
                     return Prescribed{.temperature = toSource(temperature), .humidity = toSource(humidity)};
                 }),
                 py::arg("temperature"),
                 py::arg("humidity") = py::none())
            .def_readwrite("temperature", &Prescribed::temperature)
            .def_readwrite("humidity", &Prescribed::humidity)
            .def("__repr__", [](const Prescribed & kind) {
                return std::format("Prescribed(temperature={}{})",
                                   sourceRepr(kind.temperature),
                                   optionalInput("humidity", kind.humidity));
            })};
        addEquality(prescribed);

        auto convective{
          py::class_<Convective>(bc,
                                 "Convective",
                                 "Exchange with an environment: air temperature in degrees C, relative humidity as a "
                                 "fraction, and a convection model. The fixed model takes `film_coefficient` in "
                                 "W/(m2 K); the correlations compute theirs and read `wind_speed` (m/s) and, those "
                                 "that tell windward from leeward, `wind_direction` (degrees). `flux` is an imposed "
                                 "heat flux in W/m2. `radiation` is one of THERM's radiation models: "
                                 "BCFixedCoefficientRadiation, the one the engine reads step by step; "
                                 "BCBlackBodyRadiation with constants; BCAutomaticEnclosure and BCManualEnclosure in "
                                 "a steady-state case only. Every input is a number, or a SeriesRole read from the "
                                 "segment's dataset.")
            .def(py::init([](const SourceLike & airTemperature,
                             const std::optional<SourceLike> & filmCoefficient,
                             const SourceLike & humidity,
                             const BCLibrary::ConvectionModel model,
                             const std::optional<SourceLike> & windSpeed,
                             const std::optional<SourceLike> & windDirection,
                             const std::optional<SourceLike> & flux,
                             std::optional<Radiation> radiation) {
                     return Convective{.airTemperature = toSource(airTemperature),
                                       .filmCoefficient = toSource(filmCoefficient),
                                       .humidity = toSource(humidity),
                                       .model = model,
                                       .windSpeed = toSource(windSpeed),
                                       .windDirection = toSource(windDirection),
                                       .flux = toSource(flux),
                                       .radiation = std::move(radiation)};
                 }),
                 py::arg("air_temperature"),
                 py::arg("film_coefficient") = py::none(),
                 py::arg("humidity") = 0.5,
                 py::arg("model") = BCLibrary::ConvectionModel::Fixed_Convection_Coefficient,
                 py::arg("wind_speed") = py::none(),
                 py::arg("wind_direction") = py::none(),
                 py::arg("flux") = py::none(),
                 py::arg("radiation") = py::none())
            .def_readwrite("air_temperature", &Convective::airTemperature)
            .def_readwrite("film_coefficient", &Convective::filmCoefficient)
            .def_readwrite("humidity", &Convective::humidity)
            .def_readwrite("model", &Convective::model)
            .def_readwrite("wind_speed", &Convective::windSpeed)
            .def_readwrite("wind_direction", &Convective::windDirection)
            .def_readwrite("flux", &Convective::flux)
            .def_readwrite("radiation", &Convective::radiation)
            .def("__repr__", &convectiveRepr)};
        addEquality(convective);

        model.def("record_data", &recordData, py::arg("boundary"),
                  "The boundary kind as THERM's record data: a SurfaceExchange for Convective, a PrescribedState for "
                  "Prescribed, NoExchange for Adiabatic, with the inputs as stated. pylibraryfemtherm.required_roles "
                  "on a record holding it lists the roles the kind reads.");
    }

    void bindSettings(py::module_ & model)
    {
        auto initial{py::class_<Initial>(model,
                                         "Initial",
                                         "Uniform initial state: temperature in degrees C, relative humidity as a "
                                         "fraction.")
                       .def(py::init([](const double temperature, const double humidity) {
                                return Initial{temperature, humidity};
                            }),
                            py::arg("temperature"),
                            py::arg("humidity"))
                       .def_readwrite("temperature", &Initial::temperature)
                       .def_readwrite("humidity", &Initial::humidity)
                       .def("__repr__", [](const Initial & initial) {
                           return std::format("Initial(temperature={}, humidity={})", initial.temperature, initial.humidity);
                       })};
        addEquality(initial);

        auto start{py::class_<Start>(model,
                                     "Start",
                                     "Where the clock starts in the year: a calendar position with no year, as a "
                                     "dataset's axis states it. 1 January 00:00 unless stated.")
                     .def(py::init([](const std::size_t month, const std::size_t day, const std::size_t hour, const std::size_t minute) {
                              return Start{.month = month, .day = day, .hour = hour, .minute = minute};
                          }),
                          py::arg("month") = 1U,
                          py::arg("day") = 1U,
                          py::arg("hour") = 0U,
                          py::arg("minute") = 0U)
                     .def_readwrite("month", &Start::month)
                     .def_readwrite("day", &Start::day)
                     .def_readwrite("hour", &Start::hour)
                     .def_readwrite("minute", &Start::minute)
                     .def("__repr__", [](const Start & start) {
                         return std::format("Start(month={}, day={}, hour={}, minute={})", start.month, start.day, start.hour, start.minute);
                     })};
        addEquality(start);

        auto schedule{py::class_<Schedule>(model,
                                           "Schedule",
                                           "The clock of a case whose segments bind no dataset: one length in seconds "
                                           "per step, from `start`; the steps need not be even. A case whose segments "
                                           "bind datasets runs on the datasets' clock, and a schedule stated beside them "
                                           "must agree with them. Schedule(dtime, n_steps) is the usual even clock, "
                                           "Schedule(step_lengths=[...]) states every step.")
                        .def(py::init([](const double dtime, const std::size_t nSteps, const Start & startAt) {
                                 return uniformSchedule(dtime, nSteps, startAt);
                             }),
                             py::arg("dtime"),
                             py::arg("n_steps"),
                             py::arg("start") = Start{})
                        .def(py::init([](const std::vector<double> & stepLengths, const Start & startAt) {
                                 return Schedule{.start = startAt, .stepLengths = stepLengths};
                             }),
                             py::arg("step_lengths"),
                             py::arg("start") = Start{})
                        .def_property_readonly(
                          "dtime",
                          [](const Schedule & schedule) {
                              const auto step{uniformStep(schedule)};
                              if(!step.has_value())
                              {
                                  throw py::value_error("the schedule has no single step length; read step_lengths");
                              }
                              return step.value();
                          },
                          "The length every step has. Raises ValueError when the steps differ or there are none.")
                        .def_property_readonly("n_steps", &nSteps)
                        .def_readwrite("step_lengths", &Schedule::stepLengths, "Seconds per step.")
                        .def_readwrite("start", &Schedule::start)
                        .def_property_readonly("duration", &Schedule::duration)
                        .def("__repr__", [](const Schedule & schedule) {
                            const std::string startField{schedule.start == Start{} ? std::string{}
                                                                                    : std::format(", start={}", pythonRepr(schedule.start))};
                            const auto step{uniformStep(schedule)};
                            if(step.has_value())
                            {
                                return std::format("Schedule(dtime={}, n_steps={}{})", pythonRepr(step.value()), nSteps(schedule), startField);
                            }
                            return std::format("Schedule(step_lengths={}{})", pythonRepr(schedule.stepLengths), startField);
                        })};
        addEquality(schedule);
        model.def("axis_of", &axisOf, py::arg("schedule"),
                  "The schedule as the axis of the clock dataset the writer makes when no segment binds one: even "
                  "when every step has the same length, otherwise with the lengths stated one by one.");
        model.def("schedule_for", &scheduleFor, py::arg("dataset"),
                  "The schedule a dataset dictates: its steps and their lengths, and its start, so a case driven by "
                  "a weather file runs exactly over the file.");

        auto physics{
          py::class_<Physics>(model,
                              "Physics",
                              "Which terms are simulated, stated positively. Each switch is written to the file's "
                              "calculation options; what the engine does with each is the engine's documentation.")
            .def(py::init([](const bool thermal,
                             const bool moisture,
                             const bool liquidTransport,
                             const bool heatOfEvaporation,
                             const bool capillaryConduction,
                             const bool vaporDiffusionConduction,
                             const bool latentHeatOfFusion,
                             const bool conductivityDependent) {
                     return Physics{thermal,
                                    moisture,
                                    liquidTransport,
                                    heatOfEvaporation,
                                    capillaryConduction,
                                    vaporDiffusionConduction,
                                    latentHeatOfFusion,
                                    conductivityDependent};
                 }),
                 py::arg("thermal") = true,
                 py::arg("moisture") = true,
                 py::arg("liquid_transport") = true,
                 py::arg("heat_of_evaporation") = true,
                 py::arg("capillary_conduction") = true,
                 py::arg("vapor_diffusion_conduction") = true,
                 py::arg("latent_heat_of_fusion") = true,
                 py::arg("conductivity_dependent") = true)
            .def_readwrite("thermal", &Physics::thermal)
            .def_readwrite("moisture", &Physics::moisture)
            .def_readwrite("liquid_transport", &Physics::liquidTransport)
            .def_readwrite("heat_of_evaporation", &Physics::heatOfEvaporation)
            .def_readwrite("capillary_conduction", &Physics::capillaryConduction)
            .def_readwrite("vapor_diffusion_conduction", &Physics::vaporDiffusionConduction)
            .def_readwrite("latent_heat_of_fusion", &Physics::latentHeatOfFusion)
            .def_readwrite("conductivity_dependent", &Physics::conductivityDependent)
            .def("__repr__", &physicsRepr)};
        addEquality(physics);

        auto numerics{py::class_<Numerics>(model,
                                           "Numerics",
                                           "The nonlinear iteration's settings, as the file's engine parameters.")
                        .def(py::init([](const double convergenceTolerance, const double relaxation, const int maxIterations) {
                                 return Numerics{convergenceTolerance, relaxation, maxIterations};
                             }),
                             py::arg("convergence_tolerance") = 1e-5,
                             py::arg("relaxation") = 1.0,
                             py::arg("max_iterations") = 25)
                        .def_readwrite("convergence_tolerance", &Numerics::convergenceTolerance)
                        .def_readwrite("relaxation", &Numerics::relaxation)
                        .def_readwrite("max_iterations", &Numerics::maxIterations)
                        .def("__repr__", [](const Numerics & numerics) {
                            return std::format("Numerics(convergence_tolerance={}, relaxation={}, max_iterations={})",
                                               numerics.convergenceTolerance,
                                               numerics.relaxation,
                                               numerics.maxIterations);
                        })};
        addEquality(numerics);
    }

    void bindCase(py::module_ & model)
    {
        auto modelCase{
          py::class_<ModelCase>(model,
                                "ModelCase",
                                "A THERM model stated in code: regions, each carrying its material, segments, "
                                "start, clock and settings. `mode` is the calculation the archive is set up for: "
                                "a transient case runs from `initial` through its clock, the datasets its segments "
                                "bind or `schedule` when they bind none; a steady-state case has no clock, holds "
                                "each condition as a constant, and `mesh_control` drives its error estimator.")
            .def(py::init([](std::string id,
                             std::vector<Region> regions,
                             std::vector<Segment> segments,
                             const Initial & initial,
                             const Schedule & schedule,
                             const Physics & physics,
                             const Numerics & numerics,
                             std::string title,
                             const ThermFile::CalculationMode mode,
                             const ThermFile::MeshControl & meshControl) {
                     return ModelCase{.id = std::move(id),
                                      .title = std::move(title),
                                      .regions = std::move(regions),
                                      .segments = std::move(segments),
                                      .initial = initial,
                                      .schedule = schedule,
                                      .physics = physics,
                                      .numerics = numerics,
                                      .mode = mode,
                                      .meshControl = meshControl};
                 }),
                 py::arg("id"),
                 py::arg("regions"),
                 py::arg("segments"),
                 py::arg("initial") = Initial{},
                 py::arg("schedule") = Schedule{},
                 py::arg("physics") = Physics{},
                 py::arg("numerics") = Numerics{},
                 py::arg("title") = std::string{},
                 py::arg("mode") = ThermFile::CalculationMode::cmTransient,
                 py::arg("mesh_control") = ThermFile::MeshControl{})
            .def_readwrite("id", &ModelCase::id)
            .def_readwrite("title", &ModelCase::title)
            .def_readwrite("regions", &ModelCase::regions)
            .def_readwrite("segments", &ModelCase::segments)
            .def_readwrite("initial", &ModelCase::initial)
            .def_readwrite("schedule", &ModelCase::schedule)
            .def_readwrite("physics", &ModelCase::physics)
            .def_readwrite("numerics", &ModelCase::numerics)
            .def_readwrite("mode", &ModelCase::mode)
            .def_readwrite("mesh_control", &ModelCase::meshControl)};
        addEquality(modelCase);

        model.def("is_steady_state", &isSteadyState, py::arg("model_case"),
                  "Whether the case is set up for a steady-state calculation.");
        model.def("region_material", &regionMaterial, py::arg("model_case"), py::arg("index"),
                      "The material filling the region at the given index, or None when there is no such region.");
        model.def("segment_region", &segmentRegion, py::arg("model_case"), py::arg("index"),
                      "The region the segment at the given index is attached to: the one it names, or the "
                      "only region whose edge carries it. None when issues() would report the segment.");
        model.def("kind_color", &kindColor, py::arg("boundary"),
                  "The colour a boundary kind is drawn in when a segment states none, 0xRRGGBB: black for "
                  "Adiabatic, as THERM draws its own record. The writer puts it on the kind's library record too.");
        model.def("segment_color", &segmentColor, py::arg("segment"),
                  "The colour a segment is drawn in, 0xRRGGBB: its own, or its kind's.");
        model.def("outline_gaps", &outlineGaps, py::arg("model_case"),
                  "The adiabatic segments the case leaves unstated: every stretch of a region's edge that no "
                  "other region's edge and no stated segment covers. Empty when every outside face is stated.");
        model.def("completed", &completed, py::arg("model_case"),
                  "The case with its outline gaps appended as adiabatic segments, after the stated ones. This "
                  "is what build writes; a caller states only the faces that exchange something.");
    }

    //! An enumerator by its qualified name, `SeriesRole.AirTemperature`.
    template<typename Enum>
    std::string enumName(const Enum value)
    {
        return py::str(py::cast(value)).template cast<std::string>();
    }

    std::string rolesField(const std::vector<TimeSeriesLibrary::SeriesRole> & roles)
    {
        if(roles.empty())
        {
            return {};
        }
        std::string names;
        for(const auto role : roles)
        {
            names += (names.empty() ? "" : ", ") + enumName(role);
        }
        return std::format(", roles=[{}]", names);
    }

    std::string issueRepr(const Issue & issue)
    {
        return std::format("Issue(kind={}{}{}{}{}{}{})",
                           enumName(issue.kind),
                           optionalField("region", issue.region),
                           optionalField("other_region", issue.otherRegion),
                           optionalField("segment", issue.segment),
                           optionalField("step", issue.step),
                           rolesField(issue.roles),
                           issue.input.has_value() ? ", input=" + enumName(issue.input.value()) : std::string{});
    }

    void bindIssueKinds(py::module_ & model)
    {
        py::enum_<IssueKind>(model, "IssueKind", "What is wrong with a case, one kind per check issues() makes.")
          .value("NoRegions", IssueKind::NoRegions)
          .value("TooFewPoints", IssueKind::TooFewPoints)
          .value("UnnamedMaterial", IssueKind::UnnamedMaterial)
          .value("MaterialNameClash", IssueKind::MaterialNameClash)
          .value("RegionMissing", IssueKind::RegionMissing)
          .value("OffRegion", IssueKind::OffRegion)
          .value("OffEveryRegion", IssueKind::OffEveryRegion)
          .value("SharedEdge", IssueKind::SharedEdge)
          .value("NoSteps", IssueKind::NoSteps)
          .value("NonPositiveStep", IssueKind::NonPositiveStep)
          .value("ScheduleDisagrees", IssueKind::ScheduleDisagrees)
          .value("ConvectionInputMissing", IssueKind::ConvectionInputMissing)
          .value("RoleWithoutDataset", IssueKind::RoleWithoutDataset)
          .value("RoleNotProvided", IssueKind::RoleNotProvided)
          .value("EnclosureInTransient", IssueKind::EnclosureInTransient)
          .value("BlackBodyFromDataset", IssueKind::BlackBodyFromDataset)
          .value("PrescribedInSteadyState", IssueKind::PrescribedInSteadyState)
          .value("RoleInSteadyState", IssueKind::RoleInSteadyState)
          .value("DatasetInSteadyState", IssueKind::DatasetInSteadyState)
          .value("UnsupportedMode", IssueKind::UnsupportedMode);
        py::enum_<ConvectionInput>(model, "ConvectionInput", "The input a convection model reads.")
          .value("FilmCoefficient", ConvectionInput::FilmCoefficient)
          .value("WindSpeed", ConvectionInput::WindSpeed)
          .value("WindDirection", ConvectionInput::WindDirection);
    }

    //! The issues: facts a caller can act on, and LibraryFEMTherm's wording of them.
    void bindIssues(py::module_ & model)
    {
        auto issue{py::class_<Issue>(model,
                                     "Issue",
                                     "One thing wrong with a case: its kind and what it is about, as indices into "
                                     "the case and the roles or input concerned; only the fields the kind names are "
                                     "set. describe() words it.")
                     .def_readonly("kind", &Issue::kind)
                     .def_readonly("region", &Issue::region)
                     .def_readonly("other_region", &Issue::otherRegion)
                     .def_readonly("segment", &Issue::segment)
                     .def_readonly("step", &Issue::step)
                     .def_readonly("roles", &Issue::roles)
                     .def_readonly("input", &Issue::input)
                     .def("__repr__", &issueRepr)};
        addEquality(issue);
        model.def("issues", &issues, py::arg("model_case"),
                  "Everything that stops the case from being written, one Issue each; empty when it can be.");
        model.def("describe", &describe, py::arg("model_case"), py::arg("issue"),
                  "LibraryFEMTherm's wording of an issue of the case it was found in, as THERM shows it.");
    }

    //! Raises ValueError with the issues worded one per line, the issues themselves on its
    //! `issues` attribute for a caller that wants them.
    [[noreturn]] void raiseIssues(const ModelCase & modelCase, const std::vector<Issue> & found)
    {
        std::string text;
        for(const auto & issue : found)
        {
            text += (text.empty() ? "" : "\n") + describe(modelCase, issue);
        }
        const py::object error{py::module_::import("builtins").attr("ValueError")(text)};
        error.attr("issues") = py::cast(found);
        PyErr_SetObject(PyExc_ValueError, error.ptr());
        throw py::error_already_set();
    }

    void bindArchive(py::module_ & build)
    {
        py::class_<Libraries>(build, "Libraries", "Everything a model refers to by UUID.")
          .def_readonly("materials", &Libraries::materials)
          .def_readonly("boundary_conditions", &Libraries::boundaryConditions)
          .def_readonly("datasets", &Libraries::datasets);

        build.attr("DRAWING_ORIGIN_MM") = py::make_tuple(drawingOriginMm.x, drawingOriginMm.y);

        build.def("case_uuid", &caseUuid, py::arg("model_case"), py::arg("key"),
                  "A UUID that depends on the case id and a key, stable across runs.");
        build.attr("ADIABATIC_RECORD_UUID") = std::string{adiabaticRecordUuid};

        auto binding{py::class_<Binding>(build,
                                         "Binding",
                                         "What a segment refers to by UUID: its record, and its dataset when it "
                                         "binds one (or carries the clock).")
                       .def_readonly("record_uuid", &Binding::recordUuid)
                       .def_readonly("dataset_uuid", &Binding::datasetUuid)
                       .def("__repr__", [](const Binding & bound) {
                           return std::format("Binding(record_uuid={}{})",
                                              pythonRepr(bound.recordUuid),
                                              optionalField("dataset_uuid", bound.datasetUuid));
                       })};
        addEquality(binding);
        build.def("segment_records", &segmentRecords, py::arg("model_case"),
                  "Every record the case's segments refer to, each once, in the order first used: THERM's "
                  "built-in Adiabatic record and one record per distinct lowered condition, named after the "
                  "first segment using it.");
        build.def("segment_bindings", &segmentBindings, py::arg("model_case"),
                  "The record and dataset each segment of the completed case refers to, in the completed "
                  "case's order.");
        build.def("boundary_datasets", &boundaryDatasets, py::arg("model_case"),
                  "Segment index -> dataset for every segment of the completed case that binds one, or a clock "
                  "on the first segment when none does. Segments sharing a dataset repeat it.");
        build.def("model", &model, py::arg("model_case"),
                  "The THERM model of the case: polygons, boundary segments and calculation options. The case "
                  "must have no issues.");
        build.def("libraries", &libraries, py::arg("model_case"),
                  "The materials, the boundary-condition records and the datasets the model uses. The case "
                  "must have no issues.");
        build.def(
          "thmz_entries",
          [](const ModelCase & modelCase) {
              const auto entries{thmzEntries(modelCase)};
              if(!entries.has_value())
              {
                  raiseIssues(modelCase, entries.error());
              }
              return entries.value();
          },
          py::arg("model_case"),
          "Every entry of the case's THMZ, name -> content, before zipping. Raises ValueError with the case's "
          "issues worded one per line, and the issues themselves as its `issues`, when it cannot be written.");
        build.def(
          "thmz",
          [](const ModelCase & modelCase, const std::string & path) {
              const auto written{thmz(modelCase, path)};
              if(!written.has_value())
              {
                  raiseIssues(modelCase, written.error());
              }
              return written.value();
          },
          py::arg("model_case"),
          py::arg("path"),
          "Writes the case's THMZ and returns the path. Creates the parent directory and overwrites an existing "
          "file. Raises ValueError with the case's issues worded one per line, and the issues themselves as its "
          "`issues`, when it cannot be written.");
        build.def("schedule_of", &scheduleOf, py::arg("datasets"),
                  "The clock the mediator will run, read from the first dataset as the mediator does.");
    }
}   // namespace

//! Two submodules. `model` is the vocabulary a THERM model is stated in: regions in metres
//! carrying their material, segments carrying a physical condition (the kinds live in
//! `model.bc`), a uniform start, a clock and the calculation switches. `build` turns such a
//! case into what THERM would have saved:
//! the model, its libraries, the archive. The file-level types bound elsewhere in this
//! module are what the archive is made of; these two are the layer above.
void bind_case(py::module_ & mod)
{
    auto model{mod.def_submodule("model", "State a THERM model in physical terms; see pylibraryfemtherm docs.")};
    // The kinds first: Segment's signatures name them, and a signature uses a type's Python
    // name only once that type is registered.
    bindBoundaryKinds(model);
    bindGeometry(model);
    bindSettings(model);
    bindCase(model);
    bindIssueKinds(model);
    bindIssues(model);

    auto build{mod.def_submodule("build", "Turn a stated model into the archive THERM would have saved.")};
    bindArchive(build);
}
