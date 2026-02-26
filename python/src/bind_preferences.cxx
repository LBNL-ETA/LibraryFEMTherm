#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "THMZ/Preferences/Preferences.hxx"

namespace py = pybind11;

void bind_preferences(py::module_ & mod)
{
    py::class_<ThermFile::Underlay>(mod, "Underlay")
        .def(py::init<>())
        .def_readwrite("name", &ThermFile::Underlay::name)
        .def_readwrite("is_underlay_on", &ThermFile::Underlay::isUnderlayOn)
        .def_readwrite("unit_system", &ThermFile::Underlay::unitSystem)
        .def_readwrite("origin", &ThermFile::Underlay::origin)
        .def_readwrite("underlay_type", &ThermFile::Underlay::underlayType)
        .def_readwrite("horizontal_scale", &ThermFile::Underlay::horizontalScale)
        .def_readwrite("vertical_scale", &ThermFile::Underlay::verticalScale);

    py::class_<ThermFile::Settings>(mod, "Settings")
        .def(py::init<>())
        .def_readwrite("origin", &ThermFile::Settings::origin)
        .def_readwrite("scale", &ThermFile::Settings::scale)
        .def_readwrite("conductivity_units", &ThermFile::Settings::conductivityUnits)
        .def_readwrite("u_value_basis", &ThermFile::Settings::uValueBasis)
        .def_readwrite("condensation_resistance_model", &ThermFile::Settings::condensationResistanceModel);

    py::class_<ThermFile::Snap>(mod, "Snap")
        .def(py::init<>())
        .def_readwrite("snap_to_grid", &ThermFile::Snap::snapToGrid)
        .def_readwrite("smart_snap", &ThermFile::Snap::smartSnap)
        .def_readwrite("snap_to_underlay", &ThermFile::Snap::snapToUnderlay)
        .def_readwrite("snap_to_vertical", &ThermFile::Snap::snapToVertical)
        .def_readwrite("snap_to_horizontal", &ThermFile::Snap::snapToHorizontal)
        .def_readwrite("snap_to_angle", &ThermFile::Snap::snapToAngle)
        .def_readwrite("vertical_and_horizontal_lock", &ThermFile::Snap::verticalAndHorizontalLock)
        .def_readwrite("draw_grid", &ThermFile::Snap::drawGrid)
        .def_readwrite("snap_angle", &ThermFile::Snap::snapAngle)
        .def_readwrite("grid_width", &ThermFile::Snap::gridWidth)
        .def_readwrite("grid_height", &ThermFile::Snap::gridHeight)
        .def_readwrite("origin", &ThermFile::Snap::origin);

    py::class_<ThermFile::Preferences>(mod, "Preferences")
        .def(py::init<>())
        .def_readwrite("underlay", &ThermFile::Preferences::underlay)
        .def_readwrite("settings", &ThermFile::Preferences::settings)
        .def_readwrite("snap", &ThermFile::Preferences::snap);
}
