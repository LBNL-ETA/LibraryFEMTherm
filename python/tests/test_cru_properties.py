"""Tests ported from CRUDeserialization.unit.cxx.

Verifies CRU (Condensation Resistance - Unsealed) properties
are correctly loaded from the sample-sill.thmz glazing system.
"""

import pytest

import pylibraryfemtherm as fem

TOL = 1e-6


class TestCRUDeserialization:
    """Ported from CRUDeserialization::LoadFromTHMZFile."""

    def test_glazing_system_has_gases(self, sample_model):
        glazing_systems = sample_model.glazing_systems
        assert len(glazing_systems) > 0
        gases = glazing_systems[0].gases
        assert len(gases) > 0

    def test_first_gas_has_cr_properties(self, sample_model):
        first_gas = sample_model.glazing_systems[0].gases[0]
        assert first_gas.cr_properties is not None

    def test_cr_properties_values(self, sample_model):
        crp = sample_model.glazing_systems[0].gases[0].cr_properties
        assert crp.calc_unsealed_cr is True
        assert crp.use_custom_relative_humidity_ratio is False
        assert crp.use_unsealed_toward_interior is True
        assert abs(crp.custom_relative_humidity_ratio - 0.0) < TOL
        assert abs(crp.bc_relative_humidity - 0.5) < TOL
        assert abs(crp.bc_temperature - 21.0) < TOL
        assert len(crp.bc_name) > 0
