# Python Bindings — Usage Guide

`pylibraryfemtherm` provides Python access to the LibraryFEMTherm C++ library for reading, writing, and manipulating THERM (`.thmz`) thermal simulation files.

## Installation

```bash
pip install libraryfemtherm
```

Or build from source:

```bash
cmake --preset python
cmake --build build/python --config Release
```

When building from source, add the output directory to your Python path:

```python
import sys
sys.path.insert(0, "python")          # Linux / macOS
sys.path.insert(0, "python/Release")  # Windows (MSVC)
```

## Quick Start

```python
import pylibraryfemtherm as fem

# Load a THMZ file
model = fem.load_model_from_zip_file("sample-sill.thmz")

# Inspect the model
print(f"Version:    {model.version}")
print(f"Polygons:   {len(model.polygons)}")
print(f"Boundaries: {len(model.boundary_conditions)}")
print(f"Cavities:   {len(model.cavities)}")
print(f"Glazing:    {len(model.glazing_systems)}")

# Access model properties
props = model.properties
print(f"Title:      {props.general.title}")
print(f"Engine:     {props.calculation_options.simulation_engine}")
print(f"Cross-sect: {props.model_exposure.exposure_type.cross_section}")

# Save to a new THMZ file
fem.save_model_to_zip_file(model, "output.thmz")
```

## Loading and Saving Models

### From/to THMZ files (ZIP archives)

```python
model = fem.load_model_from_zip_file("path/to/model.thmz")
fem.save_model_to_zip_file(model, "path/to/output.thmz")
```

### From/to XML or JSON strings

```python
# XML (default)
xml_str = fem.save_model_to_string(model)
model = fem.load_model_from_string(xml_str)

# JSON
json_str = fem.save_model_to_string(model, fem.FileFormat.JSON)
model = fem.load_model_from_string(json_str, fem.FileFormat.JSON)
```

### From/to XML files

```python
model = fem.load_model_from_file("Model.xml")
fem.save_model_to_file(model, "Model.xml")
```

## Working with ZIP Contents Directly

The `pylibraryfemtherm.zip` submodule gives low-level access to the THMZ archive:

```python
# Extract all files from a THMZ archive
contents = fem.zip.unzip_files("sample.thmz")
for name, xml in contents.items():
    print(f"{name}: {len(xml)} bytes")

# Extract specific files only
contents = fem.zip.unzip_files("sample.thmz", [
    fem.zip.MODEL_FILE_NAME,           # "Model.xml"
    fem.zip.MATERIALS_FILE_NAME,       # "Materials.xml"
])

# Extract a single file
xml = fem.zip.unzip_file("sample.thmz", fem.zip.MATERIALS_FILE_NAME)

# Create a new THMZ from a dictionary of {filename: xml_content}
fem.zip.zip_files({"Model.xml": xml_str, "Materials.xml": mat_str}, "new.thmz")

# Add or overwrite a file inside an existing THMZ
fem.zip.add_to_zip_file("existing.thmz", "Materials.xml", new_materials_xml)
```

### Standard file names inside a THMZ

| Constant                              | Value                       |
|---------------------------------------|-----------------------------|
| `fem.zip.MODEL_FILE_NAME`             | `"Model.xml"`               |
| `fem.zip.MATERIALS_FILE_NAME`         | `"Materials.xml"`           |
| `fem.zip.GASES_FILE_NAME`             | `"Gases.xml"`               |
| `fem.zip.STEADY_STATE_BC_FILE_NAME`   | `"SteadyStateBC.xml"`       |
| `fem.zip.MESH_NAME`                   | `"Mesh.xml"`                |
| `fem.zip.STEADY_STATE_RESULTS_NAME`   | `"SteadyStateResults.xml"`  |
| `fem.zip.STEADY_STATE_MESH_RESULTS_NAME` | `"MeshResults.xml"`      |
| `fem.zip.CMA_LIBRARY`                 | `"CMALibrary.xml"`          |

## Materials

### Loading the materials database

