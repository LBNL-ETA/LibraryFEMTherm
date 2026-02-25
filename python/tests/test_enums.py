"""Tests for enum bindings — verify all enum types are accessible and have expected values."""

import _femtherm as fem


class TestGeometryEnums:

    def test_direction_values(self):
        assert fem.Direction.Unknown is not None
        assert fem.Direction.Right is not None
        assert fem.Direction.Left is not None
        assert fem.Direction.Up is not None
        assert fem.Direction.Down is not None

    def test_surface_type_values(self):
        assert fem.SurfaceType.Unknown is not None
        assert fem.SurfaceType.BoundaryCondition is not None
        assert fem.SurfaceType.GlazingCavity is not None
        assert fem.SurfaceType.FrameCavity is not None

    def test_polygon_type_values(self):
        assert fem.PolygonType.Material is not None
        assert fem.PolygonType.Glass is not None
        assert fem.PolygonType.Gap is not None
        assert fem.PolygonType.Spacer is not None

    def test_shade_modifier_values(self):
        assert getattr(fem.ShadeModifier, "None") is not None
        assert fem.ShadeModifier.Exterior is not None
        assert fem.ShadeModifier.Interior is not None

    def test_material_side_values(self):
        assert fem.MaterialSide.Front is not None
        assert fem.MaterialSide.Back is not None


class TestPreferencesEnums:

    def test_unit_system(self):
        assert fem.UnitSystem.SI is not None
        assert fem.UnitSystem.IP is not None

    def test_conductivity_units(self):
        assert fem.ConductivityUnits.BtuPerHr_Ft_Fahrenheit is not None

    def test_u_value_basis(self):
        assert fem.UValueBasisType.TotalLength is not None
        assert fem.UValueBasisType.YProjected is not None

    def test_underlay_type(self):
        assert fem.UnderlayType.None_ is not None
        assert fem.UnderlayType.Bitmap is not None
        assert fem.UnderlayType.DXF is not None


class TestPropertiesEnums:

    def test_simulation_engine(self):
        assert fem.SimulationEngine.Conrad is not None
        assert fem.SimulationEngine.HygroThermFEM is not None

    def test_calculation_mode(self):
        assert fem.CalculationMode.cmTransient is not None
        assert fem.CalculationMode.cmSteadyState is not None

    def test_mesher_type(self):
        assert fem.MesherType.QuadTreeMesher is not None
        assert fem.MesherType.Simmetrix_Version_2022 is not None

    def test_model_purpose(self):
        assert fem.ModelPurpose.Window_TransparentFacade is not None
        assert fem.ModelPurpose.OpaqueFacade is not None

    def test_cross_section_types(self):
        assert fem.WindowCrossSectionType.Sill is not None
        assert fem.OpaqueCrossSectionType.SillPlate is not None
        assert fem.OtherCrossSectionType.CommonThermalBridge is not None

    def test_gravity_orientation(self):
        assert fem.GravityOrientation.Down is not None
        assert fem.GravityOrientation.Up is not None


class TestMaterialsEnums:

    def test_material_type(self):
        assert fem.MaterialType.Solid is not None
        assert fem.MaterialType.Cavity is not None
        assert fem.MaterialType.RadiationEnclosure is not None

    def test_material_roughness(self):
        assert fem.MaterialRoughness.VeryRough is not None
        assert fem.MaterialRoughness.VerySmooth is not None

    def test_cavity_standard(self):
        assert fem.CavityStandard.NFRC is not None
        assert fem.CavityStandard.CEN is not None
        assert fem.CavityStandard.ISO15099 is not None


class TestFileFormat:

    def test_file_format(self):
        assert fem.FileFormat.XML is not None
        assert fem.FileFormat.JSON is not None
        assert fem.FileFormat.Unknown is not None


class TestCMAEnums:

    def test_cma_igu_type(self):
        assert fem.CMAIGUType.SingleLayer is not None
        assert fem.CMAIGUType.DoubleLayer is not None

    def test_cma_case(self):
        assert fem.CMACase.None_ is not None
        assert fem.CMACase.Low is not None
        assert fem.CMACase.High is not None
