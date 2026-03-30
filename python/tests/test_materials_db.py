"""Tests ported from MaterialsXMLReading.unit.cxx.

Uses the sample-sill.thmz Materials.xml extracted via ThermZip bindings,
mirroring how the C++ tests load from XML files.
"""

import pytest

import pylibraryfemtherm as fem

TOL = 1e-6


@pytest.fixture()
def materials_db(sample_sill_path):
    """Load MaterialsDB from the sample-sill THMZ."""
    contents = fem.zip.unzip_files(sample_sill_path, ["Materials.xml"])
    mdb = fem.MaterialsDB()
    mdb.load_from_string(contents["Materials.xml"])
    return mdb


class TestMaterialsDBLoading:

    def test_get_names_not_empty(self, materials_db):
        names = materials_db.get_names()
        assert len(names) > 0

    def test_get_by_name(self, materials_db):
        names = materials_db.get_names()
        mat = materials_db.get_by_name(names[0])
        assert mat is not None
        assert mat.name == names[0]

    def test_get_by_uuid(self, materials_db):
        materials = materials_db.get_materials()
        assert len(materials) > 0
        first = materials[0]
        found = materials_db.get_by_uuid(first.uuid)
        assert found is not None
        assert found.name == first.name

    def test_nonexistent_uuid_returns_none(self, materials_db):
        result = materials_db.get_by_uuid("00000000-0000-0000-0000-000000000000")
        assert result is None

    def test_solid_material_has_hygro_thermal(self, materials_db):
        """Ported from ReadMaterial1ByUUID — verify solid material fields."""
        materials = materials_db.get_materials()
        solid_found = False
        for mat in materials:
            if fem.is_solid(mat):
                solid_found = True
                data = mat.data
                assert isinstance(data, fem.Solid)
                assert data.hygro_thermal is not None
                break
        assert solid_found, "No solid material found in DB"

    def test_material_variant_types(self, materials_db):
        """Verify variant data field returns correct types."""
        materials = materials_db.get_materials()
        for mat in materials:
            data = mat.data
            assert isinstance(data, (fem.Solid, fem.MaterialCavity, fem.RadiationEnclosure))

    def test_cavity_material_fields(self, materials_db):
        """Check cavity materials have expected fields."""
        materials = materials_db.get_materials()
        for mat in materials:
            if fem.is_cavity(mat):
                data = mat.data
                assert isinstance(data, fem.MaterialCavity)
                assert isinstance(data.cavity_standard, fem.CavityStandard)
                return
        pytest.skip("No cavity material in this DB")


class TestMaterialDatabaseSource:

    def test_material_with_database_round_trip(self):
        """Create a material with Database/WINDOW source, save and reload."""
        mdb = fem.MaterialsDB()

        mat = fem.Material()
        mat.uuid = "d1e2f3a4-b5c6-7890-abcd-ef1234567890"
        mat.name = "Shade With DB Source"
        mat.color = "0xABCDEF"
        mat.data = fem.Solid()

        window = fem.WINDOW()
        window.path = r"C:\WINDOWdb\ShadeDB.mdb"
        window.name = "Roller Shade Dark"
        window.id = 42

        database = fem.Database()
        database.window = window
        mat.database = database

        mdb.add(mat)

        xml_str = mdb.save_to_string(fem.FileFormat.XML)

        reloaded = fem.MaterialsDB()
        reloaded.load_from_string(xml_str)

        found = reloaded.get_by_uuid("d1e2f3a4-b5c6-7890-abcd-ef1234567890")
        assert found is not None
        assert found.name == "Shade With DB Source"
        assert found.database is not None
        assert found.database.window is not None
        assert found.database.window.path == r"C:\WINDOWdb\ShadeDB.mdb"
        assert found.database.window.name == "Roller Shade Dark"
        assert found.database.window.id == 42

    def test_material_without_database_has_none(self):
        """Regular material should have no database source."""
        mdb = fem.MaterialsDB()

        mat = fem.Material()
        mat.uuid = "test-no-db-uuid"
        mat.name = "Plain Material"
        mat.data = fem.Solid()

        mdb.add(mat)

        xml_str = mdb.save_to_string(fem.FileFormat.XML)
        reloaded = fem.MaterialsDB()
        reloaded.load_from_string(xml_str)

        found = reloaded.get_by_uuid("test-no-db-uuid")
        assert found is not None
        assert found.database is None


class TestMaterialsDBSaveLoad:

    def test_save_and_reload_xml(self, materials_db):
        """Ported from MaterialsXMLSaving — XML round-trip."""
        xml_str = materials_db.save_to_string(fem.FileFormat.XML)
        assert len(xml_str) > 0

        reloaded = fem.MaterialsDB()
        reloaded.load_from_string(xml_str)
        assert materials_db.get_names() == reloaded.get_names()

    def test_save_and_reload_json(self, materials_db):
        json_str = materials_db.save_to_string(fem.FileFormat.JSON)
        assert len(json_str) > 0

    def test_add_material(self, materials_db):
        original_count = len(materials_db.get_materials())

        mat = fem.Material()
        mat.uuid = "test-uuid-12345678"
        mat.name = "Test Added Material"
        mat.color = "0xFF0000"
        solid = fem.Solid()
        mat.data = solid

        materials_db.add(mat)
        assert len(materials_db.get_materials()) == original_count + 1

        found = materials_db.get_by_uuid("test-uuid-12345678")
        assert found is not None
        assert found.name == "Test Added Material"

    def test_delete_material(self, materials_db):
        materials = materials_db.get_materials()
        original_count = len(materials)
        assert original_count > 0

        uuid_to_delete = materials[0].uuid
        materials_db.delete_with_uuid(uuid_to_delete)
        assert len(materials_db.get_materials()) == original_count - 1
