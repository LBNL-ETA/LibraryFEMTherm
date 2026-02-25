#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Materials/DB.hxx"

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

    py::class_<MaterialsLibrary::Cavity>(mod, "MaterialCavity")
        .def(py::init<>())
        .def_readwrite("emissivity_side1", &MaterialsLibrary::Cavity::EmissivitySide1)
        .def_readwrite("emissivity_side2", &MaterialsLibrary::Cavity::EmissivitySide2)
        .def_readwrite("cavity_standard", &MaterialsLibrary::Cavity::cavityStandard)
        .def_readwrite("gas_name", &MaterialsLibrary::Cavity::GasName);

    py::class_<MaterialsLibrary::RadiationEnclosure>(mod, "RadiationEnclosure")
        .def(py::init<>())
        .def_readwrite("emissivity_default", &MaterialsLibrary::RadiationEnclosure::emissivityDefault);

    // --- Material ---
    py::class_<MaterialsLibrary::Material>(mod, "Material")
        .def(py::init<>())
        .def_readwrite("uuid", &MaterialsLibrary::Material::UUID)
        .def_readwrite("name", &MaterialsLibrary::Material::Name)
        .def_readwrite("project_name", &MaterialsLibrary::Material::ProjectName)
        .def_readwrite("protected_", &MaterialsLibrary::Material::Protected)
        .def_readwrite("color", &MaterialsLibrary::Material::Color)
        .def_property("data",
            [](const MaterialsLibrary::Material & mat) -> py::object {
                return std::visit([](const auto & val) -> py::object {
                    return py::cast(val);
                }, mat.data);
            },
            [](MaterialsLibrary::Material & mat, py::object val) {
                try { mat.data = val.cast<MaterialsLibrary::Solid>(); return; } catch(py::cast_error &) {}
                try { mat.data = val.cast<MaterialsLibrary::Cavity>(); return; } catch(py::cast_error &) {}
                try { mat.data = val.cast<MaterialsLibrary::RadiationEnclosure>(); return; } catch(py::cast_error &) {}
                throw py::type_error("Expected Solid, MaterialCavity, or RadiationEnclosure");
            });

    // Helper functions
    mod.def("is_solid", &MaterialsLibrary::isSolid, py::arg("material"));
    mod.def("is_cavity", &MaterialsLibrary::isCavity, py::arg("material"));
    mod.def("is_radiation_enclosure", &MaterialsLibrary::isRadiationEnclosure, py::arg("material"));

    // --- DB ---
    py::class_<MaterialsLibrary::DB>(mod, "MaterialsDB")
        .def(py::init<>())
        .def(py::init<const std::string &>(), py::arg("xml_file_name"))
        .def("load_from_string", &MaterialsLibrary::DB::loadFromString, py::arg("str"))
        .def("save_to_string", &MaterialsLibrary::DB::saveToString,
             py::arg("format") = FileParse::FileFormat::XML)
        .def("get_materials", &MaterialsLibrary::DB::getMaterials,
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
