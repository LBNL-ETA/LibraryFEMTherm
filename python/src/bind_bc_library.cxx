#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "BoundaryConditions/BoundaryConditions.hxx"
#include "BoundaryConditions/DB.hxx"
#include "BoundaryConditions/Definitions.hxx"
#include "BoundaryConditions/Utils.hxx"

namespace py = pybind11;

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
        .def("__repr__", [](const BCLibrary::Constant & constant) {
            return "Constant(" + std::to_string(constant.value) + ")";
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
        .def("__repr__", [](const BCLibrary::FromTimeSeries & source) {
            return "FromTimeSeries(SeriesRole." + std::string(py::str(py::cast(source.role))) + ")";
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

    py::class_<BCLibrary::AutomaticEnclosure>(mod, "BCAutomaticEnclosure")
        .def(py::init<>())
        .def_readwrite("emissivity", &BCLibrary::AutomaticEnclosure::emissivity)
        .def_readwrite("temperature", &BCLibrary::AutomaticEnclosure::temperature);

    py::class_<BCLibrary::ManualEnclosure>(mod, "BCManualEnclosure")
        .def(py::init<>());

    py::class_<BCLibrary::BlackBodyRadiation>(mod, "BCBlackBodyRadiation")
        .def(py::init<>())
        .def_readwrite("temperature", &BCLibrary::BlackBodyRadiation::temperature)
        .def_readwrite("emissivity", &BCLibrary::BlackBodyRadiation::emissivity)
        .def_readwrite("view_factor", &BCLibrary::BlackBodyRadiation::viewFactor);

    py::class_<BCLibrary::FixedCoefficientRadiation>(mod, "BCFixedCoefficientRadiation")
        .def(py::init<>())
        .def_readwrite("temperature", &BCLibrary::FixedCoefficientRadiation::temperature)
        .def_readwrite("coefficient", &BCLibrary::FixedCoefficientRadiation::coefficient);

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
        .def("load_from_zip_file", &BCLibrary::DB::loadFromZipFile, py::arg("zip_file_name"),
             "Reads the archive's BoundaryConditions entry; leaves the DB unchanged when absent.")
        .def("load_from_entries", &BCLibrary::DB::loadFromEntries, py::arg("entries"),
             "Same, from an already-extracted archive dict.")
        .def("save_to_zip_file", &BCLibrary::DB::saveToZipFile, py::arg("zip_file_name"),
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
