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


def linear_sorption() -> fem.Material:
    """THERM's library record for a material stated from a few numbers."""
    return fem.Material(
        name="linear-sorption",
        diffusion_resistance_factor=10.0,
        sorption_curve=[(0.0, 0.0), (1.0, 100.0)],
        density=1000.0,
        heat_capacity=1000.0,
        thermal_conductivity=1.0,
    )


def hourly(role: fem.SeriesRole, values: list[float], name: str = "weather",
           step_seconds: float = 3600.0) -> fem.TimeSeriesData:
    """A dataset from 1 January: one series under `role`, its UUID its name."""
    return fem.TimeSeriesData(name=name, uuid=name, axis=fem.TimeAxis(step_seconds=step_seconds),
                              series=[fem.Series(role=role, values=values)])


def kinds(found: list[model.Issue]) -> list[model.IssueKind]:
    return [issue.kind for issue in found]


def sealed_strip() -> model.ModelCase:
    """One rectangle, four segments, prescribed temperature on the two short ends."""
    return model.ModelCase(
        id="sealed_strip_gradient",
        regions=[model.Region(linear_sorption(), [(0.0, 0.0), (0.1, 0.0), (0.1, 0.005), (0.0, 0.005)])],
        segments=[
            model.Segment(model.bc.Adiabatic(), (0.0, 0.0), (0.1, 0.0)),
            model.Segment(model.bc.Prescribed(temperature=20.0), (0.1, 0.0), (0.1, 0.005)),
            model.Segment(model.bc.Adiabatic(), (0.1, 0.005), (0.0, 0.005)),
            model.Segment(model.bc.Prescribed(temperature=40.0), (0.0, 0.005), (0.0, 0.0)),
        ],
        initial=model.Initial(temperature=30.0, humidity=0.4),
        schedule=model.Schedule(dtime=36000.0, n_steps=300),
        physics=model.Physics(liquid_transport=False, heat_of_evaporation=False),
    )


def steady_l() -> model.ModelCase:
    """A low block beside a tall one, cold on the low block's outer face and warm on the tall
    block's; steady state, so no clock is stated."""
    return model.ModelCase(
        id="steady_l",
        regions=[
            model.Region(linear_sorption(), [(0.0, 0.0), (0.09, 0.0), (0.09, 0.06), (0.0, 0.06)]),
            model.Region(linear_sorption(), [(0.09, 0.0), (0.17, 0.0), (0.17, 0.09), (0.09, 0.09)]),
        ],
        segments=[
            model.Segment(model.bc.Convective(air_temperature=-18.0, film_coefficient=26.0, humidity=0.5),
                          (0.0, 0.06), (0.0, 0.0)),
            model.Segment(model.bc.Convective(air_temperature=21.0, film_coefficient=8.0, humidity=0.5),
                          (0.17, 0.0), (0.17, 0.09)),
        ],
        physics=model.Physics(moisture=False),
        mode=fem.CalculationMode.cmSteadyState,
        mesh_control=fem.MeshControl(error_energy_norm=8.0, maximum_iterations=4),
    )


