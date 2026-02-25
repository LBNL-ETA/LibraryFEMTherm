#include <pybind11/pybind11.h>

namespace py = pybind11;

void bind_enums(py::module_ & mod);
void bind_geometry(py::module_ & mod);
void bind_preferences(py::module_ & mod);
void bind_glazing_system(py::module_ & mod);
void bind_cma(py::module_ & mod);
void bind_properties(py::module_ & mod);
void bind_model(py::module_ & mod);
void bind_zip(py::module_ & mod);
void bind_materials(py::module_ & mod);
void bind_bc(py::module_ & mod);
void bind_gases(py::module_ & mod);
void bind_mesh(py::module_ & mod);
void bind_mesh_results(py::module_ & mod);
void bind_steady_state_results(py::module_ & mod);

PYBIND11_MODULE(_femtherm, mod)
{
    mod.doc() = "Python bindings for LibraryFEMTherm — THMZ load/save and library access";

    bind_enums(mod);
    bind_geometry(mod);
    bind_preferences(mod);
    bind_glazing_system(mod);
    bind_cma(mod);
    bind_properties(mod);
    bind_model(mod);
    bind_zip(mod);
    bind_materials(mod);
    bind_bc(mod);
    bind_gases(mod);
    bind_mesh(mod);
    bind_mesh_results(mod);
    bind_steady_state_results(mod);
}