```python
# From a THMZ file
contents = fem.zip.unzip_files("model.thmz", [fem.zip.MATERIALS_FILE_NAME])
db = fem.MaterialsDB()
db.load_from_string(contents[fem.zip.MATERIALS_FILE_NAME])

# Or from a standalone XML file
db = fem.MaterialsDB("materials.xml")
```

### Querying materials

```python
# List all material names
print(db.get_names())

# Look up by name or UUID
mat = db.get_by_name("PVC/Vinyl")
mat = db.get_by_uuid("some-uuid-string")

# Check material type
if fem.is_solid(mat):
    solid = mat.data  # Solid object
    print(f"Conductivity: {solid.hygro_thermal.thermal_conductivity_dry}")
    print(f"Density:      {solid.hygro_thermal.bulk_density}")
elif fem.is_cavity(mat):
    cav = mat.data  # MaterialCavity object
    print(f"Standard:     {cav.cavity_standard}")
    print(f"Emissivity 1: {cav.emissivity_side1}")
elif fem.is_radiation_enclosure(mat):
    enc = mat.data  # RadiationEnclosure object
    print(f"Emissivity:   {enc.emissivity_default}")

# Iterate all materials
for mat in db.get_materials():
    print(f"{mat.name} (uuid={mat.uuid}, solid={fem.is_solid(mat)})")
```

### Creating and modifying materials

```python
mat = fem.Material()
mat.name = "Custom Insulation"

solid = fem.Solid()
solid.hygro_thermal.thermal_conductivity_dry = 0.04
solid.hygro_thermal.bulk_density = 30.0
mat.data = solid

db.add(mat)

# Save back to XML
xml = db.save_to_string()

# Or JSON
json_str = db.save_to_string(fem.FileFormat.JSON)
```

## Boundary Conditions

### Loading boundary conditions

```python
contents = fem.zip.unzip_files("model.thmz", [fem.zip.STEADY_STATE_BC_FILE_NAME])
db = fem.BCSteadyStateDB()
db.load_from_string(contents[fem.zip.STEADY_STATE_BC_FILE_NAME])
```

### Querying

```python
print(db.get_names())

bc = db.get_by_name("Interior")
print(f"UUID: {bc.uuid}")
print(f"Name: {bc.name}")

# Boundary conditions use variant types for their data
data = bc.data
if isinstance(data, fem.Comprehensive):
    print(f"Convection temp:    {data.convection.temperature}")
    print(f"Film coefficient:   {data.convection.film_coefficient}")
    print(f"Relative humidity:  {data.relative_humidity}")
    print(f"Constant flux:      {data.constant_flux.flux}")
elif isinstance(data, fem.Simplified):
    print(f"Temperature:        {data.temperature}")
    print(f"Film coefficient:   {data.film_coefficient}")
elif isinstance(data, fem.RadiationSurface):
    print(f"Temperature:        {data.temperature}")
    print(f"Emissivity:         {data.emissivity}")
```

### Creating boundary conditions

```python
bc = fem.BCSteadyState()
bc.name = "Custom Interior"

comp = fem.Comprehensive()
comp.convection.temperature = 21.0
comp.convection.film_coefficient = 8.29
comp.relative_humidity = 0.5
bc.data = comp

db.add(bc)
```

## Gases

### Loading the gas database

```python
contents = fem.zip.unzip_files("model.thmz", [fem.zip.GASES_FILE_NAME])
db = fem.GasesDB()
db.load_from_string(contents[fem.zip.GASES_FILE_NAME])
```

### Querying gases

```python
# Pure gases (e.g. Air, Argon, Krypton)
print(db.get_pure_gas_names())

air = db.get_pure_gas_by_name("Air")
print(f"Molecular weight: {air.properties.molecular_weight}")
print(f"Conductivity A:   {air.properties.conductivity.a}")

# Gas mixtures
print(db.get_names())
for gas in db.get_gases():
    print(f"{gas.name}:")
    for comp in gas.components:
        print(f"  {comp.pure_gas_name}: {comp.fraction * 100:.0f}%")
```

