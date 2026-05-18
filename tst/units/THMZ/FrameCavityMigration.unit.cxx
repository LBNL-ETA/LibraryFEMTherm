#include <gtest/gtest.h>

#include "THMZ/Model/FrameCavityMigration.hxx"
#include "THMZ/Model/THMX.hxx"
#include "Materials/DB.hxx"

namespace
{
    MaterialsLibrary::Material makeSolidMaterial(std::string uuid, std::string name)
    {
        MaterialsLibrary::Material material;
        material.UUID = std::move(uuid);
        material.Name = std::move(name);
        material.data = MaterialsLibrary::Solid{};
        return material;
    }

    MaterialsLibrary::Material makeCavityMaterial(std::string uuid,
                                                  std::string name,
                                                  MaterialsLibrary::CavityStandard standard,
                                                  std::string gas)
    {
        MaterialsLibrary::Material material;
        material.UUID = std::move(uuid);
        material.Name = std::move(name);
        MaterialsLibrary::Cavity cavity;
        cavity.cavityStandard = standard;
        cavity.GasName = std::move(gas);
        material.data = cavity;
        return material;
    }

    MaterialsLibrary::Material makeRadiationEnclosureMaterial(std::string uuid, std::string name)
    {
        MaterialsLibrary::Material material;
        material.UUID = std::move(uuid);
        material.Name = std::move(name);
        material.data = MaterialsLibrary::RadiationEnclosure{};
        return material;
    }

    ThermFile::Polygon makePolygon(std::string uuid, std::string materialUUID)
    {
        ThermFile::Polygon polygon;
        polygon.uuid = std::move(uuid);
        polygon.materialUUID = std::move(materialUUID);
        polygon.polygonType = ThermFile::PolygonType::Material;
        return polygon;
    }

    MaterialsLibrary::DB makeDBWith(std::initializer_list<MaterialsLibrary::Material> materials)
    {
        MaterialsLibrary::DB db;
        for(const auto & material : materials)
        {
            db.add(material);
        }
        return db;
    }
}   // namespace

class TestFrameCavityMigration : public testing::Test
{};

TEST_F(TestFrameCavityMigration, NoLegacyMaterialsIsNoOp)
{
    auto materials = makeDBWith({makeSolidMaterial("solid-1", "Steel")});

    ThermFile::ThermModel model;
    model.polygons.push_back(makePolygon("poly-1", "solid-1"));

    ThermFile::applyFrameCavityMigration(materials, model);

    EXPECT_EQ(materials.getMaterials().size(), 1u);
    EXPECT_EQ(model.polygons[0].polygonType, ThermFile::PolygonType::Material);
    EXPECT_EQ(model.polygons[0].materialUUID, "solid-1");
    EXPECT_FALSE(model.polygons[0].cavity.has_value());

    // Project-level defaults stay at their constructed values when nothing to promote.
    const auto & properties = model.properties.calculationOptions.frameCavityProperties;
    EXPECT_EQ(properties.standard, ThermFile::CavityStandard::ISO15099);
    EXPECT_EQ(properties.defaultGas, "Air");
    EXPECT_FALSE(properties.defaultVentilated);
}

TEST_F(TestFrameCavityMigration, CavityMaterialBecomesPerPolygonCavity)
{
    auto materials = makeDBWith({
      makeSolidMaterial("solid-1", "Steel"),
      makeCavityMaterial("cavity-1", "Frame Cavity NFRC 100",
                         MaterialsLibrary::CavityStandard::ISO15099, "Air"),
    });

    ThermFile::ThermModel model;
    model.polygons.push_back(makePolygon("poly-1", "cavity-1"));
    model.polygons.push_back(makePolygon("poly-2", "solid-1"));

    ThermFile::applyFrameCavityMigration(materials, model);

    // Cavity material is gone from the library; solid stays.
    ASSERT_EQ(materials.getMaterials().size(), 1u);
    EXPECT_EQ(materials.getMaterials()[0].UUID, "solid-1");

    // Cavity polygon: rewritten to FrameCavity with per-polygon CavityData.
    const auto & cavityPolygon = model.polygons[0];
    EXPECT_EQ(cavityPolygon.polygonType, ThermFile::PolygonType::FrameCavity);
    ASSERT_TRUE(cavityPolygon.cavity.has_value());
    EXPECT_EQ(cavityPolygon.cavity->gas, "Air");
    EXPECT_FALSE(cavityPolygon.cavity->ventilated);
    EXPECT_TRUE(cavityPolygon.materialUUID.empty());

    // Solid polygon: untouched.
    EXPECT_EQ(model.polygons[1].polygonType, ThermFile::PolygonType::Material);
    EXPECT_EQ(model.polygons[1].materialUUID, "solid-1");
    EXPECT_FALSE(model.polygons[1].cavity.has_value());

    // Project defaults promoted from the only cavity polygon.
    const auto & properties = model.properties.calculationOptions.frameCavityProperties;
    EXPECT_EQ(properties.standard, ThermFile::CavityStandard::ISO15099);
    EXPECT_EQ(properties.defaultGas, "Air");
    EXPECT_FALSE(properties.defaultVentilated);
}

