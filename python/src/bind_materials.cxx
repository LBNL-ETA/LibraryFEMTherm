#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Materials/DB.hxx"
#include "Materials/FromValues.hxx"

namespace py = pybind11;

void bind_materials(py::module_ & mod)
{
    // --- Supporting structs ---
    py::class_<MaterialsLibrary::point>(mod, "MaterialPoint")
        .def(py::init<>())
        .def(py::init<double, double>())
        .def_readwrite("x", &MaterialsLibrary::point::x)
        .def_readwrite("y", &MaterialsLibrary::point::y);

    py::class_<MaterialsLibrary::HygroThermal>(mod, "HygroThermal")
        .def(py::init<>())
        .def_readwrite("default_thickness", &MaterialsLibrary::HygroThermal::DefaultThickness)
        .def_readwrite("material_information", &MaterialsLibrary::HygroThermal::MaterialInformation)
        .def_readwrite("bulk_density", &MaterialsLibrary::HygroThermal::BulkDensity)
        .def_readwrite("porosity", &MaterialsLibrary::HygroThermal::Porosity)
        .def_readwrite("specific_heat_capacity_dry", &MaterialsLibrary::HygroThermal::SpecificHeatCapacityDry)
        .def_readwrite("thermal_conductivity_dry", &MaterialsLibrary::HygroThermal::ThermalConductivityDry)
        .def_readwrite("roughness", &MaterialsLibrary::HygroThermal::Roughness)
        .def_readwrite("water_vapor_diffusion_resistance_factor", &MaterialsLibrary::HygroThermal::WaterVaporDiffusionResistanceFactor)
        .def_readwrite("moisture_storage_function", &MaterialsLibrary::HygroThermal::MoistureStorageFunction)
        .def_readwrite("liquid_transportation_coefficient_suction", &MaterialsLibrary::HygroThermal::LiquidTransportationCoefficientSuction)
        .def_readwrite("liquid_transportation_coefficient_redistribution", &MaterialsLibrary::HygroThermal::LiquidTransportationCoefficientRedistribution)
        .def_readwrite("thermal_conductivity_moisture_dependent", &MaterialsLibrary::HygroThermal::ThermalConductivityMoistureDependent)
        .def_readwrite("water_vapor_diffusion_resistance_factor_moisture_dependent",
                       &MaterialsLibrary::HygroThermal::WaterVaporDiffusionResistanceFactorMoistureDependent)
        .def_readwrite("thermal_conductivity_temperature_dependent", &MaterialsLibrary::HygroThermal::ThermalConductivityTemperatureDependent);

    py::class_<MaterialsLibrary::OpticalProperties>(mod, "OpticalProperties")
        .def(py::init<>())
        .def_readwrite("transmittance", &MaterialsLibrary::OpticalProperties::Transmittance)
        .def_readwrite("reflectance", &MaterialsLibrary::OpticalProperties::Reflectance);

    py::class_<MaterialsLibrary::OpticalSide>(mod, "OpticalSide")
        .def(py::init<>())
        .def_readwrite("front", &MaterialsLibrary::OpticalSide::Front)
        .def_readwrite("back", &MaterialsLibrary::OpticalSide::Back);

    py::class_<MaterialsLibrary::OpticalType>(mod, "OpticalType")
        .def(py::init<>())
        .def_readwrite("direct", &MaterialsLibrary::OpticalType::Direct)
        .def_readwrite("diffuse", &MaterialsLibrary::OpticalType::Diffuse);

    py::class_<MaterialsLibrary::Infrared>(mod, "Infrared")
        .def(py::init<>())
        .def_readwrite("transmittance", &MaterialsLibrary::Infrared::Transmittance)
        .def_readwrite("emissivity_front", &MaterialsLibrary::Infrared::EmissivityFront)
        .def_readwrite("emissivity_back", &MaterialsLibrary::Infrared::EmissivityBack);

    py::class_<MaterialsLibrary::Integrated>(mod, "Integrated")
        .def(py::init<>())
        .def_readwrite("ir", &MaterialsLibrary::Integrated::IR)
        .def_readwrite("solar", &MaterialsLibrary::Integrated::Solar)
        .def_readwrite("visible", &MaterialsLibrary::Integrated::Visible);

    py::class_<MaterialsLibrary::Optical>(mod, "Optical")
        .def(py::init<>())
        .def_readwrite("integrated", &MaterialsLibrary::Optical::integrated);

    py::class_<MaterialsLibrary::Solid>(mod, "Solid")
        .def(py::init<>())
        .def_readwrite("hygro_thermal", &MaterialsLibrary::Solid::hygroThermal)
        .def_readwrite("optical", &MaterialsLibrary::Solid::optical);

    // --- Database source ---
    py::class_<MaterialsLibrary::WINDOW>(mod, "WINDOW")
        .def(py::init<>())
        .def_readwrite("path", &MaterialsLibrary::WINDOW::Path)
        .def_readwrite("name", &MaterialsLibrary::WINDOW::Name)
        .def_readwrite("id", &MaterialsLibrary::WINDOW::ID);

    py::class_<MaterialsLibrary::Database>(mod, "Database")
        .def(py::init<>())
        .def_readwrite("window", &MaterialsLibrary::Database::Window);

    // --- Material ---
    py::class_<MaterialsLibrary::Material>(
      mod,
      "Material",
      "One record of THERM's materials library: the same object a library holds, a file carries and a model's "
      "region is filled with. Build it empty and fill it field by field, or from physical values by keyword: "
      "Material(name=..., diffusion_resistance_factor=..., sorption_curve=[(humidity, water content), ...], "
      "density=..., heat_capacity=..., thermal_conductivity=..., color=...), which writes THERM's tables and "
      "derives the UUID from the name.")
        .def(py::init<>())
        .def(py::init([](const std::string & name,
                         const double diffusionResistanceFactor,
                         const MaterialsLibrary::Curve & sorptionCurve,
                         const MaterialsLibrary::Curve & liquidTransportCurve,
                         const MaterialsLibrary::Curve & muCurve,
                         const double density,
                         const double heatCapacity,
                         const double thermalConductivity,
                         const double thermalConductivityBeta,
                         const double thermalConductivityMoistureSlope,
                         const double porosity,
                         const std::optional<std::string> & color) {
                 return MaterialsLibrary::fromValues(name,
                                                   diffusionResistanceFactor,
                                                   sorptionCurve,
                                                   liquidTransportCurve,
                                                   muCurve,
                                                   density,
                                                   heatCapacity,
                                                   thermalConductivity,
                                                   thermalConductivityBeta,
                                                   thermalConductivityMoistureSlope,
                                                   porosity,
                                                   color);
             }),
             py::arg("name"),
             py::arg("diffusion_resistance_factor"),
             py::arg("sorption_curve"),
             py::arg("liquid_transport_curve") = MaterialsLibrary::Curve{},
             py::arg("mu_curve") = MaterialsLibrary::Curve{},
             py::arg("density") = 0.0,
             py::arg("heat_capacity") = 0.0,
             py::arg("thermal_conductivity") = 0.0,
             py::arg("thermal_conductivity_beta") = 0.0,
             py::arg("thermal_conductivity_moisture_slope") = 0.0,
             py::arg("porosity") = 0.0,
             py::arg("color") = py::none(),
             "The record for a material stated in physical terms. Required: a name, a vapour resistance factor "
             "and a sorption isotherm as (humidity, water content) pairs; the thermal properties default to zero "
             "so a moisture-only case need not invent them. A linear k(T) or k(w) law becomes a two-point table, "
             "a constant a single point, and the mu(phi) curve is re-keyed by water content. `color` is THERM's "
             "fill, 0xRRGGBB; left None, one derived from the name. The UUID is derived from the name, so "
             "regenerating a case never makes a second record.")
        .def_readwrite("uuid", &MaterialsLibrary::Material::UUID)
        .def_readwrite("name", &MaterialsLibrary::Material::Name)
        .def_readwrite("project_name", &MaterialsLibrary::Material::ProjectName)
        .def_readwrite("protected_", &MaterialsLibrary::Material::Protected)
        .def_readwrite("color", &MaterialsLibrary::Material::Color)
        .def_readwrite("database", &MaterialsLibrary::Material::database)
        .def_readwrite("data", &MaterialsLibrary::Material::data);

    mod.def("material_uuid", &MaterialsLibrary::materialUuid, py::arg("name"),
            "The record UUID for a material name, the same on every machine and run.");
    mod.def("material_color", &MaterialsLibrary::materialColor, py::arg("name"),
            "THERM's polygon fill colour derived from a material name, 0xRRGGBB, the same in every archive.");
    mod.def("materials_database", &MaterialsLibrary::materialsDatabase, py::arg("records"),
            "A materials library holding the given records, one per distinct UUID.");

    // --- DB ---
    py::class_<MaterialsLibrary::DB>(mod, "MaterialsDB")
        .def(py::init<>())
        .def(py::init<const std::string &>(), py::arg("xml_file_name"))
        .def("load_from_string", &MaterialsLibrary::DB::loadFromString, py::arg("str"))
        .def("save_to_string", &MaterialsLibrary::DB::saveToString,
             py::arg("format") = FileParse::FileFormat::XML)
        .def("get_materials",
             py::overload_cast<>(&MaterialsLibrary::DB::getMaterials, py::const_),
             py::return_value_policy::reference_internal)
        .def("get_by_name", &MaterialsLibrary::DB::getByName, py::arg("name"))
        .def("get_by_uuid", &MaterialsLibrary::DB::getByUUID, py::arg("uuid"))
        .def("get_default_record", &MaterialsLibrary::DB::getDefaultRecord)
        .def("get_names", &MaterialsLibrary::DB::getNames)
        .def("get_display_names", &MaterialsLibrary::DB::getDisplayNames)
        .def("add", &MaterialsLibrary::DB::add, py::arg("material"))
        .def("update", &MaterialsLibrary::DB::update, py::arg("material"))
        .def("update_or_add", &MaterialsLibrary::DB::updateOrAdd, py::arg("material"))
        .def("delete_with_uuid", &MaterialsLibrary::DB::deleteWithUUID, py::arg("uuid"));
}