### Creating a pure gas

```python
gas = fem.PureGas()
gas.name = "Custom Gas"
gas.properties.molecular_weight = 28.97

coeffs = fem.GasCoefficients()
coeffs.a = 2.873e-3
coeffs.b = 7.76e-5
coeffs.c = 0.0
gas.properties.conductivity = coeffs

db.add_pure_gas(gas)
```

## Glazing Systems

```python
model = fem.load_model_from_zip_file("model.thmz")

for gs in model.glazing_systems:
    print(f"Glazing system: {gs.name} (id={gs.id})")
    print(f"  U-value:     {gs.properties.u_value}")
    print(f"  Orientation: {gs.orientation}")
    print(f"  Height:      {gs.actual_height}")

    # Glass layers
    for layer in gs.layers:
        print(f"  Layer: {layer.name}")
        print(f"    Thickness:    {layer.thickness}")
        print(f"    Conductivity: {layer.conductivity}")
        print(f"    Surface 1 T:  {layer.surface1.temperature}")
        print(f"    Surface 2 T:  {layer.surface2.temperature}")

    # Gas gaps between layers
    for gas in gs.gases:
        print(f"  Gap: {gas.name}, thickness={gas.thickness}")
        for comp in gas.components:
            props = comp.properties
            print(f"    {props.name}: {comp.fraction * 100:.0f}%")

        # Condensation resistance properties
        cr = gas.cr_properties
        print(f"    CR unsealed:   {cr.calc_unsealed_cr}")
        print(f"    BC temp:       {cr.bc_temperature}")
        print(f"    BC humidity:   {cr.bc_relative_humidity}")
```

## Geometry — Polygons and Boundaries

```python
model = fem.load_model_from_zip_file("model.thmz")

# Polygons represent material regions
for poly in model.polygons:
    print(f"Polygon: id={poly.id}, type={poly.polygon_type}")
    print(f"  Material: {poly.material_name} (uuid={poly.material_uuid})")
    print(f"  Origin:   ({poly.origin.x}, {poly.origin.y})")
    print(f"  Points:   {[(p.x, p.y) for p in poly.points]}")

# Boundaries represent edge conditions
for bnd in model.boundary_conditions:
    print(f"BC: {bnd.name} (uuid={bnd.uuid})")
```

## Model Properties and Preferences

```python
model = fem.load_model_from_zip_file("model.thmz")

# General info
gen = model.properties.general
print(f"Title:    {gen.title}")
print(f"Created:  {gen.creation_date}")
print(f"Company:  {gen.company}")
print(f"Notes:    {gen.notes}")

# Calculation options
calc = model.properties.calculation_options
print(f"Engine:           {calc.simulation_engine}")
print(f"Calculation mode: {calc.calculation_mode}")
print(f"Mesher:           {calc.mesh_control.mesh_type}")
print(f"Mesh parameter:   {calc.mesh_control.mesh_parameter}")
print(f"Convergence tol:  {calc.engine_parameters.convergence_tolerance}")

# Model exposure / cross-section type
exp = model.properties.model_exposure
print(f"Purpose:     {exp.exposure_type.model_purpose}")
print(f"Cross-sect:  {exp.exposure_type.cross_section}")
print(f"Orientation: {exp.model_orientation.orientation}")

# Preferences
prefs = model.preferences
print(f"Scale:            {prefs.settings.scale}")
print(f"U-value basis:    {prefs.settings.u_value_basis}")
print(f"Conductivity:     {prefs.settings.conductivity_units}")
print(f"Snap to grid:     {prefs.snap.snap_to_grid}")
```

## Mesh Data

### Loading

```python
# From THMZ
mesh = fem.load_mesh_from_zip_file("model.thmz")

# From XML string
mesh = fem.load_mesh_from_string(xml_str)
```

### Inspecting

