#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "THMZ/SteadyStateMeshResults/Results.hxx"
#include "THMZ/SteadyStateMeshResults/DB.hxx"

namespace py = pybind11;

void bind_mesh_results(py::module_ & mod)
{
    // --- NodeResults ---
    py::class_<ThermFile::NodeResults>(mod, "NodeResults")
        .def(py::init<>())
        .def_readwrite("index", &ThermFile::NodeResults::index)
        .def_readwrite("temperature", &ThermFile::NodeResults::temperature)
        .def_readwrite("x_flux", &ThermFile::NodeResults::xFlux)
        .def_readwrite("y_flux", &ThermFile::NodeResults::yFlux);

    // --- EdgeResults ---
    py::class_<ThermFile::EdgeResults>(mod, "EdgeResults")
        .def(py::init<>())
        .def_readwrite("segment_type", &ThermFile::EdgeResults::segmentType)
        .def_readwrite("index", &ThermFile::EdgeResults::index)
        .def_readwrite("area", &ThermFile::EdgeResults::area)
        .def_readwrite("flux", &ThermFile::EdgeResults::flux)
        .def_readwrite("heat_rate", &ThermFile::EdgeResults::heatRate);

    // --- TagNodes ---
    py::class_<ThermFile::TagNodes>(mod, "TagNodes")
        .def(py::init<>())
        .def_readwrite("name", &ThermFile::TagNodes::name)
        .def_readwrite("nodes", &ThermFile::TagNodes::nodes);

    // --- TagNodesCase ---
    py::class_<ThermFile::TagNodesCase>(mod, "TagNodesCase")
        .def(py::init<>())
        .def_property("results_type",
            [](const ThermFile::TagNodesCase & cas) -> py::object {
                if(!cas.resultsType.has_value()) return py::none();
                return py::cast(*cas.resultsType);
            },
            [](ThermFile::TagNodesCase & cas, py::object val) {
                if(val.is_none()) { cas.resultsType = std::nullopt; return; }
                cas.resultsType = val.cast<ThermFile::RunType>();
            })
        .def_property("glazing_case",
            [](const ThermFile::TagNodesCase & cas) -> py::object {
                if(!cas.glazingCase.has_value()) return py::none();
                return py::cast(*cas.glazingCase);
            },
            [](ThermFile::TagNodesCase & cas, py::object val) {
                if(val.is_none()) { cas.glazingCase = std::nullopt; return; }
                cas.glazingCase = val.cast<ThermFile::CMACase>();
            })
        .def_property("spacer_case",
            [](const ThermFile::TagNodesCase & cas) -> py::object {
                if(!cas.spacerCase.has_value()) return py::none();
                return py::cast(*cas.spacerCase);
            },
            [](ThermFile::TagNodesCase & cas, py::object val) {
                if(val.is_none()) { cas.spacerCase = std::nullopt; return; }
                cas.spacerCase = val.cast<ThermFile::CMACase>();
            })
        .def_readwrite("tag_nodes", &ThermFile::TagNodesCase::tagNodes);

    // --- CaseMeshResults ---
    py::class_<ThermFile::CaseMeshResults>(mod, "CaseMeshResults")
        .def(py::init<>())
        .def_property("results_type",
            [](const ThermFile::CaseMeshResults & cas) -> py::object {
                if(!cas.resultsType.has_value()) return py::none();
                return py::cast(*cas.resultsType);
            },
            [](ThermFile::CaseMeshResults & cas, py::object val) {
                if(val.is_none()) { cas.resultsType = std::nullopt; return; }
                cas.resultsType = val.cast<ThermFile::RunType>();
            })
        .def_property("glazing_case",
            [](const ThermFile::CaseMeshResults & cas) -> py::object {
                if(!cas.glazingCase.has_value()) return py::none();
                return py::cast(*cas.glazingCase);
            },
            [](ThermFile::CaseMeshResults & cas, py::object val) {
                if(val.is_none()) { cas.glazingCase = std::nullopt; return; }
                cas.glazingCase = val.cast<ThermFile::CMACase>();
            })
        .def_property("spacer_case",
            [](const ThermFile::CaseMeshResults & cas) -> py::object {
                if(!cas.spacerCase.has_value()) return py::none();
                return py::cast(*cas.spacerCase);
            },
            [](ThermFile::CaseMeshResults & cas, py::object val) {
                if(val.is_none()) { cas.spacerCase = std::nullopt; return; }
                cas.spacerCase = val.cast<ThermFile::CMACase>();
            })
        .def_readwrite("nodes", &ThermFile::CaseMeshResults::nodes)
        .def_readwrite("element_edges", &ThermFile::CaseMeshResults::elementEdges)
        .def_readwrite("polygon_edges", &ThermFile::CaseMeshResults::polygonEdges)
        .def_readwrite("temperature_contour_node_ids", &ThermFile::CaseMeshResults::temperatureCountourNodeIDs);

    // --- MeshResults ---
    py::class_<ThermFile::MeshResults>(mod, "MeshResults")
        .def(py::init<>())
        .def_readwrite("version", &ThermFile::MeshResults::version)
        .def_readwrite("cases", &ThermFile::MeshResults::cases)
        .def_readwrite("tag_nodes", &ThermFile::MeshResults::tagNodes);

    // --- Free functions (overloaded saveToFile/saveToZipFile/saveToString) ---
    mod.def("load_steady_state_mesh_results_from_file",
            &ThermFile::loadSteadyStateMeshResultsFromFile,
            py::arg("file_name"));

    mod.def("save_mesh_results_to_file",
            static_cast<int(*)(const ThermFile::MeshResults &, std::string_view)>(
                &ThermFile::saveToFile),
            py::arg("results"), py::arg("file_name"));

    mod.def("load_steady_state_mesh_results_from_zip_file",
            &ThermFile::loadSteadyStateMeshResultsFromZipFile,
            py::arg("zip_file_name"));

    mod.def("save_mesh_results_to_zip_file",
            static_cast<int(*)(const ThermFile::MeshResults &, std::string_view)>(
                &ThermFile::saveToZipFile),
            py::arg("results"), py::arg("zip_file_name"));

    mod.def("load_steady_state_mesh_results_from_string",
            &ThermFile::loadSteadyStateMeshResultsFromString,
            py::arg("xml_string"));

    mod.def("save_mesh_results_to_string",
            static_cast<std::string(*)(const ThermFile::MeshResults &)>(
                &ThermFile::saveToString),
            py::arg("results"));
}
