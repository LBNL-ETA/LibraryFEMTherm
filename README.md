# LibraryFEMTherm

C++20 static library for reading, writing, and manipulating THERM (.thmz) files. Provides data structures and serialization for thermal models, materials, boundary conditions, gases, glazing systems, and CMA data. Optional Python bindings via pybind11.

## Requirements

- CMake 3.8+
- C++20 compiler (MSVC 2022, GCC 11+, Clang 14+)
- Ninja or Visual Studio generator
- Python 3.11+ (only when building Python bindings)

## Building

### C++ only (default)

```bash
cmake --preset default
cmake --build build/default --config Release
```

### C++ with Python bindings

```bash
cmake --preset python
cmake --build build/python --config Release
```

### Manual configuration (Visual Studio generator)

```bash
cmake -G "Visual Studio 17 2022" -A x64 -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Add `-DBUILD_PYTHON_BINDINGS=ON` to include Python bindings.

## CMake presets

| Preset    | Description                  | Python bindings | C++ tests |
|-----------|------------------------------|-----------------|-----------|
| `default` | C++ only                     | OFF             | ON        |
| `python`  | C++ with Python bindings     | ON              | ON        |

CLion and VS Code automatically detect these presets.

## Running tests

```bash
ctest --test-dir build/python -C Release -V
```

This runs both C++ Google Tests and Python pytest tests (when built with the `python` preset).

## CMake options

| Option                        | Default | Description                          |
|-------------------------------|---------|--------------------------------------|
| `BUILD_PYTHON_BINDINGS`       | OFF     | Build Python bindings via pybind11   |
| `BUILD_LibraryFEMTHERM_TESTING` | ON   | Build C++ test targets               |

## Python bindings

When built with `BUILD_PYTHON_BINDINGS=ON`, a `_femtherm` Python extension module is produced in the `python/` directory.

```python
import sys
sys.path.insert(0, "python")  # or python/Release on Windows with MSVC
import _femtherm as fem

# Load a THMZ file
model = fem.load_model_from_zip_file("tst/products/sample-sill.thmz")
print(f"Polygons: {len(model.polygons)}, Boundaries: {len(model.boundaries)}")

# Save to XML string
xml = fem.save_model_to_string(model)

# Save to a new THMZ file
fem.save_model_to_zip_file(model, "output.thmz")

# Work with ZIP contents directly
contents = fem.zip.unzip_files("sample.thmz", ["Materials.xml"])
db = fem.MaterialsDB()
db.load_from_string(contents["Materials.xml"])
```

## Project structure

```
src/
  BCSteadyState/     # Steady-state boundary conditions
  BCTransient/       # Transient boundary conditions
  CMA/               # CMA (Component Modeling Approach) data
  Common/            # Shared utilities
  Gases/             # Gas properties and mixtures
  Materials/         # Material definitions (solid, cavity, radiation)
  THMZ/              # ThermModel, geometry, preferences, properties, zip I/O
  LibraryUtilities/  # Common helpers
  Schemas/           # XML/JSON schemas
python/
  src/               # pybind11 binding source files
  tests/             # pytest test suite
tst/
  units/             # C++ Google Test files
  products/          # Test THMZ data files
cmake/               # CMake macros and compiler config
```

## Dependencies (fetched automatically)

- [FileParse](https://github.com/LBNL-ETA/FileParse) - XML/JSON serialization
- [LBNLCPPCommon](https://github.com/LBNL-ETA/LBNLCPPCommon) - Common utilities
- [miniz](https://github.com/richgel999/miniz) - ZIP compression
- [pybind11](https://github.com/pybind/pybind11) v2.13.6 - Python bindings (optional)
- [Google Test](https://github.com/google/googletest) v1.13.0 - Testing (optional)
