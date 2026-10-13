#include <map>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "SteadyStateField/SteadyStateField.hxx"

namespace py = pybind11;

void bind_steady_state_field(py::module_ & mod)
{
    using namespace ThermFile::SteadyStateField;

    auto field{
      mod.def_submodule("steady_state_field", "One steady-state run's nodal results on the mesh they were solved on.")};

    py::class_<Field>(field,
                      "Field",
                      "Per-node vectors are in the order of `nodes`, so an element corner indexes them directly. "
                      "NaN where the run has no value for the node.")
      .def_readonly("run_type", &Field::runType)
      .def_readonly("nodes", &Field::nodes)
      .def_readonly("elements", &Field::elements)
      .def_readonly("temperature", &Field::temperature)
      .def_readonly("x_flux", &Field::xFlux)
      .def_readonly("y_flux", &Field::yFlux)
      .def_readonly("humidity", &Field::humidity)
      .def_readonly("water_content", &Field::waterContent)
      .def_readonly("ice_content", &Field::iceContent)
      .def_property_readonly("flux_magnitude", &fluxMagnitude, "Heat-flux magnitude at every node.");

    field.def(
      "read_entries",
      [](const std::map<std::string, std::string> & entries, const ThermFile::RunType runType) {
          const auto result{read(entries, runType)};
          if(!result.has_value())
          {
              throw py::value_error(result.error());
          }
          return result.value();
      },
      py::arg("entries"),
      py::arg("run_type") = ThermFile::RunType::UFactor,
      "Reads the run's field out of extracted entries. Raises ValueError when the archive has no stored mesh, no "
      "nodal results, or no case of that run type.");
    field.def(
      "read",
      [](const std::string & zipFileName, const ThermFile::RunType runType) {
          const auto result{readFromZipFile(zipFileName, runType)};
          if(!result.has_value())
          {
              throw py::value_error(result.error());
          }
          return result.value();
      },
      py::arg("thmz_file_name"),
      py::arg("run_type") = ThermFile::RunType::UFactor,
      "Extracts the archive's mesh and nodal results and reads the run's field. Raises ValueError as read_entries() does.");
}
