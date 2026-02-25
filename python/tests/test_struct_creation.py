"""Tests for creating and manipulating C++ structs from Python.

Verifies default constructors, field assignment, and variant setters work.
"""

import pylibraryfemtherm as fem

TOL = 1e-6


class TestPointCreation:

    def test_default_constructor(self):
        pnt = fem.Point()
        assert abs(pnt.x) < TOL
        assert abs(pnt.y) < TOL

    def test_parameterized_constructor(self):
        pnt = fem.Point(3.14, 2.71)
        assert abs(pnt.x - 3.14) < TOL
        assert abs(pnt.y - 2.71) < TOL

    def test_repr(self):
        pnt = fem.Point(1.0, 2.0)
        rep = repr(pnt)
        assert "1.0" in rep
        assert "2.0" in rep

    def test_field_assignment(self):
        pnt = fem.Point()
        pnt.x = 100.5
        pnt.y = -200.3
        assert abs(pnt.x - 100.5) < TOL
        assert abs(pnt.y + 200.3) < TOL


class TestPolygonCreation:

    def test_default_polygon(self):
        poly = fem.Polygon()
        assert poly.uuid == ""
        assert poly.id == -1
        assert len(poly.points) == 0
        assert poly.polygon_type == fem.PolygonType.None_

    def test_add_points(self):
        poly = fem.Polygon()
        poly.points = [fem.Point(0, 0), fem.Point(10, 0), fem.Point(10, 10), fem.Point(0, 10)]
        assert len(poly.points) == 4


class TestMaterialCreation:

    def test_create_solid_material(self):
        mat = fem.Material()
        mat.uuid = "my-uuid"
        mat.name = "My Solid"
        mat.data = fem.Solid()
        assert fem.is_solid(mat)
        assert not fem.is_cavity(mat)

    def test_create_cavity_material(self):
        mat = fem.Material()
        mat.data = fem.MaterialCavity()
        assert fem.is_cavity(mat)
        assert not fem.is_solid(mat)

    def test_create_radiation_enclosure_material(self):
        mat = fem.Material()
        mat.data = fem.RadiationEnclosure()
        assert fem.is_radiation_enclosure(mat)

    def test_switch_variant_type(self):
        mat = fem.Material()
        mat.data = fem.Solid()
        assert fem.is_solid(mat)
        mat.data = fem.MaterialCavity()
        assert fem.is_cavity(mat)


class TestBCCreation:

    def test_create_simplified(self):
        bcd = fem.BCSteadyState()
        simplified = fem.Simplified()
        simplified.temperature = 21.0
        simplified.film_coefficient = 8.0
        simplified.relative_humidity = 0.5
        bcd.data = simplified
        assert isinstance(bcd.data, fem.Simplified)
        assert abs(bcd.data.temperature - 21.0) < TOL

    def test_create_comprehensive(self):
        bcd = fem.BCSteadyState()
        comp = fem.Comprehensive()
        comp.relative_humidity = 0.5
        conv = fem.Convection()
        conv.temperature = 21.0
        conv.film_coefficient = 4.65
        comp.convection = conv
        bcd.data = comp
        assert isinstance(bcd.data, fem.Comprehensive)
        assert bcd.data.convection is not None
        assert abs(bcd.data.convection.temperature - 21.0) < TOL


class TestThermModelCreation:

    def test_default_model(self):
        model = fem.ThermModel()
        assert model.calculation_ready is False
        assert len(model.polygons) == 0
        assert len(model.boundary_conditions) == 0
        assert len(model.cavities) == 0
        assert model.cma_data is None

    def test_build_model_from_scratch(self):
        model = fem.ThermModel()
        model.calculation_ready = True
        model.version = "1"

        poly = fem.Polygon()
        poly.uuid = "poly-1"
        poly.points = [fem.Point(0, 0), fem.Point(100, 0), fem.Point(100, 100), fem.Point(0, 100)]
        model.polygons = [poly]

        assert len(model.polygons) == 1
        assert model.polygons[0].uuid == "poly-1"

        xml = fem.save_model_to_string(model)
        assert len(xml) > 0
        assert "<CalculationReady>true</CalculationReady>" in xml
