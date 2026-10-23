"""Resolving a model's boundary segments against the archive's libraries: the record and the
dataset each segment refers to, and the pairing check on the pair.

The C++ unit tests hold the lookup rules; these hold them to what the writer produces, segment
by segment, and cover the Python surface: None when nothing is bound.
"""

import pylibraryfemtherm as fem
from pylibraryfemtherm import build, model


def linear_sorption() -> fem.Material:
    return fem.Material(
        name="linear-sorption",
        diffusion_resistance_factor=10.0,
        sorption_curve=[(0.0, 0.0), (1.0, 100.0)],
        density=1000.0,
        heat_capacity=1000.0,
        thermal_conductivity=1.0,
    )


def weather_strip() -> model.ModelCase:
    """One rectangle: a series of air temperatures on the left face, a constant room on the
    right, the rest left for the writer to fill adiabatic."""
    return model.ModelCase(
        id="weather_strip",
        regions=[model.Region(material=linear_sorption(), points=[(0.0, 0.0), (0.1, 0.0), (0.1, 0.05), (0.0, 0.05)])],
        segments=[
            model.Segment(kind=model.bc.Convective(air_temperature=[-5.0, -3.0, 0.0], film_coefficient=25.0),
                          start=(0.0, 0.05), end=(0.0, 0.0), name="outside"),
            model.Segment(kind=model.bc.Convective(air_temperature=20.0, film_coefficient=8.0),
                          start=(0.1, 0.0), end=(0.1, 0.05), name="room side"),
        ],
        schedule=model.Schedule(dtime=3600.0, n_steps=3),
        physics=model.Physics(moisture=False),
    )


class TestAgainstTheWriter:
    def test_every_boundary_resolves_to_what_the_writer_bound(self):
        case: model.ModelCase = weather_strip()
        therm_model: fem.ThermModel = build.model(model_case=case)
        libraries: build.Libraries = build.libraries(model_case=case)
        bindings: list[build.Binding] = build.segment_bindings(model_case=case)

        assert len(therm_model.boundary_conditions) == len(bindings)
        for boundary, binding in zip(therm_model.boundary_conditions, bindings):
            record: fem.BoundaryCondition | None = fem.boundary_record(library=libraries.boundary_conditions,
                                                                       boundary=boundary)
            dataset: fem.TimeSeriesData | None = fem.boundary_dataset(datasets=libraries.datasets, boundary=boundary)
            assert record is not None
            assert record.uuid == binding.record_uuid
            assert (dataset.uuid if dataset is not None else None) == binding.dataset_uuid

    def test_the_series_face_reads_its_dataset_and_misses_nothing(self):
        case: model.ModelCase = weather_strip()
        libraries: build.Libraries = build.libraries(model_case=case)
        outside: fem.Boundary = build.model(model_case=case).boundary_conditions[0]

        record: fem.BoundaryCondition | None = fem.boundary_record(library=libraries.boundary_conditions,
                                                                   boundary=outside)
        dataset: fem.TimeSeriesData | None = fem.boundary_dataset(datasets=libraries.datasets, boundary=outside)
        assert record is not None
        assert dataset is not None
        assert fem.required_roles(boundary_condition=record) == [fem.SeriesRole.AirTemperature]
        assert fem.missing_roles(boundary_condition=record, dataset=dataset) == []


class TestNothingBound:
    def test_a_record_is_found_by_name_without_a_uuid(self):
        library: fem.BoundaryConditionsDB = fem.BoundaryConditionsDB()
        record: fem.BoundaryCondition = fem.BoundaryCondition()
        record.uuid = "uuid-outside"
        record.name = "outside"
        library.add(condition=record)
        boundary: fem.Boundary = fem.Boundary()
        boundary.name = "outside"

        found: fem.BoundaryCondition | None = fem.boundary_record(library=library, boundary=boundary)
        assert found is not None
        assert found.uuid == "uuid-outside"

    def test_none_when_nothing_matches(self):
        boundary: fem.Boundary = fem.Boundary()
        boundary.name = "attic"
        boundary.bc_uuid = "uuid-gone"

        assert fem.boundary_record(library=fem.BoundaryConditionsDB(), boundary=boundary) is None
        assert fem.boundary_dataset(datasets=[], boundary=boundary) is None
