#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <format>

#include "TransientResults/TransientResults.hxx"

namespace py = pybind11;

using namespace ThermFile::TransientResults;

//! The node results of a transient solve, read back out of the archive: the solver mesh
//! and one series per stored quantity, values[frame][column], with column k belonging to
//! the node with ID k + 1. Nothing is rescaled; humidity is the stored fraction.
void bind_transient_results(py::module_ & mod)
{
    auto transient{mod.def_submodule("transient_results",
                                     "Read the node results of a transient solve out of a THMZ.")};

    py::enum_<Quantity>(transient, "Quantity")
      .value("Temperature", Quantity::Temperature)
      .value("Humidity", Quantity::Humidity)
      .value("WaterContent", Quantity::WaterContent)
      .value("IceContent", Quantity::IceContent);

    transient.def("entry_name", &entryName, py::arg("quantity"),
                  "The archive entry a nodal quantity is stored under.");

    py::class_<Node>(transient, "Node", "One node of the solver mesh, in column order.")
      .def_readonly("id", &Node::id)
      .def_readonly("x", &Node::x)
      .def_readonly("y", &Node::y)
      .def("__repr__", [](const Node & node) { return std::format("Node({}, {}, {})", node.id, node.x, node.y); });

    py::class_<FluxVector>(transient, "FluxVector")
      .def_readonly("x", &FluxVector::x)
      .def_readonly("y", &FluxVector::y)
      .def("__repr__", [](const FluxVector & flux) { return std::format("FluxVector({}, {})", flux.x, flux.y); });

    // The solver mesh as the results folder stores it. Bound here, minimally, because the
    // results carry it; nothing else in the module hands one out.
    py::class_<GeometryLibrary::Material>(transient, "GeometryMaterial")
      .def_readonly("id", &GeometryLibrary::Material::ID)
      .def_readonly("name", &GeometryLibrary::Material::Name)
      .def_readonly("color", &GeometryLibrary::Material::Color);
    py::class_<GeometryLibrary::Node>(transient, "GeometryNode")
      .def_readonly("id", &GeometryLibrary::Node::ID)
      .def_readonly("x", &GeometryLibrary::Node::x)
      .def_readonly("y", &GeometryLibrary::Node::y);
    py::class_<GeometryLibrary::Element>(transient, "GeometryElement",
                                         "Node references are the geometry's node IDs, 1-based.")
      .def_readonly("id", &GeometryLibrary::Element::id)
      .def_readonly("node1", &GeometryLibrary::Element::nodeID1)
      .def_readonly("node2", &GeometryLibrary::Element::nodeID2)
      .def_readonly("node3", &GeometryLibrary::Element::nodeID3)
      .def_readonly("node4", &GeometryLibrary::Element::nodeID4)
      .def_readonly("material_id", &GeometryLibrary::Element::materialID);
    py::class_<GeometryLibrary::MaterialPolygon>(transient, "MaterialPolygon")
      .def_readonly("id", &GeometryLibrary::MaterialPolygon::id)
      .def_readonly("material_id", &GeometryLibrary::MaterialPolygon::materialID)
      .def_readonly("nodes", &GeometryLibrary::MaterialPolygon::m_Nodes);
    py::class_<GeometryLibrary::InputGeometryDataRecord>(transient, "ResultsGeometry",
                                                          "The solver mesh: nodes, elements, materials, polygons.")
      .def_readonly("version", &GeometryLibrary::InputGeometryDataRecord::version)
      .def_readonly("materials", &GeometryLibrary::InputGeometryDataRecord::materials)
      .def_readonly("nodes", &GeometryLibrary::InputGeometryDataRecord::nodes)
      .def_readonly("elements", &GeometryLibrary::InputGeometryDataRecord::elements)
      .def_readonly("material_polygons", &GeometryLibrary::InputGeometryDataRecord::materialPolygons)
      .def_readonly("model_enclosure", &GeometryLibrary::InputGeometryDataRecord::modelEnclosure);

    py::class_<NodalSeries>(transient, "NodalSeries",
                            "One nodal quantity over the run: values[frame][column], one solve error per frame.")
      .def_readonly("values", &NodalSeries::values)
      .def_readonly("solve_error", &NodalSeries::solveError);

    py::class_<Results>(transient, "Results", "Every stored quantity of the archive's transient run.")
      .def_readonly("geometry", &Results::geometry)
      .def_readonly("nodes", &Results::nodes)
      .def_readonly("frames", &Results::frames)
      .def_readonly("first_frame_is_initial", &Results::firstFrameIsInitial)
      .def_readonly("nodal", &Results::nodal)
      .def_readonly("heat_flux", &Results::heatFlux)
      .def_readonly("water_flux", &Results::waterFlux)
      .def_property_readonly("node_count", &Results::nodeCount)
      .def_property_readonly("frame_count", &Results::frameCount)
      .def("has", &Results::has, py::arg("quantity"))
      .def(
        "__getitem__",
        [](const Results & results, const Quantity quantity) -> const NodalSeries & {
            const auto found{results.nodal.find(quantity)};
            if(found == results.nodal.end())
            {
                throw py::key_error(std::string(py::str(py::cast(quantity))));
            }
            return found->second;
        },
        py::arg("quantity"),
        py::return_value_policy::reference_internal);

    transient.def("has_transient_results", &hasTransientResults, py::arg("entries"),
                  "Whether extracted entries carry a transient results folder with a nodal CSV.");
    transient.def(
      "read",
      [](const std::map<std::string, std::string> & entries) {
          const auto results{read(entries)};
          if(!results.has_value())
          {
              throw py::value_error(results.error());
          }
          return results.value();
      },
      py::arg("entries"),
      "Reads the transient results out of extracted entries. Raises ValueError when the archive carries none "
      "or they are inconsistent.");
    transient.def(
      "read_from_zip_file",
      [](const std::string & zipFileName) {
          const auto results{readFromZipFile(zipFileName)};
          if(!results.has_value())
          {
              throw py::value_error(results.error());
          }
          return results.value();
      },
      py::arg("zip_file_name"),
      "Extracts the archive and reads its transient results. Raises ValueError when it carries none.");
}
