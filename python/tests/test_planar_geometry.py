"""pylibraryfemtherm.geometry: plane geometry on anything with an x and a y."""

from __future__ import annotations

from dataclasses import dataclass

import pytest

import pylibraryfemtherm as lft
from pylibraryfemtherm import geometry


@dataclass(frozen=True)
class Corner:
    """A point type of the test's own: the functions take anything with an x and a y."""

    x: float
    y: float


RECTANGLE: list[Corner] = [Corner(x=0.0, y=0.0), Corner(x=4.0, y=0.0), Corner(x=4.0, y=2.0), Corner(x=0.0, y=2.0)]


def test_bounding_box_of_points() -> None:
    box: geometry.BoundingBox = geometry.bounding_box(RECTANGLE)
    assert box == geometry.BoundingBox(min_x=0.0, min_y=0.0, max_x=4.0, max_y=2.0)
    assert (box.width, box.height) == (4.0, 2.0)
    assert not box.empty


def test_bounding_box_takes_a_generator_and_the_library_points() -> None:
    points: list[lft.Point] = [lft.Point(1.0, -1.0), lft.Point(3.0, 5.0)]
    box: geometry.BoundingBox = geometry.bounding_box(point for point in points)
    assert (box.min_x, box.min_y, box.max_x, box.max_y) == (1.0, -1.0, 3.0, 5.0)


def test_bounding_box_of_nothing_is_empty() -> None:
    box: geometry.BoundingBox = geometry.bounding_box([])
    assert box.empty
    assert box.width == 0.0


def test_distance_to_segment() -> None:
    assert geometry.distance_to_segment(Corner(x=1.0, y=3.0), RECTANGLE[0], RECTANGLE[1]) == pytest.approx(3.0)
    assert geometry.distance_to_segment(Corner(x=7.0, y=4.0), RECTANGLE[0], RECTANGLE[1]) == pytest.approx(5.0)


def test_inside_polygon_and_area() -> None:
    assert geometry.is_inside_polygon(Corner(x=1.0, y=1.0), RECTANGLE)
    assert not geometry.is_inside_polygon(Corner(x=5.0, y=1.0), RECTANGLE)
    assert geometry.polygon_area(RECTANGLE) == pytest.approx(8.0)
    assert geometry.polygon_area(list(reversed(RECTANGLE))) == pytest.approx(8.0)


def test_a_point_without_coordinates_is_refused() -> None:
    with pytest.raises(AttributeError):
        geometry.bounding_box([1.0, 2.0])
