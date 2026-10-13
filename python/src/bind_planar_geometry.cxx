#include <string>
#include <vector>

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Common/PlanarGeometry.hxx"

namespace py = pybind11;

namespace
{
    //! The coordinates of whatever Python passed as a point: anything with an x and a y,
    //! from this module, from pyhygrotherm, or of the caller's own.
    struct Coordinates
    {
        double x{0.0};
        double y{0.0};
    };

    [[nodiscard]] Coordinates coordinatesOf(const py::handle & point)
    {
        return {.x = point.attr("x").cast<double>(), .y = point.attr("y").cast<double>()};
    }

    [[nodiscard]] std::vector<Coordinates> coordinatesOf(const py::iterable & points)
    {
        std::vector<Coordinates> result;
        for(const auto & point : points)
        {
            result.push_back(coordinatesOf(point));
        }
        return result;
    }

    [[nodiscard]] std::string boxRepr(const Geometry::BoundingBox & box)
    {
        return "BoundingBox(min_x=" + std::to_string(box.minX) + ", min_y=" + std::to_string(box.minY)
               + ", max_x=" + std::to_string(box.maxX) + ", max_y=" + std::to_string(box.maxY) + ")";
    }
}   // namespace

void bind_planar_geometry(py::module_ & mod)
{
    auto geometry{mod.def_submodule(
      "geometry", "Plane geometry on any points with an x and a y: this module's, pyhygrotherm's, or your own.")};

    py::class_<Geometry::BoundingBox>(geometry, "BoundingBox", "Axis-aligned bounds; empty when it holds no point.")
      .def(py::init([](double minX, double minY, double maxX, double maxY) {
               return Geometry::BoundingBox{.minX = minX, .minY = minY, .maxX = maxX, .maxY = maxY};
           }),
           py::kw_only(),
           py::arg("min_x"),
           py::arg("min_y"),
           py::arg("max_x"),
           py::arg("max_y"))
      .def_readonly("min_x", &Geometry::BoundingBox::minX)
      .def_readonly("min_y", &Geometry::BoundingBox::minY)
      .def_readonly("max_x", &Geometry::BoundingBox::maxX)
      .def_readonly("max_y", &Geometry::BoundingBox::maxY)
      .def_property_readonly("width", &Geometry::BoundingBox::width)
      .def_property_readonly("height", &Geometry::BoundingBox::height)
      .def_property_readonly("empty", &Geometry::BoundingBox::empty)
      .def(py::self == py::self)
      .def("__repr__", &boxRepr);

    geometry.def(
      "bounding_box",
      [](const py::iterable & points) { return Geometry::boundingBox(coordinatesOf(points)); },
      py::arg("points"),
      "The bounds of the points; empty when there are none.");

    geometry.def(
      "distance_to_segment",
      [](const py::handle & point, const py::handle & start, const py::handle & end) {
          return Geometry::distanceToSegment(coordinatesOf(point), coordinatesOf(start), coordinatesOf(end));
      },
      py::arg("point"),
      py::arg("start"),
      py::arg("end"),
      "Shortest distance from the point to the segment between start and end.");

    geometry.def(
      "is_inside_polygon",
      [](const py::handle & point, const py::iterable & polygon) {
          return Geometry::isInsidePolygon(coordinatesOf(point), coordinatesOf(polygon));
      },
      py::arg("point"),
      py::arg("polygon"),
      "Even-odd ray casting; the polygon's last corner connects back to its first.");

    geometry.def(
      "polygon_area",
      [](const py::iterable & polygon) { return Geometry::polygonArea(coordinatesOf(polygon)); },
      py::arg("polygon"),
      "Enclosed area by the shoelace formula, the same for either winding.");
}
