"""Stating a model in physical terms and writing it as a THMZ.

The C++ unit tests hold the builder to what the archive must contain; these hold the
Python surface: keyword construction, tuples where points and curve samples go,
defaults, value equality, and the two error paths surfacing as ValueError.
"""

import os

import pytest

import pylibraryfemtherm as fem
from pylibraryfemtherm import build, model

TOL = 1e-12


def linear_sorption() -> model.Material:
    return model.Material(
        name="linear-sorption",
        diffusion_resistance_factor=10.0,
        sorption_curve=[(0.0, 0.0), (1.0, 100.0)],
        density=1000.0,
        heat_capacity=1000.0,
        thermal_conductivity=1.0,
    )


def sealed_strip() -> model.ModelCase:
    """One rectangle, four segments, prescribed temperature on the two short ends."""
    return model.ModelCase(
        id="sealed_strip_gradient",
        regions=[model.Region(linear_sorption(), [(0.0, 0.0), (0.1, 0.0), (0.1, 0.005), (0.0, 0.005)])],
        segments=[
            model.Segment(model.Adiabatic(), (0.0, 0.0), (0.1, 0.0)),
            model.Segment(model.Prescribed(temperature=20.0), (0.1, 0.0), (0.1, 0.005)),
            model.Segment(model.Adiabatic(), (0.1, 0.005), (0.0, 0.005)),
            model.Segment(model.Prescribed(temperature=40.0), (0.0, 0.005), (0.0, 0.0)),
        ],
        initial=model.Initial(temperature=30.0, humidity=0.4),
        schedule=model.Schedule(dtime=36000.0, n_steps=300),
        physics=model.Physics(liquid_transport=False, heat_of_evaporation=False),
    )


class TestVocabulary:
    def test_tuples_become_points(self):
        region = model.Region(linear_sorption(), [(0.0, 0.0), (1.0, 0.0), (1.0, 1.0)])
        assert region.points[1] == model.Point(1.0, 0.0)
        assert region.points[1].x == 1.0
        assert region.material.name == "linear-sorption"
        segment = model.Segment(model.Adiabatic(), (0.0, 0.0), (1.0, 0.0))
        assert segment.end == model.Point(1.0, 0.0)
        assert segment.region is None
        assert model.Segment(model.Adiabatic(), (0.0, 0.0), (1.0, 0.0), region=1).region == 1

    def test_segments_attach_to_the_region_found(self):
        case = sealed_strip()
        assert [model.segment_region(case, index) for index in range(4)] == [0, 0, 0, 0]
        assert model.segment_region(case, 4) is None

    def test_shared_edge_must_name_its_region(self):
        case = sealed_strip()
        case.regions = list(case.regions) + [
            model.Region(linear_sorption(), [(0.1, 0.0), (0.2, 0.0), (0.2, 0.005), (0.1, 0.005)])
        ]
        found = model.issues(case)
        assert len(found) == 1
        assert "shared by regions 0 and 1" in found[0]
        assert model.segment_region(case, 1) is None
        segments = list(case.segments)
        segments[1] = model.Segment(model.Prescribed(temperature=20.0), (0.1, 0.0), (0.1, 0.005), region=1)
        case.segments = segments
        assert model.issues(case) == []
        assert model.segment_region(case, 1) == 1

    def test_boundary_kinds_and_defaults(self):
        assert model.Prescribed(20.0).humidity is None
        assert model.Prescribed(20.0, 0.8).humidity == 0.8
        assert model.Convective(20.0, 8.0).humidity == 0.5
        assert repr(model.Adiabatic()) == "Adiabatic()"
        assert build.record_name(model.Adiabatic()) == "Adiabatic"
        assert build.record_name(model.Prescribed(20.0)) == "Prescribed temperature"
        assert build.record_name(model.Prescribed(20.0, 0.8)) == "Prescribed temperature and humidity"
        assert build.record_name(model.Convective(20.0, 8.0)) == "Convective exchange"

    def test_settings_defaults(self):
        physics = model.Physics()
        assert physics.thermal and physics.moisture and physics.liquid_transport
        assert not model.Physics(moisture=False).moisture
        numerics = model.Numerics()
        assert abs(numerics.convergence_tolerance - 1e-5) < TOL
        assert numerics.max_iterations == 25
        assert abs(model.Schedule(3600.0, 24).duration - 86400.0) < TOL

    def test_material_defaults_and_curves(self):
        material = model.Material("m", 5.0, [(0.0, 0.0), (1.0, 50.0)])
        assert material.liquid_transport_curve == []
        assert material.mu_curve == []
        assert material.density == 0.0
        assert material.sorption_curve == [(0.0, 0.0), (1.0, 50.0)]
        assert abs(build.water_content(material, 0.5) - 25.0) < TOL
        assert abs(build.max_water_content(material) - 50.0) < TOL

    def test_cases_compare_by_value(self):
        assert sealed_strip() == sealed_strip()
        changed = sealed_strip()
        changed.initial = model.Initial(temperature=30.0, humidity=0.6)
        assert changed != sealed_strip()

    def test_regions_carry_their_material(self):
        case = sealed_strip()
        assert case.regions[0].material == linear_sorption()
        assert model.region_material(case, 0).name == "linear-sorption"
        assert model.region_material(case, 3) is None