class TestSteadyState:
    def test_a_case_is_transient_unless_stated(self):
        assert sealed_strip().mode == fem.CalculationMode.cmTransient
        assert not model.is_steady_state(sealed_strip())

    def test_a_steady_case_needs_no_clock(self):
        assert model.issues(steady_l()) == []
        assert model.is_steady_state(steady_l())

    def test_mesh_control_by_keyword(self):
        control = fem.MeshControl(error_energy_norm=8.0, maximum_iterations=4)
        assert control.run_error_estimator
        assert control == fem.MeshControl(error_energy_norm=8.0, maximum_iterations=4)
        assert repr(control).startswith("MeshControl(run_error_estimator=True, error_energy_norm=8")

    def test_steady_entries(self):
        entries = build.thmz_entries(steady_l())
        assert "BoundaryConditions.xml" in entries
        assert "SteadyStateBC.xml" not in entries
        assert not [name for name in entries if name.startswith("time series/")]
        options = build.model(steady_l()).properties.calculation_options
        assert options.calculation_mode == fem.CalculationMode.cmSteadyState
        assert options.mesh_control.error_energy_norm == pytest.approx(8.0)

    def test_radiation_and_tag_by_keyword(self):
        radiation = fem.BCBlackBodyRadiation(temperature=-18.0, emissivity=1.0)
        assert radiation.temperature.value == pytest.approx(-18.0)
        assert radiation == fem.BCBlackBodyRadiation(temperature=fem.Constant(-18.0), emissivity=1.0)
        cold = model.bc.Convective(air_temperature=-18.0, film_coefficient=26.0, radiation=radiation)
        assert "radiation=BCBlackBodyRadiation(temperature=-18.0, emissivity=1.0" in repr(cold)
        segment = model.Segment(cold, (0.0, 0.06), (0.0, 0.0), u_factor_tag="Frame")
        assert segment.u_factor_tag == "Frame"
        assert repr(segment).endswith("u_factor_tag='Frame')")

    def test_steady_radiation_and_tag_are_written(self):
        case = steady_l()
        segments = list(case.segments)
        segments[0] = model.Segment(
            model.bc.Convective(air_temperature=-18.0, film_coefficient=26.0,
                                radiation=fem.BCBlackBodyRadiation(temperature=-18.0)),
            (0.0, 0.06), (0.0, 0.0))
        segments[1] = model.Segment(segments[1].kind, (0.17, 0.0), (0.17, 0.09), u_factor_tag="Room")
        case.segments = segments
        assert model.issues(case) == []
        written = build.model(case)
        assert [boundary.flux_tag for boundary in written.boundary_conditions][:2] == ["", "Room"]
        assert "BoundaryConditions.xml" in build.thmz_entries(case)

    def test_a_transient_case_writes_fixed_radiation_and_refuses_enclosures(self):
        case = sealed_strip()
        segments = list(case.segments)
        sky: fem.TimeSeriesData = hourly(role=fem.SeriesRole.RadiantTemperature, values=[-10.0] * 300,
                                         name="sky", step_seconds=36000.0)
        segments[0] = model.Segment(
            kind=model.bc.Convective(air_temperature=20.0, film_coefficient=8.0,
                                     radiation=fem.BCFixedCoefficientRadiation(
                                         temperature=fem.SeriesRole.RadiantTemperature, coefficient=4.0)),
            start=(0.0, 0.0), end=(0.1, 0.0), dataset=sky)
        case.segments = segments
        assert model.issues(case) == []
        record = build.segment_records(case)[0]
        assert fem.required_roles(record) == [fem.SeriesRole.RadiantTemperature]

        segments[0] = model.Segment(
            model.bc.Convective(air_temperature=20.0, film_coefficient=8.0, radiation=fem.BCAutomaticEnclosure(emissivity=0.9)),
            (0.0, 0.0), (0.1, 0.0))
        case.segments = segments
        assert model.IssueKind.EnclosureInTransient in kinds(found=model.issues(model_case=case))

    def test_a_steady_case_refuses_a_prescribed_segment(self):
        case = sealed_strip()
        case.mode = fem.CalculationMode.cmSteadyState
        assert model.IssueKind.PrescribedInSteadyState in kinds(found=model.issues(model_case=case))