```python
for case in mesh.cases:
    print(f"Case: type={case.model_type}, glazing={case.glazing_case}")
    print(f"  Nodes:    {len(case.nodes)}")
    print(f"  Elements: {len(case.elements)}")

    # Node coordinates
    for node in case.nodes[:5]:
        print(f"    Node {node.index}: ({node.x}, {node.y})")

    # Elements (quad or tri — node4 == 0 means triangle)
    for elem in case.elements[:5]:
        print(f"    Elem {elem.index}: [{elem.node1}, {elem.node2}, "
              f"{elem.node3}, {elem.node4}] mat={elem.material_id}")

    # Boundary condition segments
    segs = case.bc_segments
    if segs.fluxes:
        print(f"  Flux segments:      {len(segs.fluxes)}")
    if segs.convections:
        print(f"  Convection segments:{len(segs.convections)}")
    if segs.temperatures:
        print(f"  Temperature nodes:  {len(segs.temperatures)}")
```

### Saving

```python
xml_str = fem.save_mesh_to_string(mesh)
fem.save_mesh_to_zip_file(mesh, "output.thmz")
```

## Steady-State Results

### U-factor results

```python
results = fem.load_steady_state_results_from_zip_file("model.thmz")

for case in results.cases:
    print(f"Case: type={case.model_type}, error={case.simulation_error}")
    for uf in case.steady_state_u_factors:
        print(f"  Tag: {uf.tag}")
        print(f"    Delta-T:   {uf.delta_t}")
        print(f"    Heat flux:  {uf.heat_flux}")
        for proj in uf.projections:
            print(f"    {proj.length_type}: length={proj.length}, "
                  f"U={proj.u_factor}")
```

### Mesh results (temperatures and fluxes at nodes)

```python
mesh_results = fem.load_steady_state_mesh_results_from_zip_file("model.thmz")

for case in mesh_results.cases:
    print(f"Case: type={case.results_type}")
    print(f"  Node results: {len(case.nodes)}")

    # Temperature range
    temps = [n.temperature for n in case.nodes]
    print(f"  T range: {min(temps):.2f} to {max(temps):.2f}")

    # Heat flux at edges
    for edge in case.element_edges[:5]:
        print(f"  Edge {edge.index}: type={edge.segment_type}, "
              f"flux={edge.flux}, heat_rate={edge.heat_rate}")

    # Tagged node groups
    for tag_case in mesh_results.tag_nodes:
        for tag in tag_case.tag_nodes:
            print(f"  Tag '{tag.name}': {len(tag.nodes)} nodes")
```

## CMA Data

The Component Modeling Approach (CMA) data is stored in the model:

```python
model = fem.load_model_from_zip_file("model.thmz")
cma = model.cma_data

print(f"IGU type:      {cma.igu_type}")
print(f"Interior temp: {cma.interior_temperature}")
print(f"Exterior temp: {cma.exterior_temperature}")
print(f"BC positions:  {cma.cma_bc_positions}")

# Layer input (variant: CMASingleLayer or CMADoubleLayer)
layer = cma.layer_input
if isinstance(layer, fem.CMASingleLayer):
    print(f"Single layer: thickness={layer.thickness}, k={layer.conductivity}")
elif isinstance(layer, fem.CMADoubleLayer):
    print(f"Double layer: inner={layer.thickness_in}, outer={layer.thickness_out}")

# Gap/spacer input
gap = cma.gap_spacer_input
print(f"Gap conductance:     {gap.gap_conductance}")
print(f"Spacer conductivity: {gap.spacer_conductivity}")
```

## Creating a Model from Scratch

```python
import pylibraryfemtherm as fem

# Create an empty model
model = fem.ThermModel()
model.version = 1

# Add a polygon
poly = fem.Polygon()
poly.polygon_type = fem.PolygonType.Material
poly.points = [
    fem.Point(0, 0),
    fem.Point(100, 0),
    fem.Point(100, 50),
    fem.Point(0, 50),
]
model.polygons = [poly]

# Add a boundary condition
bc = fem.BCSteadyState()
bc.name = "Interior"
comp = fem.Comprehensive()
comp.convection.temperature = 21.0
comp.convection.film_coefficient = 8.29
bc.data = comp
model.boundary_conditions = [bc]

# Serialize
xml = fem.save_model_to_string(model)
print(xml)
```

