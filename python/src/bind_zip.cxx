#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "THMZ/ZipModule/ZipModule.hxx"

namespace py = pybind11;

void bind_zip(py::module_ & mod)
{
    auto zip = mod.def_submodule("zip", "THMZ ZIP archive operations");

    // File name constants
    zip.attr("MODEL_FILE_NAME") = ThermZip::ModelFileName;
    zip.attr("GASES_FILE_NAME") = ThermZip::GasesFileName;
    zip.attr("MATERIALS_FILE_NAME") = ThermZip::MaterialsFileName;
    zip.attr("STEADY_STATE_BC_FILE_NAME") = ThermZip::SteadyStateBCFileName;
    zip.attr("TRANSIENT_TYPE_BC_FILE_NAME") = ThermZip::TransientTypeBCFileName;
    zip.attr("STEADY_STATE_RESULTS_NAME") = ThermZip::SteadyStateResultsName;
    zip.attr("STEADY_STATE_MESH_RESULTS_NAME") = ThermZip::SteadyStateMeshResultsName;
    zip.attr("MESH_NAME") = ThermZip::MeshName;
    zip.attr("CMA_LIBRARY") = ThermZip::CMALibrary;

    // In-memory zip: map<string, string> -> zip file
    zip.def("zip_files",
            py::overload_cast<const std::map<std::string, std::string> &, const std::string &>(
                &ThermZip::zipFiles),
            py::arg("file_contents"), py::arg("zip_file_name"),
            "Create a ZIP archive from in-memory file contents");

    // Unzip: zip file -> map<string, string>
    zip.def("unzip_files",
            py::overload_cast<std::string_view, std::vector<std::string> const &>(
                &ThermZip::unzipFiles),
            py::arg("source"), py::arg("file_names") = std::vector<std::string>(),
            "Extract files from a ZIP archive into a dict");

    // Unzip single file
    zip.def("unzip_file", &ThermZip::unzipFile,
            py::arg("zip_file_name"), py::arg("file_name"),
            "Extract a single file from a ZIP archive as a string");

    // Add file to existing zip
    zip.def("add_to_zip_file", &ThermZip::addToZipFile,
            py::arg("zip_file_name"), py::arg("file_name"), py::arg("text"),
            "Add a file to an existing ZIP archive");
}
