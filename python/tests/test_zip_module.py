"""Tests ported from ZippingStringToFile.unit.cxx.

Covers ThermZip in-memory zip/unzip, add_to_zip_file, and file constants.
"""

import os
import tempfile

import pytest

import pylibraryfemtherm as fem


class TestZipConstants:
    """Verify ThermZip file name constants are exposed."""

    def test_model_file_name(self):
        assert fem.zip.MODEL_FILE_NAME == "Model.xml"

    def test_materials_file_name(self):
        assert fem.zip.MATERIALS_FILE_NAME == "Materials.xml"

    def test_gases_file_name(self):
        assert fem.zip.GASES_FILE_NAME == "Gases.xml"

    def test_steady_state_bc_file_name(self):
        assert fem.zip.STEADY_STATE_BC_FILE_NAME == "SteadyStateBC.xml"

    def test_mesh_name(self):
        assert fem.zip.MESH_NAME == "Mesh.xml"


class TestZipInMemory:
    """Ported from ZippingStringToFile.unit.cxx — in-memory zip operations."""

    def test_zip_and_unzip_files(self):
        """Create a zip from a dict, then extract back."""
        contents = {
            "file1.xml": "<root>hello</root>",
            "file2.txt": "plain text content",
        }

        with tempfile.NamedTemporaryFile(suffix=".zip", delete=False) as tmp:
            tmp_path = tmp.name

        try:
            fem.zip.zip_files(contents, tmp_path)
            extracted = fem.zip.unzip_files(tmp_path)
            assert extracted["file1.xml"] == contents["file1.xml"]
            assert extracted["file2.txt"] == contents["file2.txt"]
        finally:
            os.remove(tmp_path)

    def test_unzip_specific_files(self):
        """Extract only specific files from a zip."""
        contents = {
            "a.xml": "<a/>",
            "b.xml": "<b/>",
            "c.xml": "<c/>",
        }

        with tempfile.NamedTemporaryFile(suffix=".zip", delete=False) as tmp:
            tmp_path = tmp.name

        try:
            fem.zip.zip_files(contents, tmp_path)
            extracted = fem.zip.unzip_files(tmp_path, ["a.xml", "c.xml"])
            assert "a.xml" in extracted
            assert "c.xml" in extracted
            assert extracted["a.xml"] == "<a/>"
            assert extracted["c.xml"] == "<c/>"
        finally:
            os.remove(tmp_path)

    def test_unzip_single_file(self):
        """Extract a single file as a string."""
        contents = {"data.xml": "<data>123</data>"}

        with tempfile.NamedTemporaryFile(suffix=".zip", delete=False) as tmp:
            tmp_path = tmp.name

        try:
            fem.zip.zip_files(contents, tmp_path)
            result = fem.zip.unzip_file(tmp_path, "data.xml")
            assert result == "<data>123</data>"
        finally:
            os.remove(tmp_path)


class TestAddToZipFile:
    """Ported from ZippingStringToFile.unit.cxx — AddAndVerifyFileContent etc."""

    def test_add_and_verify(self):
        test_string = "< test test >"
        file_name = "Test.xml"

        with tempfile.NamedTemporaryFile(suffix=".zip", delete=False) as tmp:
            zip_path = tmp.name
        os.remove(zip_path)  # miniz needs to create the file itself

        try:
            result = fem.zip.add_to_zip_file(zip_path, file_name, test_string)
            assert result == 1

            extracted = fem.zip.unzip_files(zip_path, [file_name])
            assert file_name in extracted
            assert extracted[file_name] == test_string
        finally:
            os.remove(zip_path)

    def test_overwrite_existing_file(self):
        """Ported from OverwriteExistingFile test."""
        initial = "< initial content >"
        updated = "< updated content >"
        file_name = "Test.xml"

        with tempfile.NamedTemporaryFile(suffix=".zip", delete=False) as tmp:
            zip_path = tmp.name
        os.remove(zip_path)

        try:
            fem.zip.add_to_zip_file(zip_path, file_name, initial)
            fem.zip.add_to_zip_file(zip_path, file_name, updated)

            extracted = fem.zip.unzip_files(zip_path, [file_name])
            assert extracted[file_name] == updated
        finally:
            os.remove(zip_path)

    def test_handle_empty_string(self):
        """Ported from HandleEmptyString test."""
        file_name = "Empty.xml"

        with tempfile.NamedTemporaryFile(suffix=".zip", delete=False) as tmp:
            zip_path = tmp.name
        os.remove(zip_path)

        try:
            result = fem.zip.add_to_zip_file(zip_path, file_name, "")
            assert result == 1

            extracted = fem.zip.unzip_files(zip_path, [file_name])
            assert file_name in extracted
            assert extracted[file_name] == ""
        finally:
            os.remove(zip_path)


class TestUnzipFromTHMZ:
    """Test extracting known files from real THMZ archives."""

    def test_extract_model_xml(self, sample_sill_path):
        contents = fem.zip.unzip_files(sample_sill_path, ["Model.xml"])
        assert "Model.xml" in contents
        assert "<ThermModel>" in contents["Model.xml"]

    def test_extract_all_library_files(self, sample_sill_path):
        file_names = [
            fem.zip.MODEL_FILE_NAME,
            fem.zip.MATERIALS_FILE_NAME,
            fem.zip.GASES_FILE_NAME,
            fem.zip.STEADY_STATE_BC_FILE_NAME,
        ]
        contents = fem.zip.unzip_files(sample_sill_path, file_names)
        for name in file_names:
            assert name in contents
            assert len(contents[name]) > 0
