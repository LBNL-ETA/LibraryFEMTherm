"""pylibraryfemtherm.steady_state_field and the U-factor reading helpers."""

from __future__ import annotations

import math

import pytest

import pylibraryfemtherm as lft
from pylibraryfemtherm import steady_state_field


def test_the_sills_u_factor_field(sample_sill_path: str) -> None:
    field: steady_state_field.Field = steady_state_field.read_from_zip_file(sample_sill_path)
    assert field.run_type == lft.RunType.UFactor
    assert (len(field.nodes), len(field.elements)) == (425, 417)
    assert len(field.temperature) == len(field.nodes)
    assert not any(math.isnan(value) for value in field.temperature)
    assert all(element.node4 < len(field.nodes) for element in field.elements)
    assert field.flux_magnitude[0] == pytest.approx(math.hypot(field.x_flux[0], field.y_flux[0]))


def test_the_sills_condensation_resistance_field(sample_sill_path: str) -> None:
    field: steady_state_field.Field = steady_state_field.read_from_zip_file(
        sample_sill_path, lft.RunType.CondensationResistance
    )
    assert field.run_type == lft.RunType.CondensationResistance
    assert len(field.nodes) > 0


def test_an_archive_without_results_raises(sample_sill_no_results_path: str) -> None:
    with pytest.raises(ValueError, match="nodal results"):
        steady_state_field.read_from_zip_file(sample_sill_no_results_path)


def test_every_tag_of_the_sill_shows_a_u_factor(sample_sill_path: str) -> None:
    results = lft.load_steady_state_results_from_zip_file(sample_sill_path)
    for tag in results.cases[0].steady_state_u_factors:
        shown = lft.displayed_projection(tag)
        assert shown is not None
        assert shown.length_type == tag.default_display_type
        assert lft.displayed_u_factor(tag) == pytest.approx(shown.u_factor)


def test_a_degenerate_projection_is_not_reportable() -> None:
    projection: lft.Projection = lft.Projection()
    projection.length_type = lft.UValueDimensionType.CustomRotationProjected
    projection.length = 5e-15
    projection.u_factor = 1e16
    assert lft.is_degenerate(projection)
    assert not lft.is_reportable(projection)

    tag: lft.SteadyStateUFactors = lft.SteadyStateUFactors()
    tag.default_display_type = lft.UValueDimensionType.CustomRotationProjected
    tag.projections = [projection]
    assert lft.projection_of(tag, lft.UValueDimensionType.XProjected) is None
    assert lft.displayed_u_factor(tag) is None
