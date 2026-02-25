"""Tests ported from BCSteadyStateLibraryXMLLoading.unit.cxx and THMZFile.unit.cxx.

Uses sample-sill.thmz SteadyStateBC.xml and ID50_CS_single.thmz.
"""

import pytest

import _femtherm as fem

TOL = 1e-6


@pytest.fixture()
def bc_db_sill(sample_sill_path):
    """Load BCSteadyStateDB from the sample-sill THMZ."""
    contents = fem.zip.unzip_files(sample_sill_path, ["SteadyStateBC.xml"])
    bdb = fem.BCSteadyStateDB()
    bdb.load_from_string(contents["SteadyStateBC.xml"])
    return bdb


class TestBCDBLoading:
    """Ported from THMZFile.unit.cxx — TestSteadyStateBC::Deserialization."""

    def test_load_from_id50(self, id50_path):
        contents = fem.zip.unzip_files(id50_path, ["SteadyStateBC.xml"])
        bdb = fem.BCSteadyStateDB()
        bdb.load_from_string(contents["SteadyStateBC.xml"])
        names = bdb.get_names()
        assert len(names) == 3
        assert names[0] == "Adiabatic"
        assert names[1] == "ShadeInE"
        assert names[2] == "ShadeOutE"

    def test_get_names_sill(self, bc_db_sill):
        names = bc_db_sill.get_names()
        assert len(names) > 0

    def test_get_by_name(self, bc_db_sill):
        names = bc_db_sill.get_names()
        bcd = bc_db_sill.get_by_name(names[0])
        assert bcd is not None
        assert bcd.name == names[0]

    def test_get_by_uuid(self, bc_db_sill):
        bcs = bc_db_sill.get_boundary_conditions()
        assert len(bcs) > 0
        first = bcs[0]
        found = bc_db_sill.get_by_uuid(first.uuid)
        assert found is not None
        assert found.name == first.name

    def test_nonexistent_uuid_returns_none(self, bc_db_sill):
        result = bc_db_sill.get_by_uuid("00000000-0000-0000-0000-000000000000")
        assert result is None


class TestBCVariantTypes:
    """Ported from BCSteadyStateLibraryXMLLoading — type-specific tests."""

    def test_bc_data_is_valid_variant(self, bc_db_sill):
        """Every BC should have data of type Comprehensive, Simplified, or RadiationSurface."""
        for bcd in bc_db_sill.get_boundary_conditions():
            data = bcd.data
            assert isinstance(data, (fem.Comprehensive, fem.Simplified, fem.RadiationSurface))

    def test_simplified_bc_fields(self, bc_db_sill):
        """Find a Simplified BC and verify its fields."""
        for bcd in bc_db_sill.get_boundary_conditions():
            data = bcd.data
            if isinstance(data, fem.Simplified):
                assert isinstance(data.temperature, float)
                assert isinstance(data.film_coefficient, float)
                assert isinstance(data.relative_humidity, float)
                return
        pytest.skip("No Simplified BC in this DB")

    def test_comprehensive_bc_fields(self, bc_db_sill):
        """Find a Comprehensive BC and check convection/radiation."""
        for bcd in bc_db_sill.get_boundary_conditions():
            data = bcd.data
            if isinstance(data, fem.Comprehensive):
                assert isinstance(data.relative_humidity, float)
                if data.convection is not None:
                    assert isinstance(data.convection.temperature, float)
                    assert isinstance(data.convection.film_coefficient, float)
                return
        pytest.skip("No Comprehensive BC in this DB")


class TestBCDBSaveLoad:

    def test_xml_round_trip(self, bc_db_sill):
        xml_str = bc_db_sill.save_to_string(fem.FileFormat.XML)
        assert len(xml_str) > 0

        reloaded = fem.BCSteadyStateDB()
        reloaded.load_from_string(xml_str)
        assert bc_db_sill.get_names() == reloaded.get_names()

    def test_add_bc(self, bc_db_sill):
        original_count = len(bc_db_sill.get_boundary_conditions())

        bcd = fem.BCSteadyState()
        bcd.uuid = "test-bc-uuid-12345678"
        bcd.name = "Test Added BC"
        bcd.color = "0x00FF00"
        bcd.data = fem.Simplified()

        bc_db_sill.add(bcd)
        assert len(bc_db_sill.get_boundary_conditions()) == original_count + 1
