#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <filesystem>
#include <format>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "THMZ/Archive/Archive.hxx"

#include "PathOnly.hxx"

namespace py = pybind11;

using ThermArchive::Archive;

namespace
{
    //! The part, or None when the archive has none. ValueError when the part is there and
    //! cannot be read, or the file is no longer where it was.
    template<typename Type>
    std::optional<Type> partOrRaise(const ThermArchive::Part<Type> & part)
    {
        if(!part.has_value())
        {
            throw py::value_error(part.error());
        }
        return part.value();
    }

    //! FileNotFoundError for a file that is not there, ValueError for one that is not an
    //! archive.
    Archive readOrRaise(const PathOnly & path)
    {
        const auto archive{ThermArchive::read(PathBinding::utf8Of(path.value))};
        if(archive.has_value())
        {
            return archive.value();
        }
        if(!std::filesystem::exists(path.value))
        {
            PyErr_SetString(PyExc_FileNotFoundError, archive.error().c_str());
            throw py::error_already_set();
        }
        throw py::value_error(archive.error());
    }
}   // namespace

//! Bound last: an archive's parts are the model, the mesh, the libraries, the datasets and
//! the results, so every one of those types must already be registered.
void bind_archive(py::module_ & mod)
{
    auto thmz{mod.attr("thmz").cast<py::module_>()};

    py::class_<Archive>(thmz,
                        "Archive",
                        "A THERM archive (.thmz) on disk: where it is and the names of its entries. It holds no "
                        "content. Each part is read from the file when its method is called, and again on every "
                        "call, so keep what a method returns. A part the archive does not hold is None (no "
                        "datasets is an empty list). A part that is there and cannot be read, or a file that is no "
                        "longer where it was, raises ValueError.")
      .def_property_readonly(
        "file_path", [](const Archive & archive) { return PathOnly{.value = PathBinding::pathOf(archive.fileName)}; })
      .def_readonly("entry_names", &Archive::entryNames, "The names of the archive's entries, in archive order.")
      .def("has_entry",
           &ThermArchive::hasEntry,
           py::arg("base_name"),
           "Whether the archive holds the entry under either format's spelling (\"Model\" for Model.xml or "
           "Model.json).")
      .def(
        "entry",
        [](const Archive & archive, const std::string & name) { return partOrRaise(ThermArchive::entry(archive, name)); },
        py::arg("name"),
        "The text of one entry by its full name (\"Materials.xml\"); None when the archive has no such entry.")
      .def(
        "entries",
        [](const Archive & archive, const std::vector<std::string> & names) {
            const auto entries{ThermArchive::entries(archive, names)};
            if(!entries.has_value())
            {
                throw py::value_error(entries.error());
            }
            return entries.value();
        },
        py::arg("names") = std::vector<std::string>{},
        "The text of the named entries as a dict, name -> content; every entry when no name is given. A name "
        "the archive does not hold is left out.")
      .def(
        "model",
        [](const Archive & archive) { return partOrRaise(ThermArchive::model(archive)); },
        "The model, brought up to the current version.")
      .def("mesh", [](const Archive & archive) { return partOrRaise(ThermArchive::mesh(archive)); })
      .def("materials", [](const Archive & archive) { return partOrRaise(ThermArchive::materials(archive)); })
      .def("gases", [](const Archive & archive) { return partOrRaise(ThermArchive::gases(archive)); })
      .def(
        "boundary_conditions",
        [](const Archive & archive) { return partOrRaise(ThermArchive::boundaryConditions(archive)); },
        "The unified boundary-condition library; None for an archive written before the libraries were unified.")
      .def(
        "datasets",
        [](const Archive & archive) {
            const auto datasets{ThermArchive::datasets(archive)};
            if(!datasets.has_value())
            {
                throw py::value_error(datasets.error());
            }
            return datasets.value();
        },
        "Every time series dataset the archive stores; an empty list when it stores none.")
      .def("steady_state_results",
           [](const Archive & archive) { return partOrRaise(ThermArchive::steadyStateResults(archive)); })
      .def("steady_state_mesh_results",
           [](const Archive & archive) { return partOrRaise(ThermArchive::steadyStateMeshResults(archive)); })
      .def(
        "steady_state_field",
        [](const Archive & archive, const ThermFile::RunType runType) {
            return partOrRaise(ThermArchive::steadyStateField(archive, runType));
        },
        py::arg("run_type") = ThermFile::RunType::UFactor,
        "The stored mesh joined with the nodal results of one run; None when the archive lacks the mesh or the "
        "nodal results. Raises ValueError when both are there and neither has a case of that run type.")
      .def("transient_results",
           [](const Archive & archive) { return partOrRaise(ThermArchive::transientResults(archive)); })
      .def("__repr__", [](const Archive & archive) {
          return std::format("Archive(file_path={}, entries={})",
                             py::repr(py::cast(PathOnly{.value = PathBinding::pathOf(archive.fileName)})).cast<std::string>(),
                             archive.entryNames.size());
      });

    thmz.def("read",
             &readOrRaise,
             py::arg("thmz_file_path"),
             "Opens a THERM archive and reads the names of its entries, nothing more. Raises FileNotFoundError "
             "when the file does not exist and ValueError when it is not a readable archive. The parts are then "
             "asked of the result: archive.model(), archive.transient_results(), ...");
}
