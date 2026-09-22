"""Reading a transient solve's node results back out of a THMZ.

The C++ unit tests hold the parser to the format; these hold the Python surface: the
quantities as an enum, series as nested lists, nodes in column order, and the error
paths surfacing as ValueError.
"""

import os

import pytest

import pylibraryfemtherm as fem
from pylibraryfemtherm import transient_results as tr

TOL = 1e-12


@pytest.fixture()
def strip_path(test_data_dir):
    """A one-element isothermal strip solved for 24 hourly steps: 22 nodes, 25 frames."""
    return os.path.join(test_data_dir, "transient-isothermal-strip.thmz")


class TestFixture:
    def test_reads_every_quantity(self, strip_path):
        results = tr.read_from_zip_file(strip_path)
        assert results.node_count == 22
        assert results.frame_count == 25
        assert results.first_frame_is_initial
        assert results.frames[0] == 0 and results.frames[-1] == 24
        assert results.has(tr.Quantity.Temperature)
        assert not results.has(tr.Quantity.IceContent)

        temperature = results[tr.Quantity.Temperature]
        assert len(temperature.values) == 25
        assert len(temperature.values[0]) == 22
        assert all(abs(value - 20.0) < TOL for row in temperature.values for value in row)
        assert all(abs(value - 0.7) < TOL for row in results[tr.Quantity.Humidity].values for value in row)
        assert len(temperature.solve_error) == 25

        assert len(results.heat_flux) == 25 and len(results.heat_flux[0]) == 22
        assert isinstance(results.heat_flux[0][0], tr.FluxVector)

    def test_nodes_are_in_column_order(self, strip_path):
        results = tr.read_from_zip_file(strip_path)
        assert [node.id for node in results.nodes] == list(range(1, 23))
        assert len({node.x for node in results.nodes}) == 11
        assert results.geometry.nodes[0].id >= 1

    def test_entries_and_zip_agree(self, strip_path):
        entries = fem.zip.unzip_files(strip_path)
        assert tr.has_transient_results(entries)
        from_entries = tr.read(entries)
        from_zip = tr.read_from_zip_file(strip_path)
        assert from_entries.frames == from_zip.frames
        assert from_entries[tr.Quantity.WaterContent].values == from_zip[tr.Quantity.WaterContent].values

    def test_missing_quantity_is_a_key_error(self, strip_path):
        results = tr.read_from_zip_file(strip_path)
        with pytest.raises(KeyError):
            results[tr.Quantity.IceContent]

    def test_entry_names(self):
        assert tr.entry_name(tr.Quantity.Humidity) == "transient results/Humidities.csv"
        assert tr.entry_name(tr.Quantity.IceContent) == "transient results/IceContent.csv"


class TestErrors:
    def test_a_steady_archive_raises(self, sample_sill_path):
        assert not tr.has_transient_results(fem.zip.unzip_files(sample_sill_path))
        with pytest.raises(ValueError, match="transient results"):
            tr.read_from_zip_file(sample_sill_path)

    def test_inconsistent_entries_raise(self, strip_path):
        entries = fem.zip.unzip_files(strip_path)
        entries["transient results/Humidities.csv"] = "timestep #,error tolerance,1\n0,0,1\n"
        with pytest.raises(ValueError, match="node columns"):
            tr.read(entries)
