#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "THMZ/Mesh/Mesh.hxx"
#include "THMZ/Mesh/DB.hxx"

namespace py = pybind11;

void bind_mesh(py::module_ & mod)
{
    // --- Segment structs ---
    py::class_<ThermFile::Mesh::Segment::Flux>(mod, "MeshSegmentFlux")
        .def(py::init<>())
        .def_readwrite("index", &ThermFile::Mesh::Segment::Flux::index)
        .def_readwrite("node1", &ThermFile::Mesh::Segment::Flux::node1)
        .def_readwrite("node2", &ThermFile::Mesh::Segment::Flux::node2)
        .def_readwrite("flux1", &ThermFile::Mesh::Segment::Flux::flux1)
        .def_readwrite("flux2", &ThermFile::Mesh::Segment::Flux::flux2)
        .def_readwrite("label", &ThermFile::Mesh::Segment::Flux::label);

    py::class_<ThermFile::Mesh::Segment::Temperature>(mod, "MeshSegmentTemperature")
        .def(py::init<>())
        .def_readwrite("node", &ThermFile::Mesh::Segment::Temperature::node)
        .def_readwrite("temperature", &ThermFile::Mesh::Segment::Temperature::temperature)
        .def_readwrite("label", &ThermFile::Mesh::Segment::Temperature::label);

    py::class_<ThermFile::Mesh::Segment::Convection>(mod, "MeshSegmentConvection")
        .def(py::init<>())
        .def_readwrite("index", &ThermFile::Mesh::Segment::Convection::index)
        .def_readwrite("node1", &ThermFile::Mesh::Segment::Convection::node1)
        .def_readwrite("node2", &ThermFile::Mesh::Segment::Convection::node2)
        .def_readwrite("film_coefficient", &ThermFile::Mesh::Segment::Convection::filmCoefficient)
        .def_readwrite("temperature1", &ThermFile::Mesh::Segment::Convection::temperature1)
        .def_readwrite("temperature2", &ThermFile::Mesh::Segment::Convection::temperature2)
        .def_readwrite("label", &ThermFile::Mesh::Segment::Convection::label);

    py::class_<ThermFile::Mesh::Segment::Radiation>(mod, "MeshSegmentRadiation")
        .def(py::init<>())
        .def_readwrite("index", &ThermFile::Mesh::Segment::Radiation::index)
        .def_readwrite("node1", &ThermFile::Mesh::Segment::Radiation::node1)
        .def_readwrite("node2", &ThermFile::Mesh::Segment::Radiation::node2)
        .def_readwrite("film_coefficient", &ThermFile::Mesh::Segment::Radiation::filmCoefficient)
        .def_readwrite("temperature1", &ThermFile::Mesh::Segment::Radiation::temperature1)
        .def_readwrite("temperature2", &ThermFile::Mesh::Segment::Radiation::temperature2);

    py::class_<ThermFile::Mesh::Segment::RadiationEnclosure>(mod, "MeshSegmentRadiationEnclosure")
        .def(py::init<>())
        .def_readwrite("index", &ThermFile::Mesh::Segment::RadiationEnclosure::index)
        .def_readwrite("node1", &ThermFile::Mesh::Segment::RadiationEnclosure::node1)
        .def_readwrite("node2", &ThermFile::Mesh::Segment::RadiationEnclosure::node2)
        .def_readwrite("cond_code", &ThermFile::Mesh::Segment::RadiationEnclosure::condCode)
        .def_readwrite("emissivity", &ThermFile::Mesh::Segment::RadiationEnclosure::emissivity)
        .def_readwrite("temperature", &ThermFile::Mesh::Segment::RadiationEnclosure::temperature);

    // --- BCSegments ---
    py::class_<ThermFile::Mesh::BCSegments>(mod, "MeshBCSegments")
        .def(py::init<>())
        .def_property("fluxes",
            [](const ThermFile::Mesh::BCSegments & seg) -> py::object {
                if(!seg.fluxes.has_value()) return py::none();
                return py::cast(*seg.fluxes);
            },
            [](ThermFile::Mesh::BCSegments & seg, py::object val) {
                if(val.is_none()) { seg.fluxes = std::nullopt; return; }
                seg.fluxes = val.cast<std::vector<ThermFile::Mesh::Segment::Flux>>();
            })
        .def_property("temperatures",
            [](const ThermFile::Mesh::BCSegments & seg) -> py::object {
                if(!seg.temperatures.has_value()) return py::none();
                return py::cast(*seg.temperatures);
            },
            [](ThermFile::Mesh::BCSegments & seg, py::object val) {
                if(val.is_none()) { seg.temperatures = std::nullopt; return; }
                seg.temperatures = val.cast<std::vector<ThermFile::Mesh::Segment::Temperature>>();
            })
        .def_property("convections",
            [](const ThermFile::Mesh::BCSegments & seg) -> py::object {
                if(!seg.convections.has_value()) return py::none();
                return py::cast(*seg.convections);
            },
            [](ThermFile::Mesh::BCSegments & seg, py::object val) {
                if(val.is_none()) { seg.convections = std::nullopt; return; }
                seg.convections = val.cast<std::vector<ThermFile::Mesh::Segment::Convection>>();
            })
        .def_property("radiations",
            [](const ThermFile::Mesh::BCSegments & seg) -> py::object {
                if(!seg.radiations.has_value()) return py::none();
                return py::cast(*seg.radiations);
            },
            [](ThermFile::Mesh::BCSegments & seg, py::object val) {
                if(val.is_none()) { seg.radiations = std::nullopt; return; }
                seg.radiations = val.cast<std::vector<ThermFile::Mesh::Segment::Radiation>>();
            })
        .def_property("radiation_enclosures",
            [](const ThermFile::Mesh::BCSegments & seg) -> py::object {
                if(!seg.radiationEnclosures.has_value()) return py::none();
                return py::cast(*seg.radiationEnclosures);
            },
            [](ThermFile::Mesh::BCSegments & seg, py::object val) {
                if(val.is_none()) { seg.radiationEnclosures = std::nullopt; return; }
                seg.radiationEnclosures = val.cast<std::vector<ThermFile::Mesh::Segment::RadiationEnclosure>>();
            });

    // --- Node ---
    py::class_<ThermFile::Mesh::Node>(mod, "MeshNode")
        .def(py::init<>())
        .def_readwrite("index", &ThermFile::Mesh::Node::index)
        .def_readwrite("x", &ThermFile::Mesh::Node::x)
        .def_readwrite("y", &ThermFile::Mesh::Node::y);

    // --- Element ---
    py::class_<ThermFile::Mesh::Element>(mod, "MeshElement")
        .def(py::init<>())
        .def_readwrite("index", &ThermFile::Mesh::Element::index)
        .def_readwrite("node1", &ThermFile::Mesh::Element::node1)
        .def_readwrite("node2", &ThermFile::Mesh::Element::node2)
        .def_readwrite("node3", &ThermFile::Mesh::Element::node3)
        .def_readwrite("node4", &ThermFile::Mesh::Element::node4)
        .def_property("material_id",
            [](const ThermFile::Mesh::Element & elm) -> py::object {
                if(!elm.materialID.has_value()) return py::none();
                return py::cast(*elm.materialID);
            },
            [](ThermFile::Mesh::Element & elm, py::object val) {
                if(val.is_none()) { elm.materialID = std::nullopt; return; }
                elm.materialID = val.cast<size_t>();
            });

    // --- Case ---
    py::class_<ThermFile::Mesh::Case>(mod, "MeshCase")
        .def(py::init<>())
        .def_property("model_type",
            [](const ThermFile::Mesh::Case & cas) -> py::object {
                if(!cas.modelType.has_value()) return py::none();
                return py::cast(*cas.modelType);
            },
            [](ThermFile::Mesh::Case & cas, py::object val) {
                if(val.is_none()) { cas.modelType = std::nullopt; return; }
                cas.modelType = val.cast<ThermFile::RunType>();
            })
        .def_property("glazing_case",
            [](const ThermFile::Mesh::Case & cas) -> py::object {
                if(!cas.glazingCase.has_value()) return py::none();
                return py::cast(*cas.glazingCase);
            },
            [](ThermFile::Mesh::Case & cas, py::object val) {
                if(val.is_none()) { cas.glazingCase = std::nullopt; return; }
                cas.glazingCase = val.cast<ThermFile::CMACase>();
            })
        .def_property("spacer_case",
            [](const ThermFile::Mesh::Case & cas) -> py::object {
                if(!cas.spacerCase.has_value()) return py::none();
                return py::cast(*cas.spacerCase);
            },
            [](ThermFile::Mesh::Case & cas, py::object val) {
                if(val.is_none()) { cas.spacerCase = std::nullopt; return; }
                cas.spacerCase = val.cast<ThermFile::CMACase>();
            })
        .def_readwrite("nodes", &ThermFile::Mesh::Case::nodes)
        .def_readwrite("elements", &ThermFile::Mesh::Case::elements)
        .def_readwrite("bc_segments", &ThermFile::Mesh::Case::bcSegments);

    // --- Mesh ---
    py::class_<ThermFile::Mesh::Mesh>(mod, "Mesh")
        .def(py::init<>())
        .def_readwrite("version", &ThermFile::Mesh::Mesh::version)
        .def_readwrite("cases", &ThermFile::Mesh::Mesh::cases);

    // --- Free functions ---
    mod.def("load_mesh_from_file", &ThermFile::Mesh::loadMeshFromFile,
            py::arg("file_name"));

    mod.def("save_mesh_to_file", &ThermFile::Mesh::saveToFile,
            py::arg("mesh"), py::arg("file_name"),
            py::arg("format") = FileParse::FileFormat::XML);

    mod.def("load_mesh_from_zip_file", &ThermFile::Mesh::loadMeshFromZipFile,
            py::arg("zip_file_name"));

    mod.def("save_mesh_to_zip_file", &ThermFile::Mesh::saveToZipFile,
            py::arg("mesh"), py::arg("zip_file_name"));

    mod.def("load_mesh_from_string", &ThermFile::Mesh::loadMeshFromString,
            py::arg("str"), py::arg("format") = FileParse::FileFormat::XML);

    mod.def("save_mesh_to_string", &ThermFile::Mesh::saveToString,
            py::arg("mesh"), py::arg("format") = FileParse::FileFormat::XML);
}
