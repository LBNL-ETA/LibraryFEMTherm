#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Model/BoundaryLookup.hxx"

namespace py = pybind11;

void bind_boundary_lookup(py::module_ & mod)
{
    mod.def("boundary_record",
            &ThermFile::boundaryRecord,
            py::arg("library"),
            py::arg("boundary"),
            "The record a model's boundary segment is bound to: by bc_uuid, or by the segment's name in an "
            "archive saved before the libraries were consolidated. None when the library holds neither.");
    mod.def("boundary_dataset",
            &ThermFile::boundaryDataset,
            py::arg("datasets"),
            py::arg("boundary"),
            "The dataset a model's boundary segment is bound to, by timeSeries_uuid. None when the segment "
            "binds none or no dataset carries that UUID. Pair it with boundary_record in missing_roles.");
}