class TestVocabulary:
    def test_tuples_become_points(self):
        region = model.Region(linear_sorption(), [(0.0, 0.0), (1.0, 0.0), (1.0, 1.0)])
        assert region.points[1] == model.Point(1.0, 0.0)
        assert region.points[1].x == 1.0
        assert region.material.name == "linear-sorption"
        segment = model.Segment(model.bc.Adiabatic(), (0.0, 0.0), (1.0, 0.0))
        assert segment.end == model.Point(1.0, 0.0)
        assert segment.region is None
        assert segment.name is None
        assert model.Segment(model.bc.Adiabatic(), (0.0, 0.0), (1.0, 0.0), region=1).region == 1
        assert model.Segment(model.bc.Adiabatic(), (0.0, 0.0), (1.0, 0.0), name="bottom").name == "bottom"

    def test_points_and_tuples_mix(self):
        mixed = model.Region(linear_sorption(), [model.Point(0.0, 0.0), (1.0, 0.0), model.Point(x=1.0, y=1.0)])
        assert mixed.points == [model.Point(0.0, 0.0), model.Point(1.0, 0.0), model.Point(1.0, 1.0)]
        segment = model.Segment(model.bc.Adiabatic(), model.Point(0.0, 0.0), (1.0, 0.0))
        assert segment.start == model.Point(0.0, 0.0) and segment.end == model.Point(1.0, 0.0)

    def test_signatures_name_both_point_forms(self):
        # The type stubs are generated from these signatures, so a type checker accepts a
        # tuple only if the signature names it.
        for constructor in (model.Region.__init__, model.Segment.__init__):
            assert "tuple[float, float]" in constructor.__doc__

    def test_a_named_segment_is_named_in_its_issue(self):
        case = sealed_strip()
        segments = list(case.segments)
        segments[1] = model.Segment(model.bc.Prescribed(temperature=20.0), (0.1, 0.0), (0.1, 0.02), name="warm end")
        case.segments = segments
        found: list[model.Issue] = model.issues(model_case=case)
        assert kinds(found=found) == [model.IssueKind.OffEveryRegion]
        assert found[0].segment == 1
        assert model.describe(model_case=case, issue=found[0]) == (
            "segment 1 'warm end' from (0.1, 0) to (0.1, 0.02) does not lie along an edge of any region")
        assert repr(found[0]) == "Issue(kind=IssueKind.OffEveryRegion, segment=1)"

    def test_segments_attach_to_the_region_found(self):
        case = sealed_strip()
        assert [model.segment_region(case, index) for index in range(4)] == [0, 0, 0, 0]
        assert model.segment_region(case, 4) is None

    def test_shared_edge_must_name_its_region(self):
        case = sealed_strip()
        case.regions = list(case.regions) + [
            model.Region(linear_sorption(), [(0.1, 0.0), (0.2, 0.0), (0.2, 0.005), (0.1, 0.005)])
        ]
        found: list[model.Issue] = model.issues(model_case=case)
        assert kinds(found=found) == [model.IssueKind.SharedEdge]
        assert (found[0].region, found[0].other_region, found[0].segment) == (0, 1, 1)
        assert model.segment_region(case, 1) is None
        segments = list(case.segments)
        segments[1] = model.Segment(model.bc.Prescribed(temperature=20.0), (0.1, 0.0), (0.1, 0.005), region=1)
        case.segments = segments
        assert model.issues(case) == []
        assert model.segment_region(case, 1) == 1

    def test_boundary_kinds_and_defaults(self):
        assert model.bc.Prescribed(20.0).humidity is None
        assert model.bc.Prescribed(20.0, 0.8).humidity == 0.8
        assert model.bc.Convective(20.0, 8.0).humidity == 0.5
        assert repr(model.bc.Adiabatic()) == "Adiabatic()"
        convective = model.bc.Convective(20.0)
        assert convective.film_coefficient is None
        assert convective.model == fem.BCConvectionModel.Fixed_Convection_Coefficient
        assert convective.wind_speed is None and convective.flux is None and convective.radiation is None
        found: list[model.Issue] = model.issues(model_case=model.ModelCase(
            id="probe",
            regions=[model.Region(linear_sorption(), [(0.0, 0.0), (0.1, 0.0), (0.1, 0.01), (0.0, 0.01)])],
            segments=[model.Segment(convective, (0.1, 0.0), (0.1, 0.01))],
            schedule=model.Schedule(dtime=3600.0, n_steps=1)))
        assert any(issue.kind == model.IssueKind.ConvectionInputMissing
                   and issue.input == model.ConvectionInput.FilmCoefficient for issue in found)

    def test_reprs(self):
        region = model.Region(linear_sorption(), [(0.0, 0.0), (0.2, 0.0), (0.2, 3.0)])
        assert repr(region) == "Region(material='linear-sorption', points=[Point(0, 0), Point(0.2, 0), Point(0.2, 3)])"
        segment = model.Segment(model.bc.Prescribed(temperature=20.0), (0.0, 3.0), (0.0, 0.0))
        assert repr(segment) == "Segment(kind=Prescribed(temperature=20.0), start=Point(0, 3), end=Point(0, 0))"
        weather = model.bc.Convective(air_temperature=fem.SeriesRole.AirTemperature, film_coefficient=8.0,
                                      model=fem.BCConvectionModel.TARP)
        assert repr(weather) == (
            "Convective(air_temperature=SeriesRole.AirTemperature, film_coefficient=8.0, humidity=0.5, "
            "model=BCConvectionModel.TARP)")
        # A nested record prints its inputs the same way: as they would be written.
        sky = model.bc.Convective(air_temperature=20.0, film_coefficient=8.0,
                                  radiation=fem.BCFixedCoefficientRadiation(temperature=fem.SeriesRole.RadiantTemperature,
                                                                            coefficient=4.5))
        assert repr(sky).endswith(
            "radiation=BCFixedCoefficientRadiation(temperature=SeriesRole.RadiantTemperature, coefficient=4.5))")
        bound = model.Segment(kind=weather, start=(0.0, 0.0), end=(0.2, 0.0),
                              dataset=hourly(role=fem.SeriesRole.AirTemperature, values=[1.0, 2.0]))
        assert repr(bound).endswith("dataset=TimeSeriesData(name='weather', 1 Jan 00:00, step 1 h, 2 rows))")
        named = model.Segment(model.bc.Adiabatic(), (0.0, 0.0), (0.2, 0.0), region=1, color="0x1E5A96", name="bottom")
        assert repr(named) == (
            "Segment(kind=Adiabatic(), start=Point(0, 0), end=Point(0.2, 0), region=1, color='0x1E5A96', name='bottom')"
        )
        assert repr(model.Initial(temperature=20.0, humidity=0.5)) == "Initial(temperature=20, humidity=0.5)"
        assert repr(model.Schedule(dtime=3600.0, n_steps=24)) == "Schedule(dtime=3600.0, n_steps=24)"
        assert repr(model.Physics(moisture=False)) == (
            "Physics(thermal=True, moisture=False, liquid_transport=True, heat_of_evaporation=True, "
            "capillary_conduction=True, vapor_diffusion_conduction=True, latent_heat_of_fusion=True, "
            "conductivity_dependent=True)"
        )
        assert repr(model.Numerics()) == "Numerics(convergence_tolerance=1e-05, relaxation=1, max_iterations=25)"

    def test_settings_defaults(self):
        physics = model.Physics()
        assert physics.thermal and physics.moisture and physics.liquid_transport
        assert not model.Physics(moisture=False).moisture
        numerics = model.Numerics()
        assert abs(numerics.convergence_tolerance - 1e-5) < TOL
        assert numerics.max_iterations == 25
        assert abs(model.Schedule(3600.0, 24).duration - 86400.0) < TOL

    def test_material_defaults_and_curves(self):
        record = fem.Material("m", 5.0, [(0.0, 0.0), (1.0, 50.0)])
        assert isinstance(record, fem.Material)
        hygro = record.data.hygro_thermal
        assert hygro.bulk_density == 0.0
        assert abs(hygro.water_vapor_diffusion_resistance_factor - 5.0) < TOL
        assert [(p.x, p.y) for p in hygro.moisture_storage_function] == [(0.0, 0.0), (1.0, 50.0)]
        assert [(p.x, p.y) for p in hygro.liquid_transportation_coefficient_suction] == [(0.0, 0.0)]
        assert hygro.water_vapor_diffusion_resistance_factor_moisture_dependent is None

    def test_cases_compare_by_value(self):
        assert sealed_strip() == sealed_strip()
        changed = sealed_strip()
        changed.initial = model.Initial(temperature=30.0, humidity=0.6)
        assert changed != sealed_strip()

    def test_regions_carry_their_material(self):
        case = sealed_strip()
        assert case.regions[0].material.uuid == linear_sorption().uuid
        assert model.region_material(case, 0).name == "linear-sorption"
        assert model.region_material(case, 3) is None


