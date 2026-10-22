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

namespace py = pybind11;

using namespace ThermFile::Model;
using namespace ThermFile::Build;

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

    //! A series input prints its first values and its length rather than every value:
    //! a month of weather is 744 numbers, and a repr is for reading.
    std::string inputRepr(const Input & input)
    {
        const auto * series{std::get_if<std::vector<double>>(&input)};
        constexpr std::size_t shownInFull{6U};
        if(series == nullptr || series->size() <= shownInFull)
        {
            return pythonRepr(input);
        }
        const std::vector<double> head(series->begin(), series->begin() + 3);
        return std::format("[{}, ..., {} ({} values)]",
                           pythonRepr(head).substr(1, pythonRepr(head).size() - 2),
                           pythonRepr(series->back()),
                           series->size());
    }

    std::string optionalInput(const std::string_view label, const std::optional<Input> & value)
    {
        return value.has_value() ? std::format(", {}={}", label, inputRepr(*value)) : std::string{};
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
        return std::format("Segment(kind={}, start={}, end={}{}{}{}{})",
                           pythonRepr(segment.kind),
                           pythonRepr(segment.start),
                           pythonRepr(segment.end),
                           optionalField("region", segment.region),
                           optionalField("color", segment.color),
                           optionalField("name", segment.name),
                           optionalField("u_factor_tag", segment.uFactorTag));
    }

    std::string convectiveRepr(const Convective & kind)
    {
        const std::string modelField{kind.model == BCLibrary::ConvectionModel::Fixed_Convection_Coefficient
                                       ? std::string{}
                                       : std::format(", model={}", py::str(py::cast(kind.model)).cast<std::string>())};
        return std::format("Convective(air_temperature={}{}, humidity={}{}{}{}{}{})",
                           inputRepr(kind.airTemperature),
                           optionalInput("film_coefficient", kind.filmCoefficient),
                           inputRepr(kind.humidity),
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
                                         "\"Frame\" or \"Edge\": one U-factor per tag, over every segment carrying it.")
                       .def(py::init([](Boundary kind,
                                        const PointLike & start,
                                        const PointLike & end,
                                        const std::optional<std::size_t> region,
                                        std::optional<std::string> color,
                                        std::optional<std::string> name,
                                        std::optional<std::string> uFactorTag) {
                                return Segment{.kind = std::move(kind),
                                               .start = toPoint(start),
                                               .end = toPoint(end),
                                               .region = region,
                                               .color = std::move(color),
                                               .name = std::move(name),
                                               .uFactorTag = std::move(uFactorTag)};
                            }),
                            py::arg("kind"),
                            py::arg("start"),
                            py::arg("end"),
                            py::arg("region") = py::none(),
                            py::arg("color") = py::none(),
                            py::arg("name") = py::none(),
                            py::arg("u_factor_tag") = py::none())
                       .def_readwrite("kind", &Segment::kind)
                       .def_readwrite("start", &Segment::start)
                       .def_readwrite("end", &Segment::end)
                       .def_readwrite("region", &Segment::region)
                       .def_readwrite("color", &Segment::color)
                       .def_readwrite("name", &Segment::name)
                       .def_readwrite("u_factor_tag", &Segment::uFactorTag)
                       .def("__repr__", &segmentRepr)};
        addEquality(segment);
    }

    void bindBoundaryKinds(py::module_ & model)
    {
        auto bc{model.def_submodule("bc",
                                    "The boundary conditions a segment can carry: Adiabatic, Prescribed or "
                                    "Convective. Each is constant in time.")};

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
                                 "relative humidity as a fraction. Each is a constant, or one value per step of "
                                 "the schedule.")
            .def(py::init([](Input temperature, std::optional<Input> humidity) {
                     return Prescribed{.temperature = std::move(temperature), .humidity = std::move(humidity)};
                 }),
                 py::arg("temperature"),
                 py::arg("humidity") = py::none())
            .def_readwrite("temperature", &Prescribed::temperature)
            .def_readwrite("humidity", &Prescribed::humidity)
            .def("__repr__", [](const Prescribed & kind) {
                return std::format("Prescribed(temperature={}{})",
                                   inputRepr(kind.temperature),
                                   optionalInput("humidity", kind.humidity));
            })};
        addEquality(prescribed);

        auto fixedRadiation{
          py::class_<FixedRadiation>(bc,
                                     "FixedRadiation",
                                     "Radiative exchange with a surrounding at a radiant temperature in degrees C "
                                     "through a fixed coefficient in W/(m2 K): the one radiation model the engine "
                                     "reads step by step, and the one a weather file feeds (its sky temperature is "
                                     "a radiant temperature). Each input is a constant or one value per step.")
            .def(py::init([](Input temperature, Input coefficient) {
                     return FixedRadiation{.temperature = std::move(temperature), .coefficient = std::move(coefficient)};
                 }),
                 py::arg("temperature"),
                 py::arg("coefficient"))
            .def_readwrite("temperature", &FixedRadiation::temperature)
            .def_readwrite("coefficient", &FixedRadiation::coefficient)
            .def("__repr__", [](const FixedRadiation & radiation) {
                return std::format("FixedRadiation(temperature={}, coefficient={})",
                                   inputRepr(radiation.temperature),
                                   inputRepr(radiation.coefficient));
            })};
        addEquality(fixedRadiation);

        auto convective{
          py::class_<Convective>(bc,
                                 "Convective",
                                 "Exchange with an environment: air temperature in degrees C, relative humidity as a "
                                 "fraction, and a convection model. The fixed model takes `film_coefficient` in "
                                 "W/(m2 K); the correlations compute theirs and read `wind_speed` (m/s) and, those "
                                 "that tell windward from leeward, `wind_direction` (degrees). `flux` is an imposed "
                                 "heat flux in W/m2. `radiation` is FixedRadiation, or one of THERM's own models "
                                 "(BCBlackBodyRadiation with constants; BCAutomaticEnclosure and BCManualEnclosure in "
                                 "a steady-state case only). Every input is a constant or one value per step of the "
                                 "schedule.")
            .def(py::init([](Input airTemperature,
                             std::optional<Input> filmCoefficient,
                             Input humidity,
                             const BCLibrary::ConvectionModel model,
                             std::optional<Input> windSpeed,
                             std::optional<Input> windDirection,
                             std::optional<Input> flux,
                             std::optional<Radiation> radiation) {
                     return Convective{.airTemperature = std::move(airTemperature),
                                       .filmCoefficient = std::move(filmCoefficient),
                                       .humidity = std::move(humidity),
                                       .model = model,
                                       .windSpeed = std::move(windSpeed),
                                       .windDirection = std::move(windDirection),
                                       .flux = std::move(flux),
                                       .radiation = std::move(radiation)};
                 }),
                 py::arg("air_temperature"),
                 py::arg("film_coefficient") = py::none(),
                 py::arg("humidity") = Input{0.5},
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

        auto roleInput{py::class_<RoleInput>(model,
                                             "RoleInput",
                                             "One input of a boundary kind and the dataset role it is written under.")
                         .def_readonly("role", &RoleInput::role)
                         .def_readonly("value", &RoleInput::value)
                         .def("__repr__", [](const RoleInput & input) {
                             return std::format("RoleInput(role={}, value={})",
                                                py::str(py::cast(input.role)).cast<std::string>(),
                                                pythonRepr(input.value));
                         })};
        addEquality(roleInput);
        model.def("inputs", &inputs, py::arg("boundary"),
                  "Every input a boundary kind carries, each under its role, in the order the record states "
                  "them: the one place that pairs an input with a role. Empty for Adiabatic.");
        model.def("is_constant", &isConstant, py::arg("input"), "Whether the input is one value for the whole run.");
        model.def("series_length", &seriesLength, py::arg("input"),
                  "The number of values a series input carries; None for a constant.");
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
                                           "The clock: one length in seconds per step, from `start`; the steps need "
                                           "not be even. Every series input of the case has one value per step. "
                                           "Schedule(dtime, n_steps) is the usual even clock, "
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
                  "The schedule as the axis of the datasets the writer makes: even when every step has the same "
                  "length, otherwise with the lengths stated one by one.");
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
                                "a transient case runs through its clock (`initial`, `schedule`); a steady-state "
                                "case has no clock, holds each condition as a constant, and `mesh_control` drives "
                                "its error estimator.")
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

        model.def("issues", &issues, py::arg("model_case"),
                      "Everything that stops the case from being written, one line each; empty when it can be.");
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

        py::class_<Lowered>(build,
                            "Lowered",
                            "A boundary kind in the file's terms: the record data, a constant input as a constant "
                            "source and a series input as a source reading its role, and the series the record "
                            "reads, each under that role.")
          .def_readonly("data", &Lowered::data)
          .def_readonly("series", &Lowered::series);
        build.def("lower", &lower, py::arg("boundary"), "The boundary kind lowered to record data and series.");

        auto binding{py::class_<Binding>(build,
                                         "Binding",
                                         "What a segment refers to by UUID: its record, and its dataset when the "
                                         "record reads series (or the segment carries the clock).")
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
                  "Segment index -> dataset for every segment of the completed case that reads series, plus a "
                  "clock on the first segment when none does. Segments sharing a dataset repeat it.");
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
                  throw py::value_error(entries.error());
              }
              return entries.value();
          },
          py::arg("model_case"),
          "Every entry of the case's THMZ, name -> content, before zipping. Raises ValueError with the case's "
          "issues when it cannot be written.");
        build.def(
          "thmz",
          [](const ModelCase & modelCase, const std::string & path) {
              const auto written{thmz(modelCase, path)};
              if(!written.has_value())
              {
                  throw py::value_error(written.error());
              }
              return written.value();
          },
          py::arg("model_case"),
          py::arg("path"),
          "Writes the case's THMZ and returns the path. Creates the parent directory and overwrites an existing "
          "file. Raises ValueError with the case's issues when it cannot be written.");
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

    auto build{mod.def_submodule("build", "Turn a stated model into the archive THERM would have saved.")};
    bindArchive(build);
}
