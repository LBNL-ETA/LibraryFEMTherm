"""Shared fixtures for LibraryFEMTherm Python binding tests."""

import os
import sys
import pytest

# Add the build output directory to sys.path so pylibraryfemtherm can be imported.
# FEMTHERM_MODULE_DIR points to the python/ source dir. The .so/.pyd lives
# either directly there (single-config: Ninja, Makefiles) or in a config
# subdirectory (multi-config: Visual Studio → Release/ or Debug/).
_python_dir = os.environ.get(
    "FEMTHERM_MODULE_DIR",
    os.path.join(os.path.dirname(__file__), ".."),
)
_search_dirs = [
    _python_dir,
    os.path.join(_python_dir, "Release"),
    os.path.join(_python_dir, "Debug"),
]
for _dir in _search_dirs:
    _abs = os.path.abspath(_dir)
    if _abs not in sys.path:
        sys.path.insert(0, _abs)

import pylibraryfemtherm as fem  # noqa: E402


@pytest.fixture()
def test_data_dir():
    """Return the path to the tst/products/ directory."""
    return os.path.normpath(
        os.path.join(os.path.dirname(__file__), "..", "..", "tst", "products")
    )


@pytest.fixture()
def sample_sill_path(test_data_dir):
    """Return the full path to sample-sill.thmz."""
    return os.path.join(test_data_dir, "sample-sill.thmz")


@pytest.fixture()
def sample_sill_no_results_path(test_data_dir):
    """Return the full path to sample-sill-no-results.thmz."""
    return os.path.join(test_data_dir, "sample-sill-no-results.thmz")


@pytest.fixture()
def id50_path(test_data_dir):
    """Return the full path to ID50_CS_single.thmz."""
    return os.path.join(test_data_dir, "ID50_CS_single.thmz")


@pytest.fixture()
def sample_model(sample_sill_path):
    """Load and return the sample-sill ThermModel."""
    result = fem.load_model_from_zip_file(sample_sill_path)
    assert result is not None
    return result
