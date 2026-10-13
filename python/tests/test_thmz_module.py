"""Tests ported from ZippingStringToFile.unit.cxx.

Covers pylibraryfemtherm.thmz: writing and reading entries, add_entry, and the entry names.
"""

import os
import tempfile

import pytest

import pylibraryfemtherm as fem


class TestThmzConstants:
    """Verify ThermZip entry base names are exposed without an extension.

    An archive entry is stored as either Model.xml or Model.json depending on
    the save format, so the constants name the entry without one and
    entry_name_candidates/find_entry resolve the format spelling.
    """

    def test_model_file_name(self):
        assert fem.thmz.MODEL_FILE_NAME == "Model"

    def test_materials_file_name(self):
        assert fem.thmz.MATERIALS_FILE_NAME == "Materials"

    def test_gases_file_name(self):
        assert fem.thmz.GASES_FILE_NAME == "Gases"

    def test_steady_state_bc_file_name(self):
        assert fem.thmz.STEADY_STATE_BC_FILE_NAME == "SteadyStateBC"

    def test_mesh_name(self):
        assert fem.thmz.MESH_NAME == "Mesh"


class TestEntryNameResolution:
    """Base names resolve to the spelling actually present in the archive."""

    def test_candidates_cover_both_formats(self):
        candidates = fem.thmz.entry_name_candidates(fem.thmz.MODEL_FILE_NAME)
        assert set(candidates) == {"Model.json", "Model.xml"}

    def test_find_entry_matches_either_format(self):
        for name in ("Model.xml", "Model.json"):
            assert fem.thmz.find_entry({name: "content"}, fem.thmz.MODEL_FILE_NAME) == "content"

    def test_find_entry_returns_empty_when_absent(self):
        assert fem.thmz.find_entry({"Other.xml": "content"}, fem.thmz.MODEL_FILE_NAME) == ""


class TestThmzEntries:
    """Ported from ZippingStringToFile.unit.cxx — writing and reading .thmz entries."""

    def test_write_and_read_entries(self):
        """Write a .thmz from a dict, then read the entries back."""
        contents = {
            "file1.xml": "<root>hello</root>",
            "file2.txt": "plain text content",
        }

        with tempfile.NamedTemporaryFile(suffix=".thmz", delete=False) as tmp:
            tmp_path = tmp.name

        try:
            fem.thmz.write_entries(contents, tmp_path)
            extracted = fem.thmz.read_entries(tmp_path)
            assert extracted["file1.xml"] == contents["file1.xml"]
            assert extracted["file2.txt"] == contents["file2.txt"]
        finally:
            os.remove(tmp_path)

    def test_read_named_entries(self):
        """Read only the named entries."""
        contents = {
            "a.xml": "<a/>",
            "b.xml": "<b/>",
            "c.xml": "<c/>",
        }

        with tempfile.NamedTemporaryFile(suffix=".thmz", delete=False) as tmp:
            tmp_path = tmp.name

        try:
            fem.thmz.write_entries(contents, tmp_path)
            extracted = fem.thmz.read_entries(tmp_path, ["a.xml", "c.xml"])
            assert "a.xml" in extracted
            assert "c.xml" in extracted
            assert extracted["a.xml"] == "<a/>"
            assert extracted["c.xml"] == "<c/>"
        finally:
            os.remove(tmp_path)

    def test_read_one_entry(self):
        """Extract a single file as a string."""
        contents = {"data.xml": "<data>123</data>"}

        with tempfile.NamedTemporaryFile(suffix=".thmz", delete=False) as tmp:
            tmp_path = tmp.name

        try:
            fem.thmz.write_entries(contents, tmp_path)
            result = fem.thmz.read_entry(tmp_path, "data.xml")
            assert result == "<data>123</data>"
        finally:
            os.remove(tmp_path)


class TestAddEntry:
    """Ported from ZippingStringToFile.unit.cxx — AddAndVerifyFileContent etc."""

    def test_add_and_verify(self):
        test_string = "< test test >"
        file_name = "Test.xml"

        with tempfile.NamedTemporaryFile(suffix=".thmz", delete=False) as tmp:
            thmz_path = tmp.name
        os.remove(thmz_path)  # miniz needs to create the file itself

        try:
            result = fem.thmz.add_entry(thmz_path, file_name, test_string)
            assert result == 1

            extracted = fem.thmz.read_entries(thmz_path, [file_name])
            assert file_name in extracted
            assert extracted[file_name] == test_string
        finally:
            os.remove(thmz_path)

    def test_overwrite_existing_file(self):
        """Ported from OverwriteExistingFile test."""
        initial = "< initial content >"
        updated = "< updated content >"
        file_name = "Test.xml"

        with tempfile.NamedTemporaryFile(suffix=".thmz", delete=False) as tmp:
            thmz_path = tmp.name
        os.remove(thmz_path)

        try:
            fem.thmz.add_entry(thmz_path, file_name, initial)
            fem.thmz.add_entry(thmz_path, file_name, updated)

            extracted = fem.thmz.read_entries(thmz_path, [file_name])
            assert extracted[file_name] == updated
        finally:
            os.remove(thmz_path)

    def test_handle_empty_string(self):
        """Ported from HandleEmptyString test."""
        file_name = "Empty.xml"

        with tempfile.NamedTemporaryFile(suffix=".thmz", delete=False) as tmp:
            thmz_path = tmp.name
        os.remove(thmz_path)

        try:
            result = fem.thmz.add_entry(thmz_path, file_name, "")
            assert result == 1

            extracted = fem.thmz.read_entries(thmz_path, [file_name])
            assert file_name in extracted
            assert extracted[file_name] == ""
        finally:
            os.remove(thmz_path)


class TestReadSampleEntries:
    """Test extracting known files from real THMZ archives."""

    def test_extract_model_xml(self, sample_sill_path):
        contents = fem.thmz.read_entries(sample_sill_path, ["Model.xml"])
        assert "Model.xml" in contents
        assert "<ThermModel>" in contents["Model.xml"]

    def test_extract_all_library_files(self, sample_sill_path):
        base_names = [
            fem.thmz.MODEL_FILE_NAME,
            fem.thmz.MATERIALS_FILE_NAME,
            fem.thmz.GASES_FILE_NAME,
            fem.thmz.STEADY_STATE_BC_FILE_NAME,
        ]
        # read_entries matches entry names exactly, so ask for both spellings
        # of every base name and let find_entry pick whichever the archive uses.
        requested = [
            candidate
            for base in base_names
            for candidate in fem.thmz.entry_name_candidates(base)
        ]
        contents = fem.thmz.read_entries(sample_sill_path, requested)
        for base in base_names:
            assert len(fem.thmz.find_entry(contents, base)) > 0
