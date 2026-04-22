#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "THMZ/Model/DB.hxx"

namespace py = pybind11;

void bind_model(py::module_ & mod)
{
    py::class_<ThermFile::RadianceModeTags>(mod, "RadianceModeTags")
        .def(py::init<>())
        .def_readwrite("shade_in_name", &ThermFile::RadianceModeTags::shadeInName)
        .def_readwrite("shade_in_tag", &ThermFile::RadianceModeTags::shadeInTag)
        .def_readwrite("shade_out_name", &ThermFile::RadianceModeTags::shadeOutName)
        .def_readwrite("shade_out_tag", &ThermFile::RadianceModeTags::shadeOutTag);

    py::class_<ThermFile::ThermModel>(mod, "ThermModel")
        .def(py::init<>())
        .def_readwrite("version", &ThermFile::ThermModel::version)
        .def_readwrite("calculation_ready", &ThermFile::ThermModel::calculationReady)
        .def_readwrite("preferences", &ThermFile::ThermModel::preferences)
        .def_readwrite("properties", &ThermFile::ThermModel::properties)
        .def_readwrite("glazing_origin", &ThermFile::ThermModel::glazingOrigin)
        .def_readwrite("radiance_tags", &ThermFile::ThermModel::radianceTags)
        .def_readwrite("cavities", &ThermFile::ThermModel::cavities)
        .def_readwrite("polygons", &ThermFile::ThermModel::polygons)
        .def_readwrite("boundary_conditions", &ThermFile::ThermModel::boundaryConditions)
        .def_readwrite("glazing_systems", &ThermFile::ThermModel::glazingSystems)
        .def_readwrite("cma_data", &ThermFile::ThermModel::cmaData)
        .def_readwrite("gasket_materials", &ThermFile::ThermModel::gasketMaterials);

    // Free functions for load/save
    mod.def("load_model_from_file", &ThermFile::loadThermModelFromFile,
            py::arg("file_name"));

    mod.def("save_model_to_file", &ThermFile::saveToFile,
            py::arg("model"), py::arg("file_name"),
            py::arg("format") = FileParse::FileFormat::XML);

    mod.def("load_model_from_zip_file", &ThermFile::loadThermModelFromZipFile,
            py::arg("zip_file_name"));

    mod.def("save_model_to_zip_file", &ThermFile::saveToZipFile,
            py::arg("model"), py::arg("zip_file_name"));

    mod.def("load_model_from_string", &ThermFile::loadThermModelFromString,
            py::arg("str"), py::arg("format") = FileParse::FileFormat::XML);

    mod.def("save_model_to_string", &ThermFile::saveToString,
            py::arg("model"), py::arg("format") = FileParse::FileFormat::XML);
}
