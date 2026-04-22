"""Tests ported from THMZFile.unit.cxx and THMXFile.unit.cxx.

Covers THMZ load, XML round-trip, and save-to-zip round-trip.
"""

import os
import tempfile

import pytest

import pylibraryfemtherm as fem


class TestTHMZLoad:
    """Ported from THMZFile.unit.cxx — TestSteadyStateBC::Deserialization."""

    def test_load_sample_sill(self, sample_model):
        assert sample_model.calculation_ready is True
        assert len(sample_model.polygons) == 18
        assert len(sample_model.boundary_conditions) == 64
        assert len(sample_model.cavities) == 5

    def test_load_id50(self, id50_path):
        model = fem.load_model_from_zip_file(id50_path)
        assert model is not None
        assert len(model.polygons) > 0

    def test_load_nonexistent_file_returns_none(self):
        result = fem.load_model_from_zip_file("nonexistent_file.thmz")
        assert result is None


class TestXMLRoundTrip:
    """Ported from THMXFile.unit.cxx — XMLOperations test."""

    def test_save_to_string_and_reload(self, sample_model):
        xml = fem.save_model_to_string(sample_model)
        assert len(xml) > 0
        assert "<CalculationReady>" in xml

        reloaded = fem.load_model_from_string(xml)
        assert reloaded is not None
        assert len(reloaded.polygons) == len(sample_model.polygons)
        assert len(reloaded.boundary_conditions) == len(sample_model.boundary_conditions)
        assert len(reloaded.cavities) == len(sample_model.cavities)
        assert reloaded.calculation_ready == sample_model.calculation_ready

    def test_save_to_string_json_format(self, sample_model):
        json_str = fem.save_model_to_string(sample_model, fem.FileFormat.JSON)
        assert len(json_str) > 0
        reloaded = fem.load_model_from_string(json_str, fem.FileFormat.JSON)
        assert reloaded is not None
        assert len(reloaded.polygons) == len(sample_model.polygons)


class TestZipRoundTrip:
    """Ported from THMXFile.unit.cxx — file I/O tests."""

    def test_save_and_reload_zip(self, sample_model):
        with tempfile.NamedTemporaryFile(suffix=".thmz", delete=False) as tmp:
            tmp_path = tmp.name
        os.remove(tmp_path)  # miniz needs to create the file itself

        try:
            fem.save_model_to_zip_file(sample_model, tmp_path)
            reloaded = fem.load_model_from_zip_file(tmp_path)
            assert reloaded is not None
            assert len(reloaded.polygons) == len(sample_model.polygons)
            assert len(reloaded.boundary_conditions) == len(sample_model.boundary_conditions)
            assert reloaded.calculation_ready == sample_model.calculation_ready
        finally:
            os.remove(tmp_path)

    def test_polygon_fields_survive_round_trip(self, sample_model):
        """Verify polygon fields are preserved through save/load."""
        xml = fem.save_model_to_string(sample_model)
        reloaded = fem.load_model_from_string(xml)

        for idx in range(len(sample_model.polygons)):
            orig = sample_model.polygons[idx]
            copy = reloaded.polygons[idx]
            assert orig.uuid == copy.uuid
            assert orig.material_name == copy.material_name
            assert len(orig.points) == len(copy.points)
            for jdx in range(len(orig.points)):
                assert abs(orig.points[jdx].x - copy.points[jdx].x) < 1e-6
                assert abs(orig.points[jdx].y - copy.points[jdx].y) < 1e-6

    def test_gasket_materials_survive_round_trip(self, sample_model):
        """Verify optional gasket_materials set is preserved through save/load."""
        sample_model.gasket_materials = {"Butyl rubber", "EPDM", "Silicone rubber"}

        xml = fem.save_model_to_string(sample_model)
        reloaded = fem.load_model_from_string(xml)

        assert reloaded.gasket_materials == {"Butyl rubber", "EPDM", "Silicone rubber"}


class TestModelFields:
    """Verify model struct fields are accessible."""

    def test_version(self, sample_model):
        assert sample_model.version == "1"

    def test_preferences_accessible(self, sample_model):
        prefs = sample_model.preferences
        assert prefs.settings is not None
        assert isinstance(prefs.settings.origin, fem.Point)

    def test_properties_accessible(self, sample_model):
        props = sample_model.properties
        assert props.general is not None
        assert isinstance(props.calculation_options.simulation_engine, fem.SimulationEngine)

    def test_glazing_systems(self, sample_model):
        assert len(sample_model.glazing_systems) > 0
        gls = sample_model.glazing_systems[0]
        assert len(gls.name) > 0
        assert len(gls.layers) > 0
