#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Gases/DB.hxx"

namespace py = pybind11;

void bind_gases(py::module_ & mod)
{
    py::class_<GasesLibrary::Coefficients>(mod, "GasCoefficients")
        .def(py::init<>())
        .def_readwrite("a", &GasesLibrary::Coefficients::A)
        .def_readwrite("b", &GasesLibrary::Coefficients::B)
        .def_readwrite("c", &GasesLibrary::Coefficients::C);

    py::class_<GasesLibrary::GasProperties>(mod, "GasProperties")
        .def(py::init<>())
        .def_readwrite("molecular_weight", &GasesLibrary::GasProperties::MolecularWeight)
        .def_readwrite("specific_heat_ratio", &GasesLibrary::GasProperties::SpecificHeatRatio)
        .def_readwrite("conductivity", &GasesLibrary::GasProperties::Conductivity)
        .def_readwrite("viscosity", &GasesLibrary::GasProperties::Viscosity)
        .def_readwrite("specific_heat", &GasesLibrary::GasProperties::SpecificHeat);

    py::class_<GasesLibrary::GasCommon>(mod, "GasCommon")
        .def(py::init<>())
        .def_readwrite("uuid", &GasesLibrary::GasCommon::UUID)
        .def_readwrite("name", &GasesLibrary::GasCommon::Name)
        .def_readwrite("protected_", &GasesLibrary::GasCommon::Protected);

    py::class_<GasesLibrary::PureGas, GasesLibrary::GasCommon>(mod, "PureGas")
        .def(py::init<>())
        .def_readwrite("properties", &GasesLibrary::PureGas::Properties)
        .def_readwrite("project_name", &GasesLibrary::PureGas::ProjectName);

    py::class_<GasesLibrary::Component>(mod, "GasComponent")
        .def(py::init<>())
        .def_readwrite("fraction", &GasesLibrary::Component::Fraction)
        .def_readwrite("pure_gas_name", &GasesLibrary::Component::PureGasName);

    py::class_<GasesLibrary::Gas, GasesLibrary::GasCommon>(mod, "Gas")
        .def(py::init<>())
        .def_readwrite("components", &GasesLibrary::Gas::Components)
        .def_readwrite("project_name", &GasesLibrary::Gas::ProjectName);

    py::class_<GasesLibrary::GasesData>(mod, "GasesData")
        .def(py::init<>())
        .def_readwrite("gas", &GasesLibrary::GasesData::gas)
        .def_readwrite("components", &GasesLibrary::GasesData::components)
        .def_readwrite("name", &GasesLibrary::GasesData::Name)
        .def_readwrite("project_name", &GasesLibrary::GasesData::ProjectName)
        .def_readwrite("protected_", &GasesLibrary::GasesData::Protected);

    // --- DB ---
    py::class_<GasesLibrary::DB>(mod, "GasesDB")
        .def(py::init<>())
        .def(py::init<const std::string &>(), py::arg("file_name"))
        .def("load_from_string", &GasesLibrary::DB::loadFromString, py::arg("str"))
        .def("save_to_string", &GasesLibrary::DB::saveToString,
             py::arg("format") = FileParse::FileFormat::XML)
        .def("get_gases", &GasesLibrary::DB::getGases,
             py::return_value_policy::reference_internal)
        .def("get_pure_gases", &GasesLibrary::DB::getPureGases,
             py::return_value_policy::reference_internal)
        .def("get_gases_data", &GasesLibrary::DB::getGasesData)
        .def("get_by_uuid", &GasesLibrary::DB::getByUUID, py::arg("uuid"))
        .def("get_by_name", &GasesLibrary::DB::getByName, py::arg("name"))
        .def("get_names", &GasesLibrary::DB::getNames)
        .def("get_display_names", &GasesLibrary::DB::getDisplayNames)
        .def("get_pure_gas_names", &GasesLibrary::DB::getPureGasNames)
        .def("get_pure_gas_by_uuid", &GasesLibrary::DB::getPureGasByUUID, py::arg("uuid"))
        .def("get_pure_gas_by_name", &GasesLibrary::DB::getPureGasByName, py::arg("name"))
        .def("add_pure_gas", &GasesLibrary::DB::addPureGas, py::arg("pure"))
        .def("add_gas", &GasesLibrary::DB::addGas, py::arg("gas"))
        .def("add", &GasesLibrary::DB::add, py::arg("gas_data"))
        .def("delete_with_uuid", &GasesLibrary::DB::deleteWithUUID, py::arg("uuid"));
}