class TestIssues:
    def test_well_formed_case_has_none(self):
        assert model.issues(sealed_strip()) == []

    def test_issues_are_reported_by_line(self):
        case = sealed_strip()
        nameless = linear_sorption()
        nameless.name = ""
        case.regions = [model.Region(nameless, case.regions[0].points)]
        case.schedule = model.Schedule(dtime=3600.0, n_steps=0)
        found = model.issues(case)
        assert len(found) == 2
        assert "no name" in found[0]
        assert "at least one step" in found[1]

    def test_one_name_with_two_definitions_is_an_issue(self):
        case = sealed_strip()
        denser = linear_sorption()
        denser.density = 1200.0
        case.regions = list(case.regions) + [
            model.Region(denser, [(0.1, 0.0), (0.2, 0.0), (0.2, 0.005), (0.1, 0.005)])
        ]
        segments = list(case.segments)
        segments[1] = model.Segment(model.Prescribed(temperature=20.0), (0.1, 0.0), (0.1, 0.005), region=0)
        case.segments = segments
        found = model.issues(case)
        assert len(found) == 1
        assert "'linear-sorption' with different properties" in found[0]

    def test_writing_an_invalid_case_raises(self, tmp_path):
        case = sealed_strip()
        nameless = linear_sorption()
        nameless.name = ""
        case.regions = [model.Region(nameless, case.regions[0].points)]
        with pytest.raises(ValueError, match="no name"):
            build.archive_entries(case)
        target = tmp_path / "invalid.thmz"
        with pytest.raises(ValueError, match="no name"):
            build.archive(case, str(target))
        assert not target.exists()