## Enumerations Reference

### Geometry

| Enum            | Values |
|-----------------|--------|
| `Direction`     | `Unknown`, `Right`, `Left`, `Up`, `Down` |
| `SurfaceType`   | `Unknown`, `NotASurface`, `BoundaryCondition`, `GlazingCavity`, `FrameCavity`, `Enclosure` |
| `PolygonType`   | `Unknown`, `None_`, `Material`, `Glass`, `Gap`, `BoundaryCondition`, `Spacer`, `GlassIn`, `GlassOut`, `ShadeGap` |
| `ShadeModifier` | `None`, `Exterior`, `Interior` |
| `MaterialSide`  | `Unknown`, `Front`, `Back` |

### Model Properties

| Enum                      | Values |
|---------------------------|--------|
| `ModelPurpose`            | `Unknown`, `Window_TransparentFacade`, `OpaqueFacade`, `Other` |
| `SimulationEngine`        | `Unknown`, `Conrad`, `HygroThermFEM` |
| `CalculationMode`         | `Unknown`, `cmTransient`, `cmSteadyState` |
| `MesherType`              | `Unknown`, `QuadTreeMesher`, `Simmetrix_Version_2022` |
| `Orientation`             | `Unknown`, `North`, `East`, `South`, `West` |
| `WindowCrossSectionType`  | `Unknown`, `Sill`, `Jamb`, `Head`, `HorizontalDivider`, `VerticalDivider`, `HorizontalMeetingRail`, `VerticalMeetingRail`, `CommonFrame`, `Spacer` |
| `GravityOrientation`      | `Unknown`, `Down`, `Up`, `Left`, `Right`, `IntoScreen`, `OutOfScreen` |

### Preferences

| Enum                | Values |
|---------------------|--------|
| `UnitSystem`        | `SI`, `IP` |
| `ConductivityUnits` | `BtuPerHr_Ft_Fahrenheit`, `Btu_InchPerHour_FootSquared_Fahrenheit` |
| `UValueBasisType`   | `TotalLength`, `XProjected`, `YProjected`, `Custom`, `GlassRotationProjected`, `CustomRotationProjected` |

### Materials

| Enum               | Values |
|--------------------|--------|
| `MaterialType`     | `Solid`, `Cavity`, `RadiationEnclosure` |
| `MaterialRoughness`| `VeryRough`, `Rough`, `MediumRough`, `MediumSmooth`, `Smooth`, `VerySmooth` |
| `CavityStandard`   | `NFRC`, `CEN`, `CENVentilated`, `NFRCWithUserDimensions`, `ISO15099`, `ISO15099Ventilated` |

### Glazing Systems

| Enum                     | Values |
|--------------------------|--------|
| `EnvironmentalCondition` | `Unknown`, `Winter`, `Summer` |
| `Position`               | `Unknown`, `Interior`, `Exterior` |
| `BoundaryConditionStyle` | `Unknown`, `Combined`, `Split`, `BlackBody`, `Enclosure`, `Existing` |

### CMA

| Enum          | Values |
|---------------|--------|
| `CMABCPosition` | `None_`, `Interior`, `Exterior` |
| `CMAIGUType`    | `Unknown`, `SingleLayer`, `DoubleLayer` |
| `CMACase`       | `None_`, `Low`, `High` |

### Results

| Enum                  | Values |
|-----------------------|--------|
| `RunType`             | `Unknown`, `UFactor`, `CondensationResistance` |
| `SegmentType`         | `Unknown`, `Flux`, `Temperature`, `Convection`, `Radiation`, `RadiationEnclosure` |
| `UValueDimensionType` | `Unknown`, `TotalLength`, `XProjected`, `YProjected`, `Custom`, `GlassRotationProjected`, `CustomRotationProjected` |

### File Format

| Enum         | Values |
|--------------|--------|
| `FileFormat` | `Unknown`, `XML`, `JSON` |
