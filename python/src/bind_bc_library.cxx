#include <optional>
#include <string>
#include <variant>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "BoundaryConditions/BoundaryConditions.hxx"
#include "BoundaryConditions/Capability.hxx"
#include "BoundaryConditions/DB.hxx"
#include "BoundaryConditions/Definitions.hxx"
#include "BoundaryConditions/Utils.hxx"

#include "SourceLike.hxx"

namespace py = pybind11;

using SourceBinding::SourceLike;
using SourceBinding::sourceRepr;
using SourceBinding::toSource;

//! The consolidated boundary-condition library (BC consolidation): one record shape for
//! steady-state and transient use. Every scalar input is a Source, either a Constant
//! or a FromTimeSeries reference to a dataset role; a record whose inputs are all
//! constants is steady-state capable. Stored in a THMZ as the BoundaryConditions entry.
//!
//! Depends on bind_time_series (SeriesRole), which must be registered first.
void bind_bc_library(py::module_ & mod)
{
    // "ConvectionModel" is taken by the frame-cavity enum in bind_enums.
    py::enum_<BCLibrary::ConvectionModel>(mod, "BCConvectionModel")
        .value("Fixed_Convection_Coefficient", BCLibrary::ConvectionModel::Fixed_Convection_Coefficient)
        .value("ASHRAE_NFRC_Inside", BCLibrary::ConvectionModel::ASHRAE_NFRC_Inside)
        .value("ASHRAE_NFRC_Outside", BCLibrary::ConvectionModel::ASHRAE_NFRC_Outside)
        .value("TARP", BCLibrary::ConvectionModel::TARP)
        .value("Yazdanian_Klems", BCLibrary::ConvectionModel::Yazdanian_Klems)
        .value("Kimura", BCLibrary::ConvectionModel::Kimura)
        .value("Montazeri", BCLibrary::ConvectionModel::Montazeri);

    // --- Source: Constant | FromTimeSeries ---
    // A plain number or a SeriesRole is accepted wherever a Source is expected, so
    // `record.temperature = 20.0` and `record.temperature = SeriesRole.AirTemperature`
    // both work; reading back always yields the wrapped object.
    py::class_<BCLibrary::Constant>(mod, "Constant", "A fixed input value.")
        .def(py::init<>())
        .def(py::init<double>(), py::arg("value"))
        .def_readwrite("value", &BCLibrary::Constant::value)
        .def("__eq__", [](const BCLibrary::Constant & lhs, const BCLibrary::Constant & rhs) { return lhs == rhs; }, py::is_operator())
        .def("__repr__", [](const BCLibrary::Constant & constant) {
            return "Constant(" + py::repr(py::cast(constant.value)).cast<std::string>() + ")";
        });
    // Guarded by the Python type, not by convertibility: a pybind11 enum converts to int,
    // so a looser rule would turn a SeriesRole into Constant before FromTimeSeries is tried
    // (Constant is the variant's first alternative).
    py::implicitly_convertible<py::float_, BCLibrary::Constant>();
    py::implicitly_convertible<py::int_, BCLibrary::Constant>();

    py::class_<BCLibrary::FromTimeSeries>(mod, "FromTimeSeries",
                                          "An input read from the bound dataset's series of this role.")
        .def(py::init<>())
        .def(py::init<TimeSeriesLibrary::SeriesRole>(), py::arg("role"))
        .def_readwrite("role", &BCLibrary::FromTimeSeries::role)
        .def("__eq__", [](const BCLibrary::FromTimeSeries & lhs, const BCLibrary::FromTimeSeries & rhs) { return lhs == rhs; }, py::is_operator())
        .def("__repr__", [](const BCLibrary::FromTimeSeries & source) {
            return "FromTimeSeries(" + std::string(py::str(py::cast(source.role))) + ")";
        });
    py::implicitly_convertible<TimeSeriesLibrary::SeriesRole, BCLibrary::FromTimeSeries>();

    mod.def("is_constant", &BCLibrary::isConstant, py::arg("source"));
    mod.def("is_from_time_series", &BCLibrary::isFromTimeSeries, py::arg("source"));

    // --- Surface exchange components ---
    py::class_<BCLibrary::Convection>(mod, "BCConvection")
        .def(py::init<>())
        .def_readwrite("model", &BCLibrary::Convection::model)
        .def_readwrite("air_temperature", &BCLibrary::Convection::airTemperature)
        .def_readwrite("film_coefficient", &BCLibrary::Convection::filmCoefficient)
        .def_readwrite("wind_speed", &BCLibrary::Convection::windSpeed)
        .def_readwrite("wind_direction", &BCLibrary::Convection::windDirection);

    py::class_<BCLibrary::AutomaticEnclosure>(
      mod,
      "BCAutomaticEnclosure",
      "Radiation to the other surfaces of the enclosure the solver finds around this one. `temperature` closes "
      "the enclosure; left None, it follows the convection's air temperature.")
        .def(py::init<>())
        .def(py::init([](const SourceLike & emissivity, const std::optional<SourceLike> & temperature) {
                 BCLibrary::AutomaticEnclosure enclosure;
                 enclosure.emissivity = toSource(emissivity);
                 if(temperature.has_value())
                 {
                     enclosure.temperature = toSource(temperature.value());
                 }
                 return enclosure;
             }),
             py::arg("emissivity"),
             py::arg("temperature") = py::none())
        .def_readwrite("emissivity", &BCLibrary::AutomaticEnclosure::emissivity)
        .def_readwrite("temperature", &BCLibrary::AutomaticEnclosure::temperature)
        .def(
          "__eq__",
          [](const BCLibrary::AutomaticEnclosure & lhs, const BCLibrary::AutomaticEnclosure & rhs) { return lhs == rhs; },
          py::is_operator())
        .def("__repr__", [](const BCLibrary::AutomaticEnclosure & enclosure) {
            const std::string temperature{enclosure.temperature.has_value() ? sourceRepr(enclosure.temperature.value())
                                                                            : std::string{"None"}};
            return "BCAutomaticEnclosure(emissivity=" + sourceRepr(enclosure.emissivity) + ", temperature=" + temperature
                   + ")";
        });

    py::class_<BCLibrary::ManualEnclosure>(mod, "BCManualEnclosure")
        .def(py::init<>());

    py::class_<BCLibrary::BlackBodyRadiation>(
      mod,
      "BCBlackBodyRadiation",
      "Radiation to black-body surroundings at `temperature`, as THERM's exterior records state it.")
        .def(py::init<>())
        .def(py::init([](const SourceLike & temperature, const SourceLike & emissivity, const double viewFactor) {
                 return BCLibrary::BlackBodyRadiation{
                   .temperature = toSource(temperature), .emissivity = toSource(emissivity), .viewFactor = viewFactor};
             }),
             py::arg("temperature"),
             py::arg("emissivity") = 1.0,
             py::arg("view_factor") = 1.0)
        .def_readwrite("temperature", &BCLibrary::BlackBodyRadiation::temperature)
        .def_readwrite("emissivity", &BCLibrary::BlackBodyRadiation::emissivity)
        .def_readwrite("view_factor", &BCLibrary::BlackBodyRadiation::viewFactor)
        .def(
          "__eq__",
          [](const BCLibrary::BlackBodyRadiation & lhs, const BCLibrary::BlackBodyRadiation & rhs) { return lhs == rhs; },
          py::is_operator())
        .def("__repr__", [](const BCLibrary::BlackBodyRadiation & radiation) {
            return "BCBlackBodyRadiation(temperature=" + sourceRepr(radiation.temperature)
                   + ", emissivity=" + sourceRepr(radiation.emissivity)
                   + ", view_factor=" + py::repr(py::cast(radiation.viewFactor)).cast<std::string>() + ")";
        });

    py::class_<BCLibrary::FixedCoefficientRadiation>(
      mod,
      "BCFixedCoefficientRadiation",
      "Radiation linearised to a fixed coefficient in W/(m2 K) towards `temperature`.")
        .def(py::init<>())
        .def(py::init([](const SourceLike & temperature, const SourceLike & coefficient) {
                 return BCLibrary::FixedCoefficientRadiation{.temperature = toSource(temperature),
                                                             .coefficient = toSource(coefficient)};
             }),
             py::arg("temperature"),
             py::arg("coefficient"))
        .def_readwrite("temperature", &BCLibrary::FixedCoefficientRadiation::temperature)
        .def_readwrite("coefficient", &BCLibrary::FixedCoefficientRadiation::coefficient)
        .def(
          "__eq__",
          [](const BCLibrary::FixedCoefficientRadiation & lhs, const BCLibrary::FixedCoefficientRadiation & rhs) {
              return lhs == rhs;
          },
          py::is_operator())
        .def("__repr__", [](const BCLibrary::FixedCoefficientRadiation & radiation) {
            return "BCFixedCoefficientRadiation(temperature=" + sourceRepr(radiation.temperature)
                   + ", coefficient=" + sourceRepr(radiation.coefficient) + ")";
        });

    py::class_<BCLibrary::Solar>(mod, "BCSolar")
        .def(py::init<>())
        .def_readwrite("irradiance", &BCLibrary::Solar::irradiance)
        .def_readwrite("absorptance", &BCLibrary::Solar::absorptance);

    // --- Record kinds ---
    py::class_<BCLibrary::SurfaceExchange>(mod, "SurfaceExchange",
                                           "Flux-type exchange with an environment: humidity, convection, "
                                           "radiation, solar, heat flux. Neumann and Robin conditions.")
        .def(py::init<>())
        .def_readwrite("relative_humidity", &BCLibrary::SurfaceExchange::relativeHumidity)
        .def_readwrite("convection", &BCLibrary::SurfaceExchange::convection)
        .def_readwrite("radiation", &BCLibrary::SurfaceExchange::radiation)
        .def_readwrite("solar", &BCLibrary::SurfaceExchange::solar)
        .def_readwrite("flux", &BCLibrary::SurfaceExchange::flux);

    py::class_<BCLibrary::PrescribedState>(mod, "PrescribedState",
                                           "The surface state itself is imposed: temperature, humidity "
                                           "or both. Dirichlet conditions.")
        .def(py::init<>())
        .def_readwrite("temperature", &BCLibrary::PrescribedState::temperature)
        .def_readwrite("relative_humidity", &BCLibrary::PrescribedState::relativeHumidity);

    py::class_<BCLibrary::RadiationSurface>(mod, "BCRadiationSurface")
        .def(py::init<>())
        .def_readwrite("is_default", &BCLibrary::RadiationSurface::isDefault)
        .def_readwrite("temperature", &BCLibrary::RadiationSurface::temperature)
        .def_readwrite("emissivity", &BCLibrary::RadiationSurface::emissivity);

    py::class_<BCLibrary::NoExchange>(mod, "NoExchange",
                                      "Adiabatic and moisture-tight: deliberately no exchange at all.")
        .def(py::init<>());

    py::class_<BCLibrary::BoundaryCondition>(mod, "BoundaryCondition",
                                             "One library record. `data` is a SurfaceExchange, "
                                             "PrescribedState, BCRadiationSurface or NoExchange.")
        .def(py::init<>())
        .def_readwrite("uuid", &BCLibrary::BoundaryCondition::UUID)
        .def_readwrite("name", &BCLibrary::BoundaryCondition::Name)
        .def_readwrite("protected_", &BCLibrary::BoundaryCondition::Protected)
        .def_readwrite("color", &BCLibrary::BoundaryCondition::Color)
        .def_readwrite("data", &BCLibrary::BoundaryCondition::data)
        .def_readwrite("project_name", &BCLibrary::BoundaryCondition::ProjectName)
        .def_readwrite("is_igu_surface", &BCLibrary::BoundaryCondition::isIGUSurface);

    mod.def("required_roles", &BCLibrary::requiredRoles, py::arg("boundary_condition"),
            "The dataset roles the record reads; empty means steady-state capable.");
    mod.def("is_steady_capable", &BCLibrary::isSteadyCapable, py::arg("boundary_condition"));
    mod.def("missing_roles", &BCLibrary::missingRoles, py::arg("boundary_condition"), py::arg("dataset") = py::none(),
            "The roles the record reads that the dataset does not provide: the check a segment's pairing of "
            "record and dataset must pass before a run. With no dataset, every role read. A missing "
            "RadiantTemperature is taken as the air temperature when the dataset carries one.");

    py::class_<BCLibrary::CapabilityIssue>(mod, "CapabilityIssue",
                                           "One reason a record cannot run under an engine and mode, anchored to the "
                                           "feature at fault.")
        .def_readonly("feature", &BCLibrary::CapabilityIssue::feature)
        .def_readonly("reason", &BCLibrary::CapabilityIssue::reason)
        .def("__repr__", [](const BCLibrary::CapabilityIssue & issue) {
            return "CapabilityIssue(feature=" + py::repr(py::cast(issue.feature)).cast<std::string>()
                   + ", reason=" + py::repr(py::cast(issue.reason)).cast<std::string>() + ")";
        });
    mod.def("capability_issues", &BCLibrary::capabilityIssues, py::arg("boundary_condition"), py::arg("engine"), py::arg("mode"),
            "Every reason the record cannot run under the engine and calculation mode; empty when it can. "
            "The one gate THERM's assignment, engine switch and simulate pre-flight all go through.");
    mod.def("usable", &BCLibrary::usable, py::arg("boundary_condition"), py::arg("engine"), py::arg("mode"));
    mod.def("is_adiabatic", &BCLibrary::isAdiabatic, py::arg("boundary_condition"));
    mod.def("is_convective", &BCLibrary::isConvective, py::arg("boundary_condition"));
    mod.def("is_radiation_bc", &BCLibrary::isRadiationBC, py::arg("boundary_condition"));

    // --- DB ---
    py::class_<BCLibrary::DB>(mod, "BoundaryConditionsDB",
                              "The consolidated boundary-condition library.")
        .def(py::init<>())
        .def(py::init<const std::string &>(), py::arg("xml_file_name"))
        .def("load_from_string", &BCLibrary::DB::loadFromString, py::arg("str"))
        .def("save_to_string", &BCLibrary::DB::saveToString,
             py::arg("format") = FileParse::FileFormat::XML)
        .def("load", &BCLibrary::DB::loadFromZipFile, py::arg("thmz_file_name"),
             "Reads the archive's BoundaryConditions entry; leaves the DB unchanged when absent.")
        .def("load_from_entries", &BCLibrary::DB::loadFromEntries, py::arg("entries"),
             "Same, from an already-extracted archive dict.")
        .def("save", &BCLibrary::DB::saveToZipFile, py::arg("thmz_file_name"),
             py::arg("format") = FileParse::FileFormat::XML)
        .def("get_boundary_conditions", &BCLibrary::DB::getBoundaryConditions,
             py::return_value_policy::reference_internal)
        .def("get_by_uuid", &BCLibrary::DB::getByUUID, py::arg("uuid"))
        .def("get_by_name", &BCLibrary::DB::getByName, py::arg("name"))
        .def("get_by_display_name", &BCLibrary::DB::getByDisplayName, py::arg("display_name"))
        .def("get_names", &BCLibrary::DB::getNames)
        .def("get_display_names", &BCLibrary::DB::getDisplayNames)
        .def("add", &BCLibrary::DB::add, py::arg("condition"))
        .def("update", &BCLibrary::DB::update, py::arg("condition"))
        .def("update_or_add", &BCLibrary::DB::updateOrAdd, py::arg("condition"))
        .def("delete_with_uuid", &BCLibrary::DB::deleteWithUUID, py::arg("uuid"))
        .def("get_default_record", &BCLibrary::DB::getDefaultRecord)
        .def("get_default_radiation_surface", &BCLibrary::DB::getDefaultRadiationSurface)
        .def("is_dirty", &BCLibrary::DB::isDirty);
}