class TestLibraryRecords:
    def test_library_material_is_a_materials_record(self):
        record = build.library_material(linear_sorption())
        assert isinstance(record, fem.Material)
        assert record.name == "linear-sorption"
        assert record.uuid == build.material_uuid("linear-sorption")
        hygro = record.data.hygro_thermal
        assert abs(hygro.bulk_density - 1000.0) < TOL
        assert [(p.x, p.y) for p in hygro.moisture_storage_function] == [(0.0, 0.0), (1.0, 100.0)]

    def test_convective_record_reads_three_roles(self):
        record = build.boundary_record(model.Convective(20.0, 8.0, 0.5), "probe")
        assert isinstance(record, fem.BoundaryCondition)
        assert record.project_name == "probe"
        assert fem.required_roles(record) == [
            fem.SeriesRole.RelativeHumidity,
            fem.SeriesRole.AirTemperature,
            fem.SeriesRole.ConvectiveCoefficient,
        ]
        assert build.series_values(model.Convective(20.0, 8.0, 0.5)) == [
            ("AirTemperature", 20.0),
            ("ConvectiveCoefficient", 8.0),
            ("RelativeHumidity", 0.5),
        ]
        # Temperature before humidity, as THERM writes a prescribed-state dataset.
        assert build.series_values(model.Prescribed(20.0, 0.8)) == [("PrescribedTemperature", 20.0), ("PrescribedHumidity", 0.8)]
        assert build.series_values(model.Adiabatic()) == []

    def test_model_and_libraries(self):
        case = sealed_strip()
        therm_model = build.model(case)
        assert isinstance(therm_model, fem.ThermModel)
        assert len(therm_model.polygons) == 1
        assert len(therm_model.boundary_conditions) == 4
        options = therm_model.properties.calculation_options
        assert options.simulation_engine == fem.SimulationEngine.HygroThermFEM
        assert options.calculation_mode == fem.CalculationMode.cmTransient
        assert options.modeling_options.exclude_water_liquid_transportation is True
        assert options.modeling_options.exclude_capillary_conduction is False
        assert abs(options.constant_initial_conditions_transient.temperature - 30.0) < TOL
        origin = therm_model.preferences.settings.origin
        assert (origin.x, origin.y) == build.DRAWING_ORIGIN_MM

        libraries = build.libraries(case)
        assert libraries.materials.get_names() == ["linear-sorption"]
        assert sorted(libraries.boundary_conditions.get_names()) == ["Adiabatic", "Prescribed temperature"]
        assert len(libraries.datasets) == 2
        schedule = build.schedule_of(libraries.datasets)
        assert schedule.n_steps == 300
        assert abs(schedule.dtime - 36000.0) < TOL

        datasets = build.boundary_datasets(case)
        assert sorted(datasets) == [1, 3]
        assert fem.time_series_steps(datasets[1]) == 300


class TestArchive:
    def test_entries(self):
        entries = build.archive_entries(sealed_strip())
        assert {"Model.xml", "Materials.xml", "BoundaryConditions.xml", "Gases.xml", "Mesh.xml"} <= set(entries)
        assert sum(name.startswith("time series/") for name in entries) == 2

    def test_written_archive_round_trips(self, tmp_path):
        path = str(tmp_path / "nested" / "strip.thmz")
        assert build.archive(sealed_strip(), path) == path
        assert os.path.exists(path)

        loaded = fem.load_model_from_zip_file(path)
        assert loaded is not None
        assert len(loaded.polygons) == 1
        assert len(loaded.boundary_conditions) == 4

        records = fem.BoundaryConditionsDB()
        records.load_from_zip_file(path)
        assert sorted(records.get_names()) == ["Adiabatic", "Prescribed temperature"]
        assert len(fem.load_datasets_from_zip_file(path)) == 2

        materials = fem.MaterialsDB()
        materials.load_from_string(fem.zip.unzip_files(path, ["Materials.xml"])["Materials.xml"])
        assert materials.get_names() == ["linear-sorption"]

        # Overwrites in place.
        assert build.archive(sealed_strip(), path) == path

    def test_identity_is_stable(self):
        case = sealed_strip()
        assert build.case_uuid(case, "region/0") == build.case_uuid(sealed_strip(), "region/0")
        assert build.case_uuid(case, "region/0") != build.case_uuid(case, "region/1")
        assert build.record_uuid("Convective exchange") == build.boundary_record(model.Convective(20.0, 8.0), "x").uuid
        first = build.archive_entries(case)
        second = build.archive_entries(sealed_strip())
        assert first.keys() == second.keys()

    def test_adiabatic_is_therms_built_in_record(self):
        record = build.boundary_record(model.Adiabatic(), "x")
        assert record.uuid == "61d7bd1c-22c6-4ea0-8720-0696e8c194ad"
        assert record.name == "Adiabatic"
        assert record.protected_ is True
        assert record.project_name is None
        assert build.record_uuid("Adiabatic") == record.uuid