class TestColours:
    def test_kind_colour_unless_stated(self):
        assert model.kind_color(model.bc.Adiabatic()) == "0x000000"
        segment = model.Segment(model.bc.Convective(20.0, 8.0), (0.0, 0.0), (0.1, 0.0))
        assert segment.color is None
        assert model.segment_color(segment) == model.kind_color(model.bc.Convective(20.0, 8.0))
        coloured = model.Segment(model.bc.Convective(20.0, 8.0), (0.0, 0.0), (0.1, 0.0), color="0xE07A1F")
        assert model.segment_color(coloured) == "0xE07A1F"

    def test_material_colour_unless_stated(self):
        assert linear_sorption().color == fem.material_color("linear-sorption")
        stated = fem.Material("m", 5.0, [(0.0, 0.0), (1.0, 50.0)], color="0xB03A2E")
        assert stated.color == "0xB03A2E"

    def test_colours_reach_the_file(self):
        case = sealed_strip()
        segments = list(case.segments)
        segments[1] = model.Segment(model.bc.Prescribed(temperature=20.0), (0.1, 0.0), (0.1, 0.005), color="0xE07A1F")
        case.segments = segments
        therm_model = build.model(case)
        assert therm_model.boundary_conditions[1].color == "0xE07A1F"
        assert therm_model.boundary_conditions[0].color == "0x000000"
        records = build.libraries(case).boundary_conditions
        assert records.get_by_name("Adiabatic").color == "0x000000"


