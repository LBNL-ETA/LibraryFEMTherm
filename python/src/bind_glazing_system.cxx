#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "THMZ/GlazingSystem/GlazingSystem.hxx"

namespace py = pybind11;

void bind_glazing_system(py::module_ & mod)
{
    py::class_<ThermFile::EnvironmentCoefficients>(mod, "EnvironmentCoefficients")
        .def(py::init<>())
        .def_readwrite("h", &ThermFile::EnvironmentCoefficients::h)
        .def_readwrite("hc", &ThermFile::EnvironmentCoefficients::hc)
        .def_readwrite("hr", &ThermFile::EnvironmentCoefficients::hr)
        .def_readwrite("temperature", &ThermFile::EnvironmentCoefficients::temperature);

    py::class_<ThermFile::GlazingSystemProperties>(mod, "GlazingSystemProperties")
        .def(py::init<>())
        .def_readwrite("environmental_condition", &ThermFile::GlazingSystemProperties::environmentalCondition)
        .def_readwrite("boundary_condition_style", &ThermFile::GlazingSystemProperties::boundaryConditionStyle)
        .def_readwrite("u_value", &ThermFile::GlazingSystemProperties::uValue)
        .def_readwrite("coefficients", &ThermFile::GlazingSystemProperties::coefficients)
        .def_readwrite("solar", &ThermFile::GlazingSystemProperties::solar);

    py::class_<ThermFile::IGUSurface>(mod, "IGUSurface")
        .def(py::init<>())
        .def_readwrite("temperature", &ThermFile::IGUSurface::temperature)
        .def_readwrite("emissivity", &ThermFile::IGUSurface::emissivity);

    py::class_<ThermFile::IGULayer>(mod, "IGULayer")
        .def(py::init<>())
        .def_readwrite("id", &ThermFile::IGULayer::ID)
        .def_readwrite("name", &ThermFile::IGULayer::name)
        .def_readwrite("thickness", &ThermFile::IGULayer::thickness)
        .def_readwrite("conductivity", &ThermFile::IGULayer::conductivity)
        .def_readwrite("surface1", &ThermFile::IGULayer::surface1)
        .def_readwrite("surface2", &ThermFile::IGULayer::surface2);

    py::class_<ThermFile::IGUGasCoefficients>(mod, "IGUGasCoefficients")
        .def(py::init<>())
        .def_readwrite("a", &ThermFile::IGUGasCoefficients::A)
        .def_readwrite("b", &ThermFile::IGUGasCoefficients::B)
        .def_readwrite("c", &ThermFile::IGUGasCoefficients::C);

    py::class_<ThermFile::IGUGasProperties>(mod, "IGUGasProperties")
        .def(py::init<>())
        .def_readwrite("name", &ThermFile::IGUGasProperties::name)
        .def_readwrite("molecular_weight", &ThermFile::IGUGasProperties::molecularWeight)
        .def_readwrite("conductivity", &ThermFile::IGUGasProperties::conductivity)
        .def_readwrite("viscosity", &ThermFile::IGUGasProperties::viscosity)
        .def_readwrite("specific_heat", &ThermFile::IGUGasProperties::specificHeat);

    py::class_<ThermFile::IGUPureGas>(mod, "IGUPureGas")
        .def(py::init<>())
        .def_readwrite("fraction", &ThermFile::IGUPureGas::fraction)
        .def_readwrite("properties", &ThermFile::IGUPureGas::properties);

    py::class_<ThermFile::CavityCRProperties>(mod, "CavityCRProperties")
        .def(py::init<>())
        .def_readwrite("calc_unsealed_cr", &ThermFile::CavityCRProperties::calcUnsealedCR)
        .def_readwrite("use_custom_relative_humidity_ratio", &ThermFile::CavityCRProperties::useCustomRelativeHumidityRatio)
        .def_readwrite("use_unsealed_toward_interior", &ThermFile::CavityCRProperties::useUnsealedTowardInterior)
        .def_readwrite("custom_relative_humidity_ratio", &ThermFile::CavityCRProperties::customRelativeHumidityRatio)
        .def_readwrite("bc_name", &ThermFile::CavityCRProperties::bcName)
        .def_readwrite("bc_relative_humidity", &ThermFile::CavityCRProperties::bcRelativeHumidity)
        .def_readwrite("bc_temperature", &ThermFile::CavityCRProperties::bcTemperature);

    py::class_<ThermFile::IGUGas>(mod, "IGUGas")
        .def(py::init<>())
        .def_readwrite("id", &ThermFile::IGUGas::ID)
        .def_readwrite("name", &ThermFile::IGUGas::name)
        .def_readwrite("thickness", &ThermFile::IGUGas::thickness)
        .def_readwrite("conductivity", &ThermFile::IGUGas::conductivity)
        .def_readwrite("components", &ThermFile::IGUGas::components)
        .def_readwrite("cr_properties", &ThermFile::IGUGas::crProperties);

    py::class_<ThermFile::ShadeProperties>(mod, "ShadeProperties")
        .def(py::init<>())
        .def_readwrite("d_bottom", &ThermFile::ShadeProperties::dBottom)
        .def_readwrite("d_top", &ThermFile::ShadeProperties::dTop)
        .def_readwrite("d_left", &ThermFile::ShadeProperties::dLeft)
        .def_readwrite("d_right", &ThermFile::ShadeProperties::dRight)
        .def_readwrite("gap_keff", &ThermFile::ShadeProperties::gapKeff);

    py::class_<ThermFile::IGUShadeLayer>(mod, "IGUShadeLayer")
        .def(py::init<>())
        .def_readwrite("position", &ThermFile::IGUShadeLayer::position)
        .def_readwrite("emissivity_ratio", &ThermFile::IGUShadeLayer::emissivityRatio)
        .def_readwrite("hc_ratio", &ThermFile::IGUShadeLayer::hcRatio)
        .def_readwrite("thickness", &ThermFile::IGUShadeLayer::thickness)
        .def_readwrite("gap_thickness", &ThermFile::IGUShadeLayer::gapThickness)
        .def_readwrite("modeled_height", &ThermFile::IGUShadeLayer::modeledHeight)
        .def_readwrite("properties", &ThermFile::IGUShadeLayer::properties);

    py::class_<ThermFile::GlazingSystem>(mod, "GlazingSystem")
        .def(py::init<>())
        .def_readwrite("id", &ThermFile::GlazingSystem::ID)
        .def_readwrite("index", &ThermFile::GlazingSystem::index)
        .def_readwrite("name", &ThermFile::GlazingSystem::name)
        .def_readwrite("properties", &ThermFile::GlazingSystem::properties)
        .def_readwrite("layers", &ThermFile::GlazingSystem::layers)
        .def_readwrite("gases", &ThermFile::GlazingSystem::gases)
        .def_readwrite("shade_layer", &ThermFile::GlazingSystem::shadeLayer)
        .def_readwrite("actual_height", &ThermFile::GlazingSystem::actualHeight)
        .def_readwrite("orientation", &ThermFile::GlazingSystem::orientation)
        .def_readwrite("is_nominal_thickness", &ThermFile::GlazingSystem::isNominalThickness)
        .def_readwrite("interior_bc", &ThermFile::GlazingSystem::interiorBC)
        .def_readwrite("exterior_bc", &ThermFile::GlazingSystem::exteriorBC)
        .def_readwrite("option_flag", &ThermFile::GlazingSystem::optionFlag)
        .def_readwrite("source", &ThermFile::GlazingSystem::source)
        .def_readwrite("site_line_height", &ThermFile::GlazingSystem::siteLineHeight)
        .def_readwrite("eog_height", &ThermFile::GlazingSystem::eogHeight)
        .def_readwrite("sight_line_to_shade_edge", &ThermFile::GlazingSystem::sightLineToShadeEdge)
        .def_readwrite("is_deflected", &ThermFile::GlazingSystem::isDeflected)
        .def_readwrite("edge_glass_tag", &ThermFile::GlazingSystem::edgeGlassTag)
        .def_readwrite("glazing_option_ids", &ThermFile::GlazingSystem::glazingOptionIDs);
}
