"""Tests for SteadyStateMeshResults bindings: load, round-trip, struct creation."""

import pylibraryfemtherm as fem

TOL = 1e-6


class TestMeshResultsLoadFromTHMZ:

    def test_load_sample_sill(self, sample_sill_path):
        results = fem.load_steady_state_mesh_results_from_zip_file(sample_sill_path)
        assert results is not None
        assert len(results.cases) > 0

    def test_first_case_has_nodes(self, sample_sill_path):
        results = fem.load_steady_state_mesh_results_from_zip_file(sample_sill_path)
        case = results.cases[0]
        assert len(case.nodes) > 0

    def test_node_result_fields(self, sample_sill_path):
        results = fem.load_steady_state_mesh_results_from_zip_file(sample_sill_path)
        node = results.cases[0].nodes[0]
        assert isinstance(node.index, int)
        assert isinstance(node.temperature, float)
        assert isinstance(node.x_flux, float)
        assert isinstance(node.y_flux, float)

    def test_no_results_returns_none(self, sample_sill_no_results_path):
        results = fem.load_steady_state_mesh_results_from_zip_file(
            sample_sill_no_results_path
        )
        assert results is None

    def test_nonexistent_file_returns_none(self):
        result = fem.load_steady_state_mesh_results_from_zip_file("nonexistent.thmz")
        assert result is None


class TestMeshResultsStringRoundTrip:

    def test_save_and_reload(self, sample_sill_path):
        results = fem.load_steady_state_mesh_results_from_zip_file(sample_sill_path)
        assert results is not None

        xml = fem.save_mesh_results_to_string(results)
        assert len(xml) > 0

        reloaded = fem.load_steady_state_mesh_results_from_string(xml)
        assert reloaded is not None
        assert len(reloaded.cases) == len(results.cases)
        assert len(reloaded.cases[0].nodes) == len(results.cases[0].nodes)


class TestMeshResultsStructCreation:

    def test_create_node_results(self):
        node = fem.NodeResults()
        node.index = 5
        node.temperature = 293.15
        node.x_flux = 10.0
        node.y_flux = -5.0
        assert node.index == 5
        assert abs(node.temperature - 293.15) < TOL
        assert abs(node.x_flux - 10.0) < TOL
        assert abs(node.y_flux + 5.0) < TOL

    def test_create_edge_results(self):
        edge = fem.EdgeResults()
        edge.segment_type = fem.SegmentType.Convection
        edge.index = 0
        edge.area = 0.05
        edge.flux = 100.0
        edge.heat_rate = 5.0
        assert edge.segment_type == fem.SegmentType.Convection
        assert abs(edge.area - 0.05) < TOL

    def test_create_tag_nodes(self):
        tag = fem.TagNodes()
        tag.name = "Frame"
        tag.nodes = [1, 2, 3, 4]
        assert tag.name == "Frame"
        assert len(tag.nodes) == 4

    def test_create_mesh_results(self):
        results = fem.MeshResults()
        assert results.version == "1"
        assert len(results.cases) == 0
        assert len(results.tag_nodes) == 0