class TestOutline:
    def test_every_face_stated_leaves_no_gap(self):
        assert model.outline_gaps(sealed_strip()) == []
        assert model.completed(sealed_strip()) == sealed_strip()

    def test_unstated_faces_become_adiabatic(self):
        case = sealed_strip()
        case.segments = [case.segments[1], case.segments[3]]   # the two prescribed ends
        gaps = model.outline_gaps(case)
        assert [type(gap.kind).__name__ for gap in gaps] == ["Adiabatic", "Adiabatic"]
        assert gaps[0].start == model.Point(0.0, 0.0) and gaps[0].end == model.Point(0.1, 0.0)
        assert gaps[0].region == 0
        full = model.completed(case)
        assert len(full.segments) == 4
        assert full.segments[:2] == case.segments
        assert model.outline_gaps(full) == []

        therm_model = build.model(case)
        assert [boundary.name for boundary in therm_model.boundary_conditions] == [
            "segment 1", "segment 2", "Adiabatic", "Adiabatic"
        ]


class TestIssues:
    def test_well_formed_case_has_none(self):
        assert model.issues(sealed_strip()) == []

    def test_issues_are_reported_by_line(self):
        case = sealed_strip()
        nameless = linear_sorption()
        nameless.name = ""
        case.regions = [model.Region(nameless, case.regions[0].points)]
        case.schedule = model.Schedule(dtime=3600.0, n_steps=0)
        found: list[model.Issue] = model.issues(model_case=case)
        assert kinds(found=found) == [model.IssueKind.UnnamedMaterial, model.IssueKind.NoSteps]
        assert [model.describe(model_case=case, issue=issue) for issue in found] == [
            "region 0 has a material with no name", "the schedule needs at least one step"]

    def test_one_name_with_two_records_is_an_issue(self):
        case = sealed_strip()
        another = linear_sorption()
        another.uuid = "another-record"
        case.regions = list(case.regions) + [
            model.Region(another, [(0.1, 0.0), (0.2, 0.0), (0.2, 0.005), (0.1, 0.005)])
        ]
        segments = list(case.segments)
        segments[1] = model.Segment(model.bc.Prescribed(temperature=20.0), (0.1, 0.0), (0.1, 0.005), region=0)
        case.segments = segments
        found: list[model.Issue] = model.issues(model_case=case)
        assert kinds(found=found) == [model.IssueKind.MaterialNameClash]
        assert "two different materials both named 'linear-sorption'" in model.describe(model_case=case, issue=found[0])

    def test_writing_an_invalid_case_raises(self, tmp_path):
        case = sealed_strip()
        nameless = linear_sorption()
        nameless.name = ""
        case.regions = [model.Region(nameless, case.regions[0].points)]
        with pytest.raises(ValueError, match="no name") as raised:
            build.thmz_entries(case)
        # The issues ride on the error, for a caller that wants them rather than the text.
        assert kinds(found=raised.value.issues) == [model.IssueKind.UnnamedMaterial]

        # A caller words them its own way, and leaves the kinds it does not reword to LibraryFEMTherm.
        def own_wording(issue: model.Issue) -> str:
            if issue.kind == model.IssueKind.UnnamedMaterial:
                return f"Polygon {issue.region + 1} needs a material name."
            return model.describe(model_case=case, issue=issue)

        assert [own_wording(issue=issue) for issue in raised.value.issues] == ["Polygon 1 needs a material name."]
        target = tmp_path / "invalid.thmz"
        with pytest.raises(ValueError, match="no name"):
            build.thmz(case, str(target))
        assert not target.exists()


