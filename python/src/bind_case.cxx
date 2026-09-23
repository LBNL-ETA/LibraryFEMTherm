#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <format>
#include <string>

#include "Model/Archive.hxx"
#include "Model/Materials.hxx"
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
                                       "once and the polygon closed implicitly, and the material filling it. "
                                       "Regions that share a material state the same one; the archive's library "
                                       "keeps one record per material name.")
                      .def(py::init([](Material material, std::vector<Point> points) {
                               return Region{std::move(material), std::move(points)};
                           }),
                           py::arg("material"),
                           py::arg("points"))
                      .def_readwrite("material", &Region::material)
                      .def_readwrite("points", &Region::points)};
        addEquality(region);

        auto segment{py::class_<Segment>(model,
                                         "Segment",
                                         "One boundary segment: a straight line between two points on a region's "
                                         "outline, the condition it carries (Adiabatic, Prescribed or Convective), and "
                                         "optionally the index of the region it bounds. Left None, the region is "
                                         "found: a segment on the outline lies on exactly one region's edge. It must "
                                         "be given where two regions share the edge.")
                       .def(py::init([](Boundary kind,
                                        const Point & start,
                                        const Point & end,
                                        const std::optional<std::size_t> region) {
                                return Segment{std::move(kind), start, end, region};
                            }),
                            py::arg("kind"),
                            py::arg("start"),
                            py::arg("end"),
                            py::arg("region") = py::none())
                       .def_readwrite("kind", &Segment::kind)
                       .def_readwrite("start", &Segment::start)
                       .def_readwrite("end", &Segment::end)
                       .def_readwrite("region", &Segment::region)};
        addEquality(segment);
    }

    void bindBoundaryKinds(py::module_ & model)
    {
        auto adiabatic{py::class_<Adiabatic>(model,
                                             "Adiabatic",
                                             "No exchange at all: no heat and no moisture crosses the surface. "
                                             "Written as THERM's own built-in Adiabatic record.")
                         .def(py::init<>())
                         .def("__repr__", [](const Adiabatic &) { return std::string{"Adiabatic()"}; })};
        addEquality(adiabatic);

        auto prescribed{
          py::class_<Prescribed>(model,
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
          py::class_<Convective>(model,
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
                       .def_readwrite("humidity", &Initial::humidity)};
        addEquality(initial);

        auto schedule{py::class_<Schedule>(model, "Schedule", "The clock: n_steps steps of dtime seconds.")
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
            .def_readwrite("conductivity_dependent", &Physics::conductivityDependent)};
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
                        .def_readwrite("max_iterations", &Numerics::maxIterations)};
        addEquality(numerics);
    }

    void bindMaterial(py::module_ & model)
    {
        auto material{
          py::class_<Material>(model,
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
    }

    void bindMaterialRecords(py::module_ & build)
    {
        build.def("material_uuid", &materialUuid, py::arg("name"),
                  "The record UUID for a material name, the same on every machine and run.");
        build.def("material_color", &materialColor, py::arg("name"),
                  "THERM's polygon fill colour for a material name, 0xRRGGBB, the same in every archive.");
        build.def("max_water_content", &maxWaterContent, py::arg("material"),
                  "The isotherm's last value: the water-content axis end of every w-keyed table.");
        build.def("water_content", &waterContent, py::arg("material"), py::arg("humidity"),
                  "The sorption isotherm at a humidity: linear interpolation, clamped at the table's ends.");
        build.def("resistance_factor_by_water_content", &resistanceFactorByWaterContent, py::arg("material"),
                  "The mu(phi) curve re-keyed by water content through the isotherm; empty without a curve.");
        build.def("library_material", &libraryMaterial, py::arg("material"),
                  "The model material as a THERM library record, ready for a MaterialsDB.");
        build.def("materials_database", &materialsDatabase, py::arg("materials"),
                  "A materials library holding the given materials, one record per distinct name.");
    }

    void bindCase(py::module_ & model)
    {
        auto modelCase{
          py::class_<ModelCase>(model,
                                "ModelCase",
                                "A THERM model stated in code: regions, each carrying its material, segments, "
                                "start, clock and settings.")
            .def(py::init([](std::string id,
                             std::vector<Region> regions,
                             std::vector<Segment> segments,
                             const Initial & initial,
                             const Schedule & schedule,
                             const Physics & physics,
                             const Numerics & numerics,
                             std::string title) {
                     return ModelCase{std::move(id),
                                      std::move(title),
                                      std::move(regions),
                                      std::move(segments),
                                      initial,
                                      schedule,
                                      physics,
                                      numerics};
                 }),
                 py::arg("id"),
                 py::arg("regions"),
                 py::arg("segments"),
                 py::arg("initial"),
                 py::arg("schedule"),
                 py::arg("physics") = Physics{},
                 py::arg("numerics") = Numerics{},
                 py::arg("title") = std::string{})
            .def_readwrite("id", &ModelCase::id)
            .def_readwrite("title", &ModelCase::title)
            .def_readwrite("regions", &ModelCase::regions)
            .def_readwrite("segments", &ModelCase::segments)
            .def_readwrite("initial", &ModelCase::initial)
            .def_readwrite("schedule", &ModelCase::schedule)
            .def_readwrite("physics", &ModelCase::physics)
            .def_readwrite("numerics", &ModelCase::numerics)};
        addEquality(modelCase);

        model.def("issues", &issues, py::arg("model_case"),
                      "Everything that stops the case from being written, one line each; empty when it can be.");
        model.def("region_material", &regionMaterial, py::arg("model_case"), py::arg("index"),
                      "The material filling the region at the given index, or None when there is no such region.");
        model.def("segment_region", &segmentRegion, py::arg("model_case"), py::arg("index"),
                      "The region the segment at the given index is attached to: the one it names, or the "
                      "only region whose edge carries it. None when issues() would report the segment.");
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
        build.def("record_name", &recordName, py::arg("boundary"),
                  "The library record a boundary kind is written as; the same in every generated file.");
        build.def("record_uuid", &recordUuid, py::arg("record_name"),
                  "The UUID of a boundary-condition record, the same in every generated file.");
        build.def("boundary_record", &boundaryRecord, py::arg("boundary"), py::arg("project"),
                  "The library record for a boundary kind: THERM's built-in for Adiabatic, otherwise marked as "
                  "the project's own.");
        build.def("series_values", &seriesValues, py::arg("boundary"),
                  "(role name, constant value) pairs, one per series the segment's dataset carries, in the "
                  "order the dataset writes them. Empty for an adiabatic segment.");
        build.def("boundary_datasets", &boundaryDatasets, py::arg("model_case"),
                  "Segment index -> dataset for every segment that reads values, plus a clock on the first "
                  "segment when none does.");
        build.def("model", &model, py::arg("model_case"),
                  "The THERM model of the case: polygons, boundary segments and calculation options. The case "
                  "must have no issues.");
        build.def("libraries", &libraries, py::arg("model_case"),
                  "The materials, the boundary-condition records and the datasets the model uses. The case "
                  "must have no issues.");
        build.def(
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
        build.def(
          "archive",
          [](const ModelCase & modelCase, const std::string & path) {
              const auto written{archive(modelCase, path)};
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
//! carrying their material, segments carrying a physical condition, a uniform start, a clock
//! and the calculation switches. `build` turns such a case into what THERM would have saved:
//! the model, its libraries, the archive. The file-level types bound elsewhere in this
//! module are what the archive is made of; these two are the layer above.
void bind_case(py::module_ & mod)
{
    auto model{mod.def_submodule("model", "State a THERM model in physical terms; see pylibraryfemtherm docs.")};
    // Material before Geometry: a Region takes one, and pybind11 names a parameter's type
    // in the signature only if the type is already registered.
    bindMaterial(model);
    bindGeometry(model);
    bindBoundaryKinds(model);
    bindSettings(model);
    bindCase(model);

    auto build{mod.def_submodule("build", "Turn a stated model into the archive THERM would have saved.")};
    bindMaterialRecords(build);
    bindArchive(build);
}
