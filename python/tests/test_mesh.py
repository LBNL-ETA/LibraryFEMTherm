"""Tests for Mesh bindings: load from THMZ, struct creation, round-trip."""

import pylibraryfemtherm as fem

TOL = 1e-6


class TestMeshLoadFromTHMZ:

    def test_load_sample_sill(self, sample_sill_path):
        mesh = fem.load_mesh_from_zip_file(sample_sill_path)
        assert mesh is not None
        assert len(mesh.cases) > 0

    def test_first_case_has_nodes_and_elements(self, sample_sill_path):
        mesh = fem.load_mesh_from_zip_file(sample_sill_path)
        case = mesh.cases[0]
        assert len(case.nodes) > 0
        assert len(case.elements) > 0

    def test_node_fields(self, sample_sill_path):
        mesh = fem.load_mesh_from_zip_file(sample_sill_path)
        node = mesh.cases[0].nodes[0]
        assert isinstance(node.index, int)
        assert isinstance(node.x, float)
        assert isinstance(node.y, float)

    def test_element_fields(self, sample_sill_path):
        mesh = fem.load_mesh_from_zip_file(sample_sill_path)
        elm = mesh.cases[0].elements[0]
        assert isinstance(elm.index, int)
        assert isinstance(elm.node1, int)
        assert isinstance(elm.node2, int)
        assert isinstance(elm.node3, int)
        assert isinstance(elm.node4, int)

    def test_no_results_file_still_has_mesh(self, sample_sill_no_results_path):
        """sample-sill-no-results.thmz has Mesh.xml but no results XMLs."""
        mesh = fem.load_mesh_from_zip_file(sample_sill_no_results_path)
        assert mesh is not None

    def test_nonexistent_file_returns_none(self):
        result = fem.load_mesh_from_zip_file("nonexistent.thmz")
        assert result is None


class TestMeshStringRoundTrip:

    def test_save_and_reload(self, sample_sill_path):
        mesh = fem.load_mesh_from_zip_file(sample_sill_path)
        assert mesh is not None

        xml = fem.save_mesh_to_string(mesh)
        assert len(xml) > 0

        reloaded = fem.load_mesh_from_string(xml)
        assert reloaded is not None
        assert len(reloaded.cases) == len(mesh.cases)
        assert len(reloaded.cases[0].nodes) == len(mesh.cases[0].nodes)
        assert len(reloaded.cases[0].elements) == len(mesh.cases[0].elements)


class TestMeshStructCreation:

    def test_create_node(self):
        node = fem.MeshNode()
        node.index = 1
        node.x = 0.05
        node.y = -0.1
        assert node.index == 1
        assert abs(node.x - 0.05) < TOL
        assert abs(node.y + 0.1) < TOL

    def test_create_element(self):
        elm = fem.MeshElement()
        elm.index = 0
        elm.node1 = 1
        elm.node2 = 2
        elm.node3 = 3
        elm.node4 = 4
        elm.material_id = 10
        assert elm.material_id == 10

    def test_element_material_id_none(self):
        elm = fem.MeshElement()
        elm.material_id = None
        assert elm.material_id is None

    def test_create_case(self):
        case = fem.MeshCase()
        assert case.model_type is None
        assert case.glazing_case is None
        assert case.spacer_case is None
        assert len(case.nodes) == 0
        assert len(case.elements) == 0

    def test_create_mesh(self):
        mesh = fem.Mesh()
        assert mesh.version == "1"
        assert len(mesh.cases) == 0

    def test_segment_types(self):
        flux = fem.MeshSegmentFlux()
        flux.index = 1
        flux.flux1 = 100.0
        assert abs(flux.flux1 - 100.0) < TOL

        temp = fem.MeshSegmentTemperature()
        temp.temperature = 21.0
        assert abs(temp.temperature - 21.0) < TOL

        conv = fem.MeshSegmentConvection()
        conv.film_coefficient = 8.0
        assert abs(conv.film_coefficient - 8.0) < TOL

        rad = fem.MeshSegmentRadiation()
        rad.film_coefficient = 4.65
        assert abs(rad.film_coefficient - 4.65) < TOL

        enc = fem.MeshSegmentRadiationEnclosure()
        enc.emissivity = 0.9
        assert abs(enc.emissivity - 0.9) < TOL
