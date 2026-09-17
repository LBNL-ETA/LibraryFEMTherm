# LibraryFEMTherm

C++20 static library for reading, writing, and manipulating THERM (.thmz) files. Provides data structures and serialization for thermal models, materials, boundary conditions, gases, glazing systems, and CMA data. Optional Python bindings via pybind11.

## Requirements

- CMake 3.8+
- C++20 compiler (MSVC 2022, GCC 11+, Clang 14+)
- Ninja or Visual Studio generator
- Python 3.11+ (only when building Python bindings)

## Building

### CMake presets

`CMakePresets.json` ships six visible configure presets plus three hidden inheritance bases, each with a matching build preset (and a test preset for the release variants). Use the `default-*` presets for C++-only builds, `local-*` for day-to-day development against sibling working copies of LBNL deps, and `python-*` to build the bindings the way the published wheel does:

| Preset | Description | Python bindings | C++ tests |
|---|---|---|---|
| `default-debug` / `default-release` | C++ only. Fetches all dependencies from declared remotes. | OFF | ON |
| `local-debug` / `local-release` | Development: consume sibling `../LBNLCPPCommon` and `../FileParse` working copies when present, and build the Python bindings from the same tree. | ON | ON |
| `python-debug` / `python-release` | C++ with Python bindings. Fetches all dependencies from declared remotes. | ON | ON |

Missing siblings under `local-*` fall back to the declared remote automatically, so `local-*` is safe to invoke even if you don't have the LBNL siblings checked out. `local-*` needs a Python 3.11+ interpreter on the machine, since it builds the bindings; use `default-*` where there is none.

```bash
cmake --preset default-release
cmake --build --preset default-release --parallel
ctest --preset default-release
```

For development, including the Python bindings:

```bash
cmake --preset local-release
cmake --build --preset local-release --parallel
cmake --build --preset local-release --target pylibraryfemtherm   # bindings only
ctest --preset local-release
```

The bindings module is written to `python/` (`python/Release/` with a multi-config generator such as Visual Studio).

`python-release` is the same with every dependency fetched from its remote instead of a sibling, which is how the PyPI wheel is built; the wheel itself is produced by scikit-build-core from `pyproject.toml` and does not read this presets file.

```bash
cmake --preset python-release
cmake --build --preset python-release --parallel
ctest --preset python-release
```

CLion and VS Code automatically detect these presets.

### Per-machine compiler presets (`CMakeUserPresets.json`)

To use a specific compiler (`vs2022-release`, `gcc-13-debug`, `clang-18-release`, etc.), each developer maintains their own `CMakeUserPresets.json` next to `CMakePresets.json`. It is gitignored, read automatically by CMake (and CLion, VS Code, etc.), and stays on the developer's machine.

Personal presets `inherit` from one of the shipped presets (usually `local`, which gives you sibling-repo overrides for free, or `python` if you want bindings) and override whatever they want. A complete realistic example -- building with WSL Clang on a Windows machine, with CLion 2023.2+ routed through the WSL toolchain automatically:

```json
{
    "version": 6,
    "configurePresets": [
        {
            "name": "clang-release",
            "displayName": "clang (Release)",
            "inherits": "local",
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/build/clang-release",
            "cacheVariables": {
                "CMAKE_C_COMPILER":   "clang",
                "CMAKE_CXX_COMPILER": "clang++",
                "CMAKE_BUILD_TYPE":   "Release"
            },
            "vendor": {
                "jetbrains.com/clion": {
                    "toolchain": "WSL"
                }
            }
        }
    ]
}
```

### Manual configuration (without presets)

```bash
cmake -G "Visual Studio 17 2022" -A x64 -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Add `-DBUILD_PYTHON_BINDINGS=ON` to include Python bindings.

## CMake options

| Option                          | Default | Description                          |
|---------------------------------|---------|--------------------------------------|
| `BUILD_PYTHON_BINDINGS`         | OFF     | Build Python bindings via pybind11   |
| `BUILD_LibraryFEMTHERM_TESTING` | ON      | Build C++ test targets               |

## Python bindings

Install from PyPI:

```bash
pip install libraryfemtherm
```

Or build from source with `BUILD_PYTHON_BINDINGS=ON` (see above).

```python
import pylibraryfemtherm as fem

# Load a THMZ file
model = fem.load_model_from_zip_file("sample-sill.thmz")
print(f"Polygons:   {len(model.polygons)}")
print(f"Boundaries: {len(model.boundary_conditions)}")

# Save to XML string
xml = fem.save_model_to_string(model)

# Save to a new THMZ file
fem.save_model_to_zip_file(model, "output.thmz")

# Work with ZIP contents directly
contents = fem.zip.unzip_files("sample.thmz", [fem.zip.MATERIALS_FILE_NAME])
db = fem.MaterialsDB()
db.load_from_string(contents[fem.zip.MATERIALS_FILE_NAME])
```

For comprehensive Python API documentation with examples, see [doc/python.md](doc/python.md).

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
