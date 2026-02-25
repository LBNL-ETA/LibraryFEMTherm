#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "THMZ/CMA/CMA.hxx"

namespace py = pybind11;

void bind_cma(py::module_ & mod)
{
    py::class_<ThermFile::CMAGapSpacer>(mod, "CMAGapSpacer")
        .def(py::init<>())
        .def_readwrite("glazing_case", &ThermFile::CMAGapSpacer::glazingCase)
        .def_readwrite("spacer_case", &ThermFile::CMAGapSpacer::spacerCase)
        .def_readwrite("gap_conductance", &ThermFile::CMAGapSpacer::gapConductance)
        .def_readwrite("spacer_conductivity", &ThermFile::CMAGapSpacer::spacerConductivity);

    py::class_<ThermFile::CMALayerInput>(mod, "CMALayerInput")
        .def(py::init<>())
        .def_readwrite("glazing_case", &ThermFile::CMALayerInput::glazingCase)
        .def_readwrite("spacer_case", &ThermFile::CMALayerInput::spacerCase)
        .def_readwrite("hcin", &ThermFile::CMALayerInput::hcin)
        .def_readwrite("hcout", &ThermFile::CMALayerInput::hcout)
        .def_readwrite("emissivity_in", &ThermFile::CMALayerInput::emissivityIn)
        .def_readwrite("emissivity_out", &ThermFile::CMALayerInput::emissivityOut);

    py::class_<ThermFile::CMASingleLayer>(mod, "CMASingleLayer")
        .def(py::init<>())
        .def_readwrite("thickness", &ThermFile::CMASingleLayer::thickness)
        .def_readwrite("conductivity", &ThermFile::CMASingleLayer::conductivity)
        .def_readwrite("option", &ThermFile::CMASingleLayer::option);

    py::class_<ThermFile::CMADoubleLayer>(mod, "CMADoubleLayer")
        .def(py::init<>())
        .def_readwrite("thickness_in", &ThermFile::CMADoubleLayer::thicknessIn)
        .def_readwrite("thickness_out", &ThermFile::CMADoubleLayer::thicknessOut)
        .def_readwrite("conductivity_in", &ThermFile::CMADoubleLayer::conductivityIn)
        .def_readwrite("conductivity_out", &ThermFile::CMADoubleLayer::conductivityOut)
        .def_readwrite("option", &ThermFile::CMADoubleLayer::option);

    py::class_<ThermFile::CMAData>(mod, "CMAData")
        .def(py::init<>())
        .def_readwrite("cma_bc_positions", &ThermFile::CMAData::cmaBCPositions)
        .def_readwrite("interior_temperature", &ThermFile::CMAData::interiorTemperature)
        .def_readwrite("exterior_temperature", &ThermFile::CMAData::exteriorTemperature)
        .def_readwrite("igu_type", &ThermFile::CMAData::iguType)
        .def_property("layer_input",
            [](const ThermFile::CMAData & cma) -> py::object {
                return std::visit([](const auto & val) -> py::object {
                    return py::cast(val);
                }, cma.layerInput);
            },
            [](ThermFile::CMAData & cma, py::object val) {
                try { cma.layerInput = val.cast<ThermFile::CMASingleLayer>(); return; } catch(py::cast_error &) {}
                try { cma.layerInput = val.cast<ThermFile::CMADoubleLayer>(); return; } catch(py::cast_error &) {}
                throw py::type_error("Expected CMASingleLayer or CMADoubleLayer");
            })
        .def_readwrite("gap_spacer_input", &ThermFile::CMAData::gapSpacerInput);
}
