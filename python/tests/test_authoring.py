"""Stating a model in physical terms and writing it as a THMZ.

The C++ unit tests hold the builder to what the archive must contain; these hold the
Python surface: keyword construction, tuples where points and curve samples go,
defaults, value equality, and the two error paths surfacing as ValueError.
"""

import os

import pytest

import pylibraryfemtherm as fem
from pylibraryfemtherm import authoring as au

TOL = 1e-12


def linear_sorption() -> au.Material:
    return au.Material(
        name="linear-sorption",
        diffusion_resistance_factor=10.0,
        sorption_curve=[(0.0, 0.0), (1.0, 100.0)],
        density=1000.0,
        heat_capacity=1000.0,
        thermal_conductivity=1.0,
    )


def sealed_strip() -> au.ModelCase:
    """One rectangle, four segments, prescribed temperature on the two short ends."""
    return au.ModelCase(
        id="sealed_strip_gradient",
        regions=[au.Region("linear-sorption", [(0.0, 0.0), (0.1, 0.0), (0.1, 0.005), (0.0, 0.005)])],
        segments=[
            au.Segment(au.Sealed(), (0.0, 0.0), (0.1, 0.0)),
            au.Segment(au.Prescribed(temperature=20.0), (0.1, 0.0), (0.1, 0.005)),
            au.Segment(au.Sealed(), (0.1, 0.005), (0.0, 0.005)),
            au.Segment(au.Prescribed(temperature=40.0), (0.0, 0.005), (0.0, 0.0)),
        ],
        initial=au.Initial(temperature=30.0, humidity=0.4),
        schedule=au.Schedule(dtime=36000.0, n_steps=300),
        physics=au.Physics(liquid_transport=False, heat_of_evaporation=False),
        materials={"linear-sorption": linear_sorption()},
    )


class TestVocabulary:
    def test_tuples_become_points(self):
        region = au.Region("m", [(0.0, 0.0), (1.0, 0.0), (1.0, 1.0)])
        assert region.points[1] == au.Point(1.0, 0.0)
        assert region.points[1].x == 1.0
        segment = au.Segment(au.Sealed(), (0.0, 0.0), (1.0, 0.0))
        assert segment.end == au.Point(1.0, 0.0)
        assert segment.region == 0

    def test_boundary_kinds_and_defaults(self):
        assert au.Prescribed(20.0).humidity is None
        assert au.Prescribed(20.0, 0.8).humidity == 0.8
        assert au.Convective(20.0, 8.0).humidity == 0.5
        assert repr(au.Sealed()) == "Sealed()"
        assert au.record_name(au.Sealed()) == "Sealed"
        assert au.record_name(au.Prescribed(20.0)) == "Prescribed temperature"
        assert au.record_name(au.Prescribed(20.0, 0.8)) == "Prescribed temperature and humidity"
        assert au.record_name(au.Convective(20.0, 8.0)) == "Convective exchange"

    def test_settings_defaults(self):
        physics = au.Physics()
        assert physics.thermal and physics.moisture and physics.liquid_transport
        assert not au.Physics(moisture=False).moisture
        numerics = au.Numerics()
        assert abs(numerics.convergence_tolerance - 1e-5) < TOL
        assert numerics.max_iterations == 25
        assert abs(au.Schedule(3600.0, 24).duration - 86400.0) < TOL

    def test_material_defaults_and_curves(self):
        material = au.Material("m", 5.0, [(0.0, 0.0), (1.0, 50.0)])
        assert material.liquid_transport_curve == []
        assert material.mu_curve == []
        assert material.density == 0.0
        assert material.sorption_curve == [(0.0, 0.0), (1.0, 50.0)]
        assert abs(au.water_content(material, 0.5) - 25.0) < TOL
        assert abs(au.max_water_content(material) - 50.0) < TOL

    def test_cases_compare_by_value(self):
        assert sealed_strip() == sealed_strip()
        changed = sealed_strip()
        changed.initial = au.Initial(temperature=30.0, humidity=0.6)
        assert changed != sealed_strip()

    def test_materials_are_a_dict(self):
        case = sealed_strip()
        assert list(case.materials) == ["linear-sorption"]
        assert au.material(case, "linear-sorption").name == "linear-sorption"
        assert au.material(case, "brick") is None
        assert au.region_material(case, 0).name == "linear-sorption"
        assert au.region_material(case, 3) is None


class TestIssues:
    def test_well_formed_case_has_none(self):
        assert au.issues(sealed_strip()) == []

    def test_issues_are_reported_by_line(self):
        case = sealed_strip()
        case.regions = [au.Region("brick", case.regions[0].points)]
        case.schedule = au.Schedule(dtime=3600.0, n_steps=0)
        found = au.issues(case)
        assert len(found) == 2
        assert "brick" in found[0]
        assert "at least one step" in found[1]

    def test_writing_an_invalid_case_raises(self, tmp_path):
        case = sealed_strip()
        case.regions = [au.Region("brick", case.regions[0].points)]
        with pytest.raises(ValueError, match="brick"):
            au.archive_entries(case)
        target = tmp_path / "invalid.thmz"
        with pytest.raises(ValueError, match="brick"):
            au.write_archive(case, str(target))
        assert not target.exists()


