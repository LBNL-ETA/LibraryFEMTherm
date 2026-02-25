#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "BCSteadyState/DB.hxx"

namespace py = pybind11;

void bind_bc(py::module_ & mod)
{
    py::class_<BCSteadyStateLibrary::Convection>(mod, "Convection")
        .def(py::init<>())
        .def_readwrite("temperature", &BCSteadyStateLibrary::Convection::temperature)
        .def_readwrite("film_coefficient", &BCSteadyStateLibrary::Convection::filmCoefficient);

    py::class_<BCSteadyStateLibrary::ConstantFlux>(mod, "ConstantFlux")
        .def(py::init<>())
        .def_readwrite("flux", &BCSteadyStateLibrary::ConstantFlux::flux);

    py::class_<BCSteadyStateLibrary::AutomaticEnclosure>(mod, "AutomaticEnclosure")
        .def(py::init<>())
        .def_readwrite("temperature", &BCSteadyStateLibrary::AutomaticEnclosure::temperature)
        .def_readwrite("emissivity", &BCSteadyStateLibrary::AutomaticEnclosure::emissivity);

    py::class_<BCSteadyStateLibrary::ManualEnclosure>(mod, "ManualEnclosure")
        .def(py::init<>());

    py::class_<BCSteadyStateLibrary::BlackBodyRadiation>(mod, "BlackBodyRadiation")
        .def(py::init<>())
        .def_readwrite("temperature", &BCSteadyStateLibrary::BlackBodyRadiation::temperature)
        .def_readwrite("emissivity", &BCSteadyStateLibrary::BlackBodyRadiation::emissivity)
        .def_readwrite("view_factor", &BCSteadyStateLibrary::BlackBodyRadiation::viewFactor);

    py::class_<BCSteadyStateLibrary::LinearizedRadiation>(mod, "LinearizedRadiation")
        .def(py::init<>())
        .def_readwrite("temperature", &BCSteadyStateLibrary::LinearizedRadiation::temperature)
        .def_readwrite("film_coefficient", &BCSteadyStateLibrary::LinearizedRadiation::filmCoefficient);

    py::class_<BCSteadyStateLibrary::Radiation>(mod, "Radiation")
        .def(py::init<>())
        .def_property("radiation",
            [](const BCSteadyStateLibrary::Radiation & rad) -> py::object {
                if(!rad.radiation.has_value())
                    return py::none();
                return std::visit([](const auto & val) -> py::object {
                    return py::cast(val);
                }, *rad.radiation);
            },
            [](BCSteadyStateLibrary::Radiation & rad, py::object val) {
                if(val.is_none()) { rad.radiation = std::nullopt; return; }
                try { rad.radiation = val.cast<BCSteadyStateLibrary::AutomaticEnclosure>(); return; } catch(py::cast_error &) {}
                try { rad.radiation = val.cast<BCSteadyStateLibrary::ManualEnclosure>(); return; } catch(py::cast_error &) {}
                try { rad.radiation = val.cast<BCSteadyStateLibrary::BlackBodyRadiation>(); return; } catch(py::cast_error &) {}
                try { rad.radiation = val.cast<BCSteadyStateLibrary::LinearizedRadiation>(); return; } catch(py::cast_error &) {}
                throw py::type_error("Expected AutomaticEnclosure, ManualEnclosure, BlackBodyRadiation, or LinearizedRadiation");
            });

    py::class_<BCSteadyStateLibrary::Comprehensive>(mod, "Comprehensive")
        .def(py::init<>())
        .def_readwrite("relative_humidity", &BCSteadyStateLibrary::Comprehensive::relativeHumidity)
        .def_readwrite("convection", &BCSteadyStateLibrary::Comprehensive::convection)
        .def_readwrite("constant_flux", &BCSteadyStateLibrary::Comprehensive::constantFlux)
        .def_readwrite("radiation", &BCSteadyStateLibrary::Comprehensive::radiation);

    py::class_<BCSteadyStateLibrary::Simplified>(mod, "Simplified")
        .def(py::init<>())
        .def_readwrite("temperature", &BCSteadyStateLibrary::Simplified::temperature)
        .def_readwrite("film_coefficient", &BCSteadyStateLibrary::Simplified::filmCoefficient)
        .def_readwrite("relative_humidity", &BCSteadyStateLibrary::Simplified::relativeHumidity);

    py::class_<BCSteadyStateLibrary::RadiationSurface>(mod, "RadiationSurface")
        .def(py::init<>())
        .def_readwrite("is_default", &BCSteadyStateLibrary::RadiationSurface::isDefault)
        .def_readwrite("temperature", &BCSteadyStateLibrary::RadiationSurface::temperature)
        .def_readwrite("emissivity", &BCSteadyStateLibrary::RadiationSurface::emissivity);

    py::class_<BCSteadyStateLibrary::BoundaryCondition>(mod, "BCSteadyState")
        .def(py::init<>())
        .def_readwrite("uuid", &BCSteadyStateLibrary::BoundaryCondition::UUID)
        .def_readwrite("name", &BCSteadyStateLibrary::BoundaryCondition::Name)
        .def_readwrite("protected_", &BCSteadyStateLibrary::BoundaryCondition::Protected)
        .def_readwrite("color", &BCSteadyStateLibrary::BoundaryCondition::Color)
        .def_property("data",
            [](const BCSteadyStateLibrary::BoundaryCondition & bcd) -> py::object {
                return std::visit([](const auto & val) -> py::object {
                    return py::cast(val);
                }, bcd.data);
            },
            [](BCSteadyStateLibrary::BoundaryCondition & bcd, py::object val) {
                try { bcd.data = val.cast<BCSteadyStateLibrary::Comprehensive>(); return; } catch(py::cast_error &) {}
                try { bcd.data = val.cast<BCSteadyStateLibrary::Simplified>(); return; } catch(py::cast_error &) {}
                try { bcd.data = val.cast<BCSteadyStateLibrary::RadiationSurface>(); return; } catch(py::cast_error &) {}
                throw py::type_error("Expected Comprehensive, Simplified, or RadiationSurface");
            })
        .def_readwrite("project_name", &BCSteadyStateLibrary::BoundaryCondition::ProjectName)
        .def_readwrite("is_igu_surface", &BCSteadyStateLibrary::BoundaryCondition::isIGUSurface);

    // --- DB ---
    py::class_<BCSteadyStateLibrary::DB>(mod, "BCSteadyStateDB")
        .def(py::init<>())
        .def(py::init<const std::string &>(), py::arg("xml_file_name"))
        .def("load_from_string", &BCSteadyStateLibrary::DB::loadFromString, py::arg("str"))
        .def("save_to_string", &BCSteadyStateLibrary::DB::saveToString,
             py::arg("format") = FileParse::FileFormat::XML)
        .def("get_boundary_conditions", &BCSteadyStateLibrary::DB::getBoundaryConditions,
             py::return_value_policy::reference_internal)
        .def("get_by_uuid", &BCSteadyStateLibrary::DB::getByUUID, py::arg("uuid"))
        .def("get_by_name", &BCSteadyStateLibrary::DB::getByName, py::arg("name"))
        .def("get_names", &BCSteadyStateLibrary::DB::getNames)
        .def("get_display_names", &BCSteadyStateLibrary::DB::getDisplayNames)
        .def("add", &BCSteadyStateLibrary::DB::add, py::arg("condition"))
        .def("update", &BCSteadyStateLibrary::DB::update, py::arg("condition"))
        .def("update_or_add", &BCSteadyStateLibrary::DB::updateOrAdd, py::arg("condition"))
        .def("delete_with_uuid", &BCSteadyStateLibrary::DB::deleteWithUUID, py::arg("uuid"))
        .def("get_default_record", &BCSteadyStateLibrary::DB::getDefaultRecord)
        .def("get_default_radiation_surface", &BCSteadyStateLibrary::DB::getDefaultRadiationSurface);
}
