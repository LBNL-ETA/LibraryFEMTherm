#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "THMZ/ZipModule/ZipModule.hxx"

namespace py = pybind11;

void bind_zip(py::module_ & mod)
{
    auto thmz = mod.def_submodule("thmz", "THERM archive (.thmz) operations: its entries, read and written by name");

    // Entry base names
    thmz.attr("MODEL_FILE_NAME") = ThermZip::ModelFileName;
    thmz.attr("GASES_FILE_NAME") = ThermZip::GasesFileName;
    thmz.attr("MATERIALS_FILE_NAME") = ThermZip::MaterialsFileName;
    thmz.attr("STEADY_STATE_BC_FILE_NAME") = ThermZip::SteadyStateBCFileName;
    thmz.attr("TRANSIENT_TYPE_BC_FILE_NAME") = ThermZip::TransientTypeBCFileName;
    thmz.attr("STEADY_STATE_RESULTS_NAME") = ThermZip::SteadyStateResultsName;
    thmz.attr("STEADY_STATE_MESH_RESULTS_NAME") = ThermZip::SteadyStateMeshResultsName;
    thmz.attr("MESH_NAME") = ThermZip::MeshName;
    thmz.attr("CMA_LIBRARY") = ThermZip::CMALibrary;
    thmz.attr("BOUNDARY_CONDITIONS_FILE_NAME") = ThermZip::BoundaryConditionsFileName;
    thmz.attr("TIME_SERIES_DIR") = ThermZip::TimeSeriesDir;

    thmz.def("time_series_entry_name", &ThermZip::timeSeriesEntryName,
             py::arg("dataset_uuid"), py::arg("format") = FileParse::FileFormat::XML,
             "Archive entry name of one time series dataset: \"time series/<uuid>.<ext>\"");
    thmz.def("entry_name_for_format", &ThermZip::entryNameForFormat,
             py::arg("base_name"), py::arg("format"),
             "Appends the format's extension to an entry base name (\"Model\" -> \"Model.xml\")");

    thmz.def("write_entries",
             py::overload_cast<const std::map<std::string, std::string> &, const std::string &, int>(
                 &ThermZip::zipFiles),
             py::arg("entries"), py::arg("thmz_file_name"),
             py::arg("compression_level") = ThermZip::DefaultCompressionLevel,
             "Write a .thmz holding the given entries, name -> content");

    thmz.def("read_entries",
             py::overload_cast<std::string_view, std::vector<std::string> const &>(
                 &ThermZip::unzipFiles),
             py::arg("thmz_file_name"), py::arg("names") = std::vector<std::string>(),
             "The .thmz's entries as a dict, name -> content: the named ones, or all when none are named");

    thmz.def("read_entry", &ThermZip::unzipFile,
             py::arg("thmz_file_name"), py::arg("name"),
             "One entry of a .thmz, as a string");

    thmz.def("add_entry", &ThermZip::addToZipFile,
             py::arg("thmz_file_name"), py::arg("name"), py::arg("text"),
             py::arg("obsolete_names") = std::vector<std::string>(),
             py::arg("compression_level") = ThermZip::DefaultCompressionLevel,
             "Add an entry to an existing .thmz, or replace it; obsolete_names are removed");

    // Entry base names are extension-free; these resolve the format spellings.
    thmz.def("entry_name_candidates", &ThermZip::entryNameCandidates,
             py::arg("base_name"),
             "Both format spellings of an entry base name, JSON first");
    thmz.def("find_entry", &ThermZip::findEntry,
             py::arg("entries"), py::arg("base_name"),
             "Look up an entry base name in an extracted-archive dict; empty when absent");
}
