"""Tests ported from GasesXMLReading.unit.cxx.

Uses sample-sill.thmz Gases.xml extracted via ThermZip bindings.
"""

import pytest

import _femtherm as fem

TOL = 1e-6


@pytest.fixture()
def gases_db(sample_sill_path):
    """Load GasesDB from the sample-sill THMZ."""
    contents = fem.zip.unzip_files(sample_sill_path, ["Gases.xml"])
    gdb = fem.GasesDB()
    gdb.load_from_string(contents["Gases.xml"])
    return gdb


class TestGasesDBLoading:

    def test_get_names_not_empty(self, gases_db):
        names = gases_db.get_names()
        assert len(names) > 0

    def test_get_pure_gas_names_not_empty(self, gases_db):
        names = gases_db.get_pure_gas_names()
        assert len(names) > 0

    def test_get_pure_gas_by_name(self, gases_db):
        """Ported from ReadPureGasByUUID — verify pure gas properties."""
        names = gases_db.get_pure_gas_names()
        pure = gases_db.get_pure_gas_by_name(names[0])
        assert pure is not None
        assert pure.name == names[0]
        assert pure.properties.molecular_weight > 0

    def test_pure_gas_coefficients(self, gases_db):
        """Verify conductivity/viscosity/specific_heat coefficients accessible."""
        names = gases_db.get_pure_gas_names()
        pure = gases_db.get_pure_gas_by_name(names[0])
        assert pure is not None
        props = pure.properties
        assert isinstance(props.conductivity, fem.GasCoefficients)
        assert isinstance(props.viscosity, fem.GasCoefficients)
        assert isinstance(props.specific_heat, fem.GasCoefficients)

    def test_nonexistent_pure_gas_returns_none(self, gases_db):
        result = gases_db.get_pure_gas_by_uuid("00000000-0000-0000-0000-000000000000")
        assert result is None

    def test_gas_mixture_components(self, gases_db):
        """Ported from ReadGasMixture — verify gas has components."""
        gases = gases_db.get_gases()
        assert len(gases) > 0
        gas = gases[0]
        assert len(gas.components) > 0
        for comp in gas.components:
            assert len(comp.pure_gas_name) > 0
            assert 0.0 <= comp.fraction <= 1.0

    def test_get_by_uuid(self, gases_db):
        gases = gases_db.get_gases()
        assert len(gases) > 0
        found = gases_db.get_by_uuid(gases[0].uuid)
        assert found is not None
        assert found.gas.name == gases[0].name


class TestGasesDBSaveLoad:

    def test_xml_round_trip(self, gases_db):
        """Ported from GasesXMLSaving — save and reload."""
        xml_str = gases_db.save_to_string(fem.FileFormat.XML)
        assert len(xml_str) > 0

        reloaded = fem.GasesDB()
        reloaded.load_from_string(xml_str)
        assert gases_db.get_names() == reloaded.get_names()
        assert gases_db.get_pure_gas_names() == reloaded.get_pure_gas_names()

    def test_add_pure_gas(self, gases_db):
        original_count = len(gases_db.get_pure_gases())

        pure = fem.PureGas()
        pure.uuid = "test-pure-gas-uuid"
        pure.name = "TestGas"
        pure.properties = fem.GasProperties()
        pure.properties.molecular_weight = 42.0

        gases_db.add_pure_gas(pure)
        assert len(gases_db.get_pure_gases()) == original_count + 1

        found = gases_db.get_pure_gas_by_name("TestGas")
        assert found is not None
        assert abs(found.properties.molecular_weight - 42.0) < TOL