class TestLibraryRecords:
    def test_stated_material_is_a_materials_record(self):
        record = linear_sorption()
        assert isinstance(record, fem.Material)
        assert record.name == "linear-sorption"
        assert record.uuid == fem.material_uuid("linear-sorption")
        hygro = record.data.hygro_thermal
        assert abs(hygro.bulk_density - 1000.0) < TOL
        assert [(p.x, p.y) for p in hygro.moisture_storage_function] == [(0.0, 0.0), (1.0, 100.0)]

    def test_constants_reach_the_record_as_constants(self):
        data = model.record_data(boundary=model.bc.Convective(air_temperature=20.0, film_coefficient=8.0, humidity=0.5))
        assert isinstance(data, fem.SurfaceExchange)
        assert data.convection.air_temperature == fem.Constant(20.0)
        assert data.convection.film_coefficient == fem.Constant(8.0)
        assert data.relative_humidity == fem.Constant(0.5)
        record = fem.BoundaryCondition()
        record.data = data
        assert fem.is_steady_capable(boundary_condition=record)
        assert isinstance(model.record_data(boundary=model.bc.Adiabatic()), fem.NoExchange)

    def test_roles_reach_the_record_as_roles(self):
        data = model.record_data(boundary=model.bc.Convective(air_temperature=fem.SeriesRole.AirTemperature,
                                                              film_coefficient=8.0))
        assert data.convection.air_temperature == fem.FromTimeSeries(fem.SeriesRole.AirTemperature)
        assert data.convection.film_coefficient == fem.Constant(8.0)
        prescribed = model.record_data(boundary=model.bc.Prescribed(temperature=fem.SeriesRole.PrescribedTemperature,
                                                                    humidity=0.8))
        assert prescribed.temperature == fem.FromTimeSeries(fem.SeriesRole.PrescribedTemperature)
        assert prescribed.relative_humidity == fem.Constant(0.8)
        record = fem.BoundaryCondition()
        record.data = prescribed
        assert fem.required_roles(boundary_condition=record) == [fem.SeriesRole.PrescribedTemperature]

    def test_an_input_is_a_number_or_a_role(self):
        convective = model.bc.Convective(air_temperature=fem.SeriesRole.AirTemperature, film_coefficient=8.0)
        assert convective.air_temperature == fem.FromTimeSeries(fem.SeriesRole.AirTemperature)
        assert fem.is_constant(source=convective.humidity)
        assert fem.is_from_time_series(source=convective.air_temperature)
        # Values belong in a dataset the segment binds, not in the condition.
        with pytest.raises(TypeError):
            model.bc.Convective(air_temperature=[1.0, 2.0, 3.0], film_coefficient=8.0)

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
        assert sorted(libraries.boundary_conditions.get_names()) == ["Adiabatic", "segment 2", "segment 4"]
        # Constants everywhere: the clock dataset alone.
        assert len(libraries.datasets) == 1
        schedule = build.schedule_of(libraries.datasets)
        assert schedule.n_steps == 300
        assert abs(schedule.dtime - 36000.0) < TOL

        datasets = build.boundary_datasets(case)
        assert sorted(datasets) == [0]
        assert fem.time_series_steps(datasets[0]) == 300
        bindings = build.segment_bindings(case)
        assert [bound.dataset_uuid is not None for bound in bindings] == [True, False, False, False]
        assert bindings[1].record_uuid == libraries.boundary_conditions.get_by_name("segment 2").uuid

    def test_segments_binding_the_same_weather_share_a_record_and_a_dataset(self):
        outdoor_values: list[float] = [-5.0 + 0.5 * hour for hour in range(48)]
        outdoor: fem.TimeSeriesData = hourly(role=fem.SeriesRole.AirTemperature, values=outdoor_values)
        outside = model.bc.Convective(air_temperature=fem.SeriesRole.AirTemperature, film_coefficient=25.0, humidity=0.8)
        case = model.ModelCase(
            id="weather_wall",
            regions=[model.Region(material=linear_sorption(), points=[(0.0, 0.0), (0.3, 0.0), (0.3, 0.3), (0.0, 0.3)])],
            segments=[
                model.Segment(kind=outside, start=(0.0, 0.3), end=(0.0, 0.0), name="outside", dataset=outdoor),
                model.Segment(kind=model.bc.Convective(air_temperature=20.0, film_coefficient=8.0),
                              start=(0.3, 0.0), end=(0.3, 0.3), name="room side"),
                model.Segment(kind=outside, start=(0.3, 0.3), end=(0.0, 0.3), name="top", dataset=outdoor),
            ],
        )
        assert model.issues(case) == []
        records = build.segment_records(case)
        assert [record.name for record in records] == ["outside", "room side", "Adiabatic"]
        libraries = build.libraries(case)
        assert len(libraries.datasets) == 1
        dataset = libraries.datasets[0]
        assert (dataset.uuid, dataset.name) == (outdoor.uuid, outdoor.name)   # written as given
        assert fem.values_for_role(dataset, fem.SeriesRole.AirTemperature) == outdoor_values
        assert build.schedule_of(libraries.datasets) == model.Schedule(dtime=3600.0, n_steps=48)
        bindings = build.segment_bindings(case)
        assert bindings[0].dataset_uuid == dataset.uuid == bindings[2].dataset_uuid
        assert bindings[1].dataset_uuid is None
        assert bindings[0].record_uuid == bindings[2].record_uuid != bindings[1].record_uuid

    def test_a_role_needs_a_dataset_that_provides_it(self):
        case = sealed_strip()
        segments = list(case.segments)
        warm_end = model.bc.Prescribed(temperature=fem.SeriesRole.PrescribedTemperature)
        segments[1] = model.Segment(kind=warm_end, start=(0.1, 0.0), end=(0.1, 0.005))
        case.segments = segments
        found: list[model.Issue] = model.issues(model_case=case)
        assert kinds(found=found) == [model.IssueKind.RoleWithoutDataset]
        assert (found[0].segment, found[0].roles) == (1, [fem.SeriesRole.PrescribedTemperature])

        segments[1].dataset = hourly(role=fem.SeriesRole.RelativeHumidity, values=[0.5] * 300, step_seconds=36000.0)
        case.segments = segments
        assert kinds(found=model.issues(model_case=case)) == [model.IssueKind.RoleNotProvided]

        # Ten steps against a stated schedule of 300: the case says so; left unstated, the
        # schedule is the dataset's.
        segments[1].dataset = hourly(role=fem.SeriesRole.PrescribedTemperature, values=[20.0] * 10,
                                     step_seconds=36000.0)
        case.segments = segments
        assert kinds(found=model.issues(model_case=case)) == [model.IssueKind.ScheduleDisagrees]
        case.schedule = model.Schedule(step_lengths=[])
        assert model.issues(case) == []

    def test_the_schedule_follows_a_dataset(self):
        weather = fem.TimeSeriesData(
            name="january",
            axis=fem.TimeAxis(month=1, day=1, hour=0, minute=0, step_seconds=1800.0),
            series=[fem.Series(fem.SeriesRole.AirTemperature, [0.0] * 48)],
        )
        schedule = model.schedule_for(weather)
        assert schedule == model.Schedule(dtime=1800.0, n_steps=48)
        assert model.axis_of(schedule) == weather.axis
        shifted = model.Schedule(dtime=3600.0, n_steps=24, start=model.Start(month=2, day=10, hour=6))
        assert repr(shifted) == "Schedule(dtime=3600.0, n_steps=24, start=Start(month=2, day=10, hour=6, minute=0))"
        assert model.axis_of(shifted) == fem.TimeAxis(month=2, day=10, hour=6, minute=0, step_seconds=3600.0)

    def test_an_uneven_schedule_states_every_step(self):
        uneven = model.Schedule(step_lengths=[900.0, 1800.0], start=model.Start(month=2))
        assert uneven.n_steps == 2
        assert abs(uneven.duration - 2700.0) < TOL
        with pytest.raises(ValueError):
            uneven.dtime
        assert repr(uneven) == "Schedule(step_lengths=[900.0, 1800.0], start=Start(month=2, day=1, hour=0, minute=0))"
        assert model.axis_of(uneven) == fem.TimeAxis(month=2, step_seconds=[900.0, 1800.0])
        weather = fem.TimeSeriesData(axis=model.axis_of(uneven),
                                     series=[fem.Series(fem.SeriesRole.AirTemperature, [0.0, 1.0])])
        assert model.schedule_for(weather) == uneven
        assert model.Schedule(step_lengths=[3600.0] * 3) == model.Schedule(dtime=3600.0, n_steps=3)


