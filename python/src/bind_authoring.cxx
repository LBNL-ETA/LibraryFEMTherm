#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <format>
#include <string>

#include "Authoring/Archive.hxx"
#include "Authoring/Materials.hxx"
#include "Authoring/Model.hxx"

namespace py = pybind11;

using namespace ThermFile::Authoring;

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

    void bindGeometry(py::module_ & authoring)
    {
        auto point{py::class_<Point>(authoring,
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

        auto region{py::class_<Region>(authoring,
                                       "Region",
                                       "One material polygon: its corners in metres, in drawing order, each given "
                                       "once and the polygon closed implicitly, and the name of the material "
                                       "filling it -- whatever the case calls it in `ModelCase.materials`.")
                      .def(py::init([](std::string material, std::vector<Point> points) {
                               return Region{std::move(material), std::move(points)};
                           }),
                           py::arg("material"),
                           py::arg("points"))
                      .def_readwrite("material", &Region::material)
                      .def_readwrite("points", &Region::points)};
        addEquality(region);

        auto segment{py::class_<Segment>(authoring,
                                         "Segment",
                                         "One boundary segment: a straight line between two points on a region's "
                                         "outline, the condition it carries (Sealed, Prescribed or Convective), and "
                                         "the index of the region it bounds.")
                       .def(py::init([](Boundary kind, const Point & start, const Point & end, const std::size_t region) {
                                return Segment{std::move(kind), start, end, region};
                            }),
                            py::arg("kind"),
                            py::arg("start"),
                            py::arg("end"),
                            py::arg("region") = 0U)
                       .def_readwrite("kind", &Segment::kind)
                       .def_readwrite("start", &Segment::start)
                       .def_readwrite("end", &Segment::end)
                       .def_readwrite("region", &Segment::region)};
        addEquality(segment);
    }

    void bindBoundaryKinds(py::module_ & authoring)
    {
        auto sealed{py::class_<Sealed>(authoring, "Sealed", "No exchange at all: adiabatic and moisture-tight.")
                      .def(py::init<>())
                      .def("__repr__", [](const Sealed &) { return std::string{"Sealed()"}; })};
        addEquality(sealed);

        auto prescribed{
          py::class_<Prescribed>(authoring,
                                 "Prescribed",
                                 "The surface state itself is imposed: a temperature in degrees C and optionally a "
                                 "relative humidity as a fraction. Constant in time.")
            .def(py::init([](const double temperature, const std::optional<double> humidity) {
                     return Prescribed{temperature, humidity};
                 }),
                 py::arg("temperature"),
                 py::arg("humidity") = py::none())
            .def_readwrite("temperature", &Prescribed::temperature)
            .def_readwrite("humidity", &Prescribed::humidity)
            .def("__repr__", [](const Prescribed & kind) {
                return kind.humidity.has_value()
                         ? std::format("Prescribed(temperature={}, humidity={})", kind.temperature, *kind.humidity)
                         : std::format("Prescribed(temperature={})", kind.temperature);
            })};
        addEquality(prescribed);

        auto convective{
          py::class_<Convective>(authoring,
                                 "Convective",
                                 "Exchange with an environment: air temperature in degrees C, film coefficient in "
                                 "W/(m2 K), relative humidity as a fraction. Constant in time.")
            .def(py::init([](const double airTemperature, const double filmCoefficient, const double humidity) {
                     return Convective{airTemperature, filmCoefficient, humidity};
                 }),
                 py::arg("air_temperature"),
                 py::arg("film_coefficient"),
                 py::arg("humidity") = 0.5)
            .def_readwrite("air_temperature", &Convective::airTemperature)
            .def_readwrite("film_coefficient", &Convective::filmCoefficient)
            .def_readwrite("humidity", &Convective::humidity)
            .def("__repr__", [](const Convective & kind) {
                return std::format("Convective(air_temperature={}, film_coefficient={}, humidity={})",
                                   kind.airTemperature,
                                   kind.filmCoefficient,
                                   kind.humidity);
            })};
        addEquality(convective);
    }

    void bindSettings(py::module_ & authoring)
    {
        auto initial{py::class_<Initial>(authoring,
                                         "Initial",
                                         "Uniform initial state: temperature in degrees C, relative humidity as a "
                                         "fraction.")
                       .def(py::init([](const double temperature, const double humidity) {
                                return Initial{temperature, humidity};
                            }),
                            py::arg("temperature"),
                            py::arg("humidity"))
                       .def_readwrite("temperature", &Initial::temperature)
                       .def_readwrite("humidity", &Initial::humidity)};
        addEquality(initial);

        auto schedule{py::class_<Schedule>(authoring, "Schedule", "The clock: n_steps steps of dtime seconds.")
                        .def(py::init([](const double dtime, const std::size_t nSteps) {
                                 return Schedule{dtime, nSteps};
                             }),
                             py::arg("dtime"),
                             py::arg("n_steps"))
                        .def_readwrite("dtime", &Schedule::dtime)
                        .def_readwrite("n_steps", &Schedule::nSteps)
                        .def_property_readonly("duration", &Schedule::duration)};
        addEquality(schedule);

        auto physics{
          py::class_<Physics>(authoring,
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
            .def_readwrite("conductivity_dependent", &Physics::conductivityDependent)};
        addEquality(physics);

        auto numerics{py::class_<Numerics>(authoring,
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
                        .def_readwrite("max_iterations", &Numerics::maxIterations)};
        addEquality(numerics);
    }

    void bindMaterial(py::module_ & authoring)
    {
        auto material{
          py::class_<Material>(authoring,
                               "Material",
                               "One material's thermal and hygric properties, as the caller states them. Required: "
                               "a name, a vapour resistance factor and a sorption isotherm; the thermal properties "
                               "default to zero so a moisture-only case need not invent them. Curves are lists of "
                               "(x, y) pairs.")
            .def(py::init([](std::string name,
                             const double diffusionResistanceFactor,
                             Curve sorptionCurve,
                             Curve liquidTransportCurve,
                             Curve muCurve,
                             const double density,
                             const double heatCapacity,
                             const double thermalConductivity,
                             const double thermalConductivityBeta,
                             const double thermalConductivityMoistureSlope,
                             const double porosity) {
                     return Material{std::move(name),
                                     diffusionResistanceFactor,
                                     std::move(sorptionCurve),
                                     std::move(liquidTransportCurve),
                                     std::move(muCurve),
                                     density,
                                     heatCapacity,
                                     thermalConductivity,
                                     thermalConductivityBeta,
                                     thermalConductivityMoistureSlope,
                                     porosity};
                 }),
                 py::arg("name"),
                 py::arg("diffusion_resistance_factor"),
                 py::arg("sorption_curve"),
                 py::arg("liquid_transport_curve") = Curve{},
                 py::arg("mu_curve") = Curve{},
                 py::arg("density") = 0.0,
                 py::arg("heat_capacity") = 0.0,
                 py::arg("thermal_conductivity") = 0.0,
                 py::arg("thermal_conductivity_beta") = 0.0,
                 py::arg("thermal_conductivity_moisture_slope") = 0.0,
                 py::arg("porosity") = 0.0)
            .def_readwrite("name", &Material::name)
            .def_readwrite("diffusion_resistance_factor", &Material::diffusionResistanceFactor)
            .def_readwrite("sorption_curve", &Material::sorptionCurve)
            .def_readwrite("liquid_transport_curve", &Material::liquidTransportCurve)
            .def_readwrite("mu_curve", &Material::muCurve)
            .def_readwrite("density", &Material::density)
            .def_readwrite("heat_capacity", &Material::heatCapacity)
            .def_readwrite("thermal_conductivity", &Material::thermalConductivity)
            .def_readwrite("thermal_conductivity_beta", &Material::thermalConductivityBeta)
            .def_readwrite("thermal_conductivity_moisture_slope", &Material::thermalConductivityMoistureSlope)
            .def_readwrite("porosity", &Material::porosity)};
        addEquality(material);

        authoring.def("material_uuid", &materialUuid, py::arg("name"),
                      "The record UUID for a material name, the same on every machine and run.");
        authoring.def("material_color", &materialColor, py::arg("name"),
                      "THERM's polygon fill colour for a material name, 0xRRGGBB, the same in every archive.");
        authoring.def("max_water_content", &maxWaterContent, py::arg("material"),
                      "The isotherm's last value: the water-content axis end of every w-keyed table.");
        authoring.def("water_content", &waterContent, py::arg("material"), py::arg("humidity"),
                      "The sorption isotherm at a humidity: linear interpolation, clamped at the table's ends.");
        authoring.def("resistance_factor_by_water_content", &resistanceFactorByWaterContent, py::arg("material"),
                      "The mu(phi) curve re-keyed by water content through the isotherm; empty without a curve.");
        authoring.def("library_material", &libraryMaterial, py::arg("material"),
                      "The authoring material as a THERM library record, ready for a MaterialsDB.");
        authoring.def("materials_database", &materialsDatabase, py::arg("materials"),
                      "A materials library holding the given materials, one record per distinct name.");
    }

    void bindCase(py::module_ & authoring)
    {
        auto modelCase{
          py::class_<ModelCase>(authoring,
                                "ModelCase",
                                "A THERM model stated in code: regions, segments, start, clock, settings, and the "
                                "materials the regions name, keyed by whatever the regions call them.")
            .def(py::init([](std::string id,
                             std::vector<Region> regions,
                             std::vector<Segment> segments,
                             const Initial & initial,
                             const Schedule & schedule,
                             const Physics & physics,
                             const Numerics & numerics,
                             std::map<std::string, Material> materials,
                             std::string title) {
                     return ModelCase{std::move(id),
                                      std::move(title),
                                      std::move(regions),
                                      std::move(segments),
                                      initial,
                                      schedule,
                                      physics,
                                      numerics,
                                      std::move(materials)};
                 }),
                 py::arg("id"),
                 py::arg("regions"),
                 py::arg("segments"),
                 py::arg("initial"),
                 py::arg("schedule"),
                 py::arg("physics") = Physics{},
                 py::arg("numerics") = Numerics{},
                 py::arg("materials") = std::map<std::string, Material>{},
                 py::arg("title") = std::string{})
            .def_readwrite("id", &ModelCase::id)
            .def_readwrite("title", &ModelCase::title)
            .def_readwrite("regions", &ModelCase::regions)
            .def_readwrite("segments", &ModelCase::segments)
            .def_readwrite("initial", &ModelCase::initial)
            .def_readwrite("schedule", &ModelCase::schedule)
            .def_readwrite("physics", &ModelCase::physics)
            .def_readwrite("numerics", &ModelCase::numerics)
            .def_readwrite("materials", &ModelCase::materials)};
        addEquality(modelCase);

        authoring.def("issues", &issues, py::arg("model_case"),
                      "Everything that stops the case from being written, one line each; empty when it can be.");
        authoring.def("material", &material, py::arg("model_case"), py::arg("name"),
                      "The material a region names, or None if the case does not carry it.");
        authoring.def("region_material", &regionMaterial, py::arg("model_case"), py::arg("index"),
                      "The material filling the region at the given index, or None.");
        authoring.def("record_name", &recordName, py::arg("boundary"),
                      "The library record a boundary kind is written as; the same in every generated file.");
    }

    void bindArchive(py::module_ & authoring)
    {
        py::class_<Libraries>(authoring, "Libraries", "Everything a model refers to by UUID.")
          .def_readonly("materials", &Libraries::materials)
          .def_readonly("boundary_conditions", &Libraries::boundaryConditions)
          .def_readonly("datasets", &Libraries::datasets);

        authoring.attr("DRAWING_ORIGIN_MM") = py::make_tuple(drawingOriginMm.x, drawingOriginMm.y);

        authoring.def("case_uuid", &caseUuid, py::arg("model_case"), py::arg("key"),
                      "A UUID that depends on the case id and a key, stable across runs.");
        authoring.def("record_uuid", &recordUuid, py::arg("record_name"),
                      "The UUID of a boundary-condition record, the same in every generated file.");
        authoring.def("boundary_record", &boundaryRecord, py::arg("boundary"), py::arg("project"),
                      "The library record for a boundary kind, marked as the project's own.");
        authoring.def("series_values", &seriesValues, py::arg("boundary"),
                      "(role name, constant value) pairs, one per series the segment's dataset carries, in the "
                      "order the dataset writes them. Empty for a sealed segment.");
        authoring.def("boundary_datasets", &boundaryDatasets, py::arg("model_case"),
                      "Segment index -> dataset for every segment that reads values, plus a clock on the first "
                      "segment when none does.");
        authoring.def("build_model", &buildModel, py::arg("model_case"),
                      "The THERM model of the case: polygons, boundary segments and calculation options. The case "
                      "must have no issues.");
        authoring.def("build_libraries", &buildLibraries, py::arg("model_case"),
                      "The materials, the boundary-condition records and the datasets the model uses. The case "
                      "must have no issues.");
        authoring.def(
          "archive_entries",
          [](const ModelCase & modelCase) {
              const auto entries{archiveEntries(modelCase)};
              if(!entries.has_value())
              {
                  throw py::value_error(entries.error());
              }
              return entries.value();
          },
          py::arg("model_case"),
          "Every entry of the case's THMZ, name -> content, before zipping. Raises ValueError with the case's "
          "issues when it cannot be written.");
        authoring.def(
          "write_archive",
          [](const ModelCase & modelCase, const std::string & path) {
              const auto written{writeArchive(modelCase, path)};
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
        authoring.def("schedule_of", &scheduleOf, py::arg("datasets"),
                      "The clock the mediator will run, read from the first dataset as the mediator does.");
    }
}   // namespace

//! Stating a THERM model in physical terms and writing it as the archive THERM would have
//! saved: regions in metres filled with named materials, segments carrying a physical
//! condition, a uniform start, a clock and the calculation switches. The file-level types
//! bound elsewhere in this module are what the archive is made of; this is the layer above.
void bind_authoring(py::module_ & mod)
{
    auto authoring{mod.def_submodule(
      "authoring", "State a THERM model in physical terms and write it as a THMZ; see pylibraryfemtherm docs.")};
    bindGeometry(authoring);
    bindBoundaryKinds(authoring);
    bindSettings(authoring);
    bindMaterial(authoring);
    bindCase(authoring);
    bindArchive(authoring);
}