class TestLibraryRecords:
    def test_library_material_is_a_materials_record(self):
        record = au.library_material(linear_sorption())
        assert isinstance(record, fem.Material)
        assert record.name == "linear-sorption"
        assert record.uuid == au.material_uuid("linear-sorption")
        hygro = record.data.hygro_thermal
        assert abs(hygro.bulk_density - 1000.0) < TOL
        assert [(p.x, p.y) for p in hygro.moisture_storage_function] == [(0.0, 0.0), (1.0, 100.0)]

    def test_convective_record_reads_three_roles(self):
        record = au.boundary_record(au.Convective(20.0, 8.0, 0.5), "probe")
        assert isinstance(record, fem.BoundaryCondition)
        assert record.project_name == "probe"
        assert fem.required_roles(record) == [
            fem.SeriesRole.RelativeHumidity,
            fem.SeriesRole.AirTemperature,
            fem.SeriesRole.ConvectiveCoefficient,
        ]
        assert au.series_values(au.Convective(20.0, 8.0, 0.5)) == [
            ("AirTemperature", 20.0),
            ("ConvectiveCoefficient", 8.0),
            ("RelativeHumidity", 0.5),
        ]
        # Temperature before humidity, as THERM writes a prescribed-state dataset.
        assert au.series_values(au.Prescribed(20.0, 0.8)) == [("PrescribedTemperature", 20.0), ("PrescribedHumidity", 0.8)]
        assert au.series_values(au.Sealed()) == []

    def test_model_and_libraries(self):
        case = sealed_strip()
        model = au.build_model(case)
        assert isinstance(model, fem.ThermModel)
        assert len(model.polygons) == 1
        assert len(model.boundary_conditions) == 4
        options = model.properties.calculation_options
        assert options.simulation_engine == fem.SimulationEngine.HygroThermFEM
        assert options.calculation_mode == fem.CalculationMode.cmTransient
        assert options.modeling_options.exclude_water_liquid_transportation is True
        assert options.modeling_options.exclude_capillary_conduction is False
        assert abs(options.constant_initial_conditions_transient.temperature - 30.0) < TOL
        origin = model.preferences.settings.origin
        assert (origin.x, origin.y) == au.DRAWING_ORIGIN_MM

        libraries = au.build_libraries(case)
        assert libraries.materials.get_names() == ["linear-sorption"]
        assert sorted(libraries.boundary_conditions.get_names()) == ["Prescribed temperature", "Sealed"]
        assert len(libraries.datasets) == 2
        schedule = au.schedule_of(libraries.datasets)
        assert schedule.n_steps == 300
        assert abs(schedule.dtime - 36000.0) < TOL

        datasets = au.boundary_datasets(case)
        assert sorted(datasets) == [1, 3]
        assert fem.time_series_steps(datasets[1]) == 300


class TestArchive:
    def test_entries(self):
        entries = au.archive_entries(sealed_strip())
        assert {"Model.xml", "Materials.xml", "BoundaryConditions.xml", "Gases.xml", "Mesh.xml"} <= set(entries)
        assert sum(name.startswith("time series/") for name in entries) == 2

    def test_written_archive_round_trips(self, tmp_path):
        path = str(tmp_path / "nested" / "strip.thmz")
        assert au.write_archive(sealed_strip(), path) == path
        assert os.path.exists(path)

        model = fem.load_model_from_zip_file(path)
        assert model is not None
        assert len(model.polygons) == 1
        assert len(model.boundary_conditions) == 4

        records = fem.BoundaryConditionsDB()
        records.load_from_zip_file(path)
        assert sorted(records.get_names()) == ["Prescribed temperature", "Sealed"]
        assert len(fem.load_datasets_from_zip_file(path)) == 2

        materials = fem.MaterialsDB()
        materials.load_from_string(fem.zip.unzip_files(path, ["Materials.xml"])["Materials.xml"])
        assert materials.get_names() == ["linear-sorption"]

        # Overwrites in place.
        assert au.write_archive(sealed_strip(), path) == path

    def test_identity_is_stable(self):
        case = sealed_strip()
        assert au.case_uuid(case, "region/0") == au.case_uuid(sealed_strip(), "region/0")
        assert au.case_uuid(case, "region/0") != au.case_uuid(case, "region/1")
        assert au.record_uuid("Sealed") == au.boundary_record(au.Sealed(), "x").uuid
        first = au.archive_entries(case)
        second = au.archive_entries(sealed_strip())
        assert first.keys() == second.keys()