class TestArchive:
    def test_entries(self):
        entries = build.thmz_entries(sealed_strip())
        assert {"Model.xml", "Materials.xml", "BoundaryConditions.xml", "Gases.xml", "Mesh.xml"} <= set(entries)
        assert sum(name.startswith("time series/") for name in entries) == 1

    def test_written_archive_round_trips(self, tmp_path):
        path = str(tmp_path / "nested" / "strip.thmz")
        assert build.thmz(sealed_strip(), path) == path
        assert os.path.exists(path)

        loaded = fem.load_model(path)
        assert loaded is not None
        assert len(loaded.polygons) == 1
        assert len(loaded.boundary_conditions) == 4

        records = fem.BoundaryConditionsDB()
        records.load(path)
        assert sorted(records.get_names()) == ["Adiabatic", "segment 2", "segment 4"]
        assert len(fem.load_datasets(path)) == 1

        materials = fem.MaterialsDB()
        materials.load_from_string(fem.thmz.read_entries(path, ["Materials.xml"])["Materials.xml"])
        assert materials.get_names() == ["linear-sorption"]

        # Overwrites in place.
        assert build.thmz(sealed_strip(), path) == path

    def test_identity_is_stable(self):
        case = sealed_strip()
        assert build.case_uuid(case, "region/0") == build.case_uuid(sealed_strip(), "region/0")
        assert build.case_uuid(case, "region/0") != build.case_uuid(case, "region/1")
        assert [record.uuid for record in build.segment_records(case)] == [
            record.uuid for record in build.segment_records(sealed_strip())
        ]
        first = build.thmz_entries(case)
        second = build.thmz_entries(sealed_strip())
        assert first.keys() == second.keys()

    def test_adiabatic_is_therms_built_in_record(self):
        record = next(record for record in build.segment_records(sealed_strip()) if record.name == "Adiabatic")
        assert record.uuid == "61d7bd1c-22c6-4ea0-8720-0696e8c194ad" == build.ADIABATIC_RECORD_UUID
        assert record.protected_ is True
        assert record.project_name is None
        assert isinstance(record.data, fem.NoExchange)
