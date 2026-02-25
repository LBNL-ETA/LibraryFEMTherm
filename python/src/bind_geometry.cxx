#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "THMZ/Geometry/Geometry.hxx"

namespace py = pybind11;

void bind_geometry(py::module_ & mod)
{
    py::class_<ThermFile::Point>(mod, "Point")
        .def(py::init<>())
        .def(py::init<double, double>())
        .def_readwrite("x", &ThermFile::Point::x)
        .def_readwrite("y", &ThermFile::Point::y)
        .def("__repr__", [](const ThermFile::Point & pnt) {
            return "Point(" + std::to_string(pnt.x) + ", " + std::to_string(pnt.y) + ")";
        });

    py::class_<ThermFile::Cavity>(mod, "Cavity")
        .def(py::init<>())
        .def_readwrite("uuid", &ThermFile::Cavity::uuid)
        .def_readwrite("direction", &ThermFile::Cavity::direction)
        .def_readwrite("heat_flow_direction", &ThermFile::Cavity::heatFlowDirection)
        .def_readwrite("emissivity1", &ThermFile::Cavity::emissivity1)
        .def_readwrite("emissivity2", &ThermFile::Cavity::emissivity2)
        .def_readwrite("temperature1", &ThermFile::Cavity::temperature1)
        .def_readwrite("temperature2", &ThermFile::Cavity::temperature2)
        .def_readwrite("max_x_dimension", &ThermFile::Cavity::maxXDimension)
        .def_readwrite("max_y_dimension", &ThermFile::Cavity::maxYDimension)
        .def_readwrite("actual_height", &ThermFile::Cavity::actualHeight)
        .def_readwrite("area", &ThermFile::Cavity::area)
        .def_readwrite("local_emissivities", &ThermFile::Cavity::localEmissivities)
        .def_readwrite("pressure", &ThermFile::Cavity::pressure)
        .def_readwrite("warm_locator", &ThermFile::Cavity::warmLocator)
        .def_readwrite("cold_locator", &ThermFile::Cavity::coldLocator);

    py::class_<ThermFile::GlazingSystemData>(mod, "GlazingSystemData")
        .def(py::init<>())
        .def_readwrite("id", &ThermFile::GlazingSystemData::ID)
        .def_readwrite("index", &ThermFile::GlazingSystemData::index);

    py::class_<ThermFile::Polygon>(mod, "Polygon")
        .def(py::init<>())
        .def_readwrite("uuid", &ThermFile::Polygon::uuid)
        .def_readwrite("id", &ThermFile::Polygon::ID)
        .def_readwrite("material_uuid", &ThermFile::Polygon::materialUUID)
        .def_readwrite("material_name", &ThermFile::Polygon::materialName)
        .def_readwrite("glazing_system", &ThermFile::Polygon::glazingSystem)
        .def_readwrite("origin", &ThermFile::Polygon::origin)
        .def_readwrite("points", &ThermFile::Polygon::points)
        .def_readwrite("cavity_uuid", &ThermFile::Polygon::cavityUUID)
        .def_readwrite("attributes", &ThermFile::Polygon::attributes)
        .def_readwrite("polygon_type", &ThermFile::Polygon::polygonType)
        .def_readwrite("link_id", &ThermFile::Polygon::linkID);

    py::class_<ThermFile::SurfaceData>(mod, "SurfaceData")
        .def(py::init<>())
        .def_readwrite("tilt", &ThermFile::SurfaceData::tilt)
        .def_readwrite("length", &ThermFile::SurfaceData::length);

    py::class_<ThermFile::BuildingData>(mod, "BuildingData")
        .def(py::init<>())
        .def_readwrite("width", &ThermFile::BuildingData::width)
        .def_readwrite("height", &ThermFile::BuildingData::height);

    py::class_<ThermFile::ThermalEmissionProperties>(mod, "ThermalEmissionProperties")
        .def(py::init<>())
        .def_readwrite("emissivity", &ThermFile::ThermalEmissionProperties::emissivity)
        .def_readwrite("temperature", &ThermFile::ThermalEmissionProperties::temperature)
        .def_readwrite("use_global_emissivity", &ThermFile::ThermalEmissionProperties::useGlobalEmissivity);

    py::class_<ThermFile::ShadeData>(mod, "ShadeData")
        .def(py::init<>())
        .def_readwrite("shade_modifier", &ThermFile::ShadeData::shadeModifier)
        .def_readwrite("shade_modifier_glazing_id", &ThermFile::ShadeData::shadeModifierGlazingID);

    py::class_<ThermFile::TransientData>(mod, "TransientData")
        .def(py::init<>())
        .def_readwrite("type_record_uuid", &ThermFile::TransientData::typeRecordUUID)
        .def_readwrite("transient_file_name", &ThermFile::TransientData::transientFileName)
        .def_readwrite("surface_data", &ThermFile::TransientData::surfaceData)
        .def_readwrite("building_data", &ThermFile::TransientData::buildingData);

    py::class_<ThermFile::Boundary>(mod, "Boundary")
        .def(py::init<>())
        .def_readwrite("id", &ThermFile::Boundary::ID)
        .def_readwrite("uuid", &ThermFile::Boundary::uuid)
        .def_readwrite("name", &ThermFile::Boundary::name)
        .def_readwrite("flux_tag", &ThermFile::Boundary::fluxTag)
        .def_readwrite("is_blocking", &ThermFile::Boundary::isBlocking)
        .def_readwrite("neighbor_polygon_uuid", &ThermFile::Boundary::neighborPolygonUUID)
        .def_readwrite("neighbor_polygon_uuid2", &ThermFile::Boundary::neighborPolygonUUID2)
        .def_readwrite("origin", &ThermFile::Boundary::origin)
        .def_readwrite("start_point", &ThermFile::Boundary::startPoint)
        .def_readwrite("end_point", &ThermFile::Boundary::endPoint)
        .def_readwrite("side", &ThermFile::Boundary::Side)
        .def_readwrite("thermal_emission_properties", &ThermFile::Boundary::thermalEmissionProperties)
        .def_readwrite("shade_data", &ThermFile::Boundary::shadeData)
        .def_readwrite("is_illuminated", &ThermFile::Boundary::isIlluminated)
        .def_readwrite("material_side", &ThermFile::Boundary::materialSide)
        .def_readwrite("transient_record_data", &ThermFile::Boundary::transientRecordData)
        .def_readwrite("edge_id", &ThermFile::Boundary::EdgeID)
        .def_readwrite("enclosure_number", &ThermFile::Boundary::enclosureNumber)
        .def_readwrite("surface_type", &ThermFile::Boundary::surfaceType)
        .def_readwrite("color", &ThermFile::Boundary::color)
        .def_readwrite("status", &ThermFile::Boundary::status);
}
