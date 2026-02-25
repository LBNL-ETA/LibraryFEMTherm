"""Tests for SteadyStateResults bindings: load, round-trip, struct creation."""

import pylibraryfemtherm as fem

TOL = 1e-6


class TestSteadyStateResultsLoadFromTHMZ:

    def test_load_sample_sill(self, sample_sill_path):
        results = fem.load_steady_state_results_from_zip_file(sample_sill_path)
        assert results is not None
        assert len(results.cases) > 0

    def test_first_case_has_u_factors(self, sample_sill_path):
        results = fem.load_steady_state_results_from_zip_file(sample_sill_path)
        case = results.cases[0]
        assert len(case.steady_state_u_factors) > 0

    def test_u_factor_fields(self, sample_sill_path):
        results = fem.load_steady_state_results_from_zip_file(sample_sill_path)
        ufactor = results.cases[0].steady_state_u_factors[0]
        assert isinstance(ufactor.tag, str)
        assert len(ufactor.tag) > 0
        assert isinstance(ufactor.delta_t, float)
        assert isinstance(ufactor.heat_flux, float)
        assert isinstance(ufactor.default_display_type, fem.UValueDimensionType)
        assert len(ufactor.projections) > 0

    def test_projection_fields(self, sample_sill_path):
        results = fem.load_steady_state_results_from_zip_file(sample_sill_path)
        proj = results.cases[0].steady_state_u_factors[0].projections[0]
        assert isinstance(proj.length_type, fem.UValueDimensionType)

    def test_no_results_returns_none(self, sample_sill_no_results_path):
        results = fem.load_steady_state_results_from_zip_file(
            sample_sill_no_results_path
        )
        assert results is None

    def test_nonexistent_file_returns_none(self):
        result = fem.load_steady_state_results_from_zip_file("nonexistent.thmz")
        assert result is None


class TestSteadyStateResultsStringRoundTrip:

    def test_save_and_reload(self, sample_sill_path):
        results = fem.load_steady_state_results_from_zip_file(sample_sill_path)
        assert results is not None

        xml = fem.save_steady_state_results_to_string(results)
        assert len(xml) > 0

        reloaded = fem.load_steady_state_results_from_string(xml)
        assert reloaded is not None
        assert len(reloaded.cases) == len(results.cases)
        assert len(reloaded.cases[0].steady_state_u_factors) == len(
            results.cases[0].steady_state_u_factors
        )


class TestSteadyStateResultsStructCreation:

    def test_create_projection(self):
        proj = fem.Projection()
        proj.length_type = fem.UValueDimensionType.YProjected
        proj.length = 0.15
        proj.u_factor = 2.5
        proj.projection_angle = None
        assert proj.length_type == fem.UValueDimensionType.YProjected
        assert abs(proj.length - 0.15) < TOL
        assert abs(proj.u_factor - 2.5) < TOL
        assert proj.projection_angle is None

    def test_create_u_factors(self):
        ufactors = fem.SteadyStateUFactors()
        ufactors.tag = "Frame"
        ufactors.delta_t = 35.0
        ufactors.heat_flux = 100.0
        ufactors.default_display_type = fem.UValueDimensionType.YProjected
        ufactors.projections = []
        assert ufactors.tag == "Frame"
        assert abs(ufactors.delta_t - 35.0) < TOL

    def test_create_results_case(self):
        case = fem.SteadyStateResultsCase()
        assert case.model_type is None
        assert case.simulation_error is None
        assert case.glazing_case is None
        assert case.spacer_case is None
        assert len(case.steady_state_u_factors) == 0

    def test_create_results(self):
        results = fem.SteadyStateResults()
        assert results.version == "1"
        assert len(results.cases) == 0