TEST_F(TestFrameCavityMigration, RadiationEnclosureMaterialBecomesRadiationEnclosurePolygon)
{
    auto materials = makeDBWith({
      makeRadiationEnclosureMaterial("enclosure-1", "Radiation Enclosure"),
    });

    ThermFile::ThermModel model;
    model.polygons.push_back(makePolygon("poly-1", "enclosure-1"));

    ThermFile::applyFrameCavityMigration(materials, model);

    EXPECT_TRUE(materials.getMaterials().empty());

    const auto & polygon = model.polygons[0];
    EXPECT_EQ(polygon.polygonType, ThermFile::PolygonType::RadiationEnclosure);
    EXPECT_TRUE(polygon.materialUUID.empty());
    EXPECT_FALSE(polygon.cavity.has_value());
}

TEST_F(TestFrameCavityMigration, VentilatedVariantStandardIsNormalizedAndVentilationFlagSet)
{
    auto materials = makeDBWith({
      makeCavityMaterial("cavity-vent", "Frame Cavity ISO Ventilated",
                         MaterialsLibrary::CavityStandard::ISO15099Ventilated, "Argon"),
    });

    ThermFile::ThermModel model;
    model.polygons.push_back(makePolygon("poly-1", "cavity-vent"));

    ThermFile::applyFrameCavityMigration(materials, model);

    const auto & cavity = model.polygons[0].cavity;
    ASSERT_TRUE(cavity.has_value());
    EXPECT_EQ(cavity->gas, "Argon");
    EXPECT_TRUE(cavity->ventilated);

    const auto & properties = model.properties.calculationOptions.frameCavityProperties;
    // ISO15099Ventilated -> ISO15099 at the project level (ventilation is on the polygon).
    EXPECT_EQ(properties.standard, ThermFile::CavityStandard::ISO15099);
    EXPECT_EQ(properties.defaultGas, "Argon");
    EXPECT_TRUE(properties.defaultVentilated);
}

TEST_F(TestFrameCavityMigration, NFRCAliasesNormalizeToISO15099)
{
    auto materials = makeDBWith({
      makeCavityMaterial("cavity-nfrc", "Frame Cavity NFRC",
                         MaterialsLibrary::CavityStandard::NFRC, "Air"),
    });

    ThermFile::ThermModel model;
    model.polygons.push_back(makePolygon("poly-1", "cavity-nfrc"));

    ThermFile::applyFrameCavityMigration(materials, model);

    const auto & properties = model.properties.calculationOptions.frameCavityProperties;
    EXPECT_EQ(properties.standard, ThermFile::CavityStandard::ISO15099);
}

TEST_F(TestFrameCavityMigration, MajorityVoteAcrossMultipleCavities)
{
    auto materials = makeDBWith({
      makeCavityMaterial("cavity-iso", "Frame Cavity ISO",
                         MaterialsLibrary::CavityStandard::ISO15099, "Air"),
      makeCavityMaterial("cavity-cen", "Frame Cavity CEN",
                         MaterialsLibrary::CavityStandard::CEN, "Argon"),
    });

    ThermFile::ThermModel model;
    // Two cavity polygons reference ISO/Air, one references CEN/Argon.
    model.polygons.push_back(makePolygon("poly-1", "cavity-iso"));
    model.polygons.push_back(makePolygon("poly-2", "cavity-iso"));
    model.polygons.push_back(makePolygon("poly-3", "cavity-cen"));

    ThermFile::applyFrameCavityMigration(materials, model);

    const auto & properties = model.properties.calculationOptions.frameCavityProperties;
    EXPECT_EQ(properties.standard, ThermFile::CavityStandard::ISO15099);
    EXPECT_EQ(properties.defaultGas, "Air");
}

TEST_F(TestFrameCavityMigration, TiePrefersISO15099AndAirAndFalse)
{
    auto materials = makeDBWith({
      makeCavityMaterial("cavity-iso", "Frame Cavity ISO",
                         MaterialsLibrary::CavityStandard::ISO15099, "Air"),
      makeCavityMaterial("cavity-cen", "Frame Cavity CEN Vent",
                         MaterialsLibrary::CavityStandard::CENVentilated, "Argon"),
    });

    ThermFile::ThermModel model;
    // 1:1 across every dimension (ISO vs CEN, Air vs Argon, false vs true)
    model.polygons.push_back(makePolygon("poly-1", "cavity-iso"));
    model.polygons.push_back(makePolygon("poly-2", "cavity-cen"));

    ThermFile::applyFrameCavityMigration(materials, model);

    const auto & properties = model.properties.calculationOptions.frameCavityProperties;
    EXPECT_EQ(properties.standard, ThermFile::CavityStandard::ISO15099);
    EXPECT_EQ(properties.defaultGas, "Air");
    EXPECT_FALSE(properties.defaultVentilated);
}
