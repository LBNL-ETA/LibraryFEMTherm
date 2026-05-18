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

    ThermFile::LegacyMaterialsCapture captureWithCavity(std::string uuid,
                                                       std::string standard,
                                                       std::string gas)
    {
        ThermFile::LegacyMaterialsCapture capture;
        capture.cavities[std::move(uuid)] = ThermFile::LegacyCavityEntry{std::move(standard), std::move(gas)};
        return capture;
    }
}   // namespace

class TestFrameCavityMigration : public testing::Test
{};

TEST_F(TestFrameCavityMigration, EmptyCaptureIsNoOp)
{
    MaterialsLibrary::DB materials;
    materials.add(makeSolidMaterial("solid-1", "Steel"));

    ThermFile::ThermModel model;
    model.polygons.push_back(makePolygon("poly-1", "solid-1"));

    ThermFile::LegacyMaterialsCapture empty;
    ThermFile::applyFrameCavityMigration(empty, materials, model);

    EXPECT_EQ(materials.getMaterials().size(), 1u);
    EXPECT_EQ(model.polygons[0].polygonType, ThermFile::PolygonType::Material);
    EXPECT_EQ(model.polygons[0].materialUUID, "solid-1");
    EXPECT_FALSE(model.polygons[0].cavity.has_value());

    const auto & properties = model.properties.calculationOptions.frameCavityProperties;
    EXPECT_EQ(properties.standard, ThermFile::CavityStandard::ISO15099);
    EXPECT_EQ(properties.defaultGas, "Air");
    EXPECT_FALSE(properties.defaultVentilated);
}

TEST_F(TestFrameCavityMigration, CavityCaptureBecomesPerPolygonCavity)
{
    MaterialsLibrary::DB materials;
    materials.add(makeSolidMaterial("solid-1", "Steel"));
    materials.add(makeSolidMaterial("cavity-1", "Frame Cavity NFRC 100"));  // empty Solid stand-in

    ThermFile::ThermModel model;
    model.polygons.push_back(makePolygon("poly-1", "cavity-1"));
    model.polygons.push_back(makePolygon("poly-2", "solid-1"));

    auto capture = captureWithCavity("cavity-1", "ISO15099", "Air");

    ThermFile::applyFrameCavityMigration(capture, materials, model);

    // Stub cavity material gone from DB; solid stays.
    ASSERT_EQ(materials.getMaterials().size(), 1u);
    EXPECT_EQ(materials.getMaterials()[0].UUID, "solid-1");

    const auto & cavityPolygon = model.polygons[0];
    EXPECT_EQ(cavityPolygon.polygonType, ThermFile::PolygonType::FrameCavity);
    ASSERT_TRUE(cavityPolygon.cavity.has_value());
    EXPECT_EQ(cavityPolygon.cavity->gas, "Air");
    EXPECT_FALSE(cavityPolygon.cavity->ventilated);
    EXPECT_TRUE(cavityPolygon.materialUUID.empty());

    EXPECT_EQ(model.polygons[1].polygonType, ThermFile::PolygonType::Material);
    EXPECT_EQ(model.polygons[1].materialUUID, "solid-1");
    EXPECT_FALSE(model.polygons[1].cavity.has_value());

    const auto & properties = model.properties.calculationOptions.frameCavityProperties;
    EXPECT_EQ(properties.standard, ThermFile::CavityStandard::ISO15099);
    EXPECT_EQ(properties.defaultGas, "Air");
    EXPECT_FALSE(properties.defaultVentilated);
}

TEST_F(TestFrameCavityMigration, RadiationEnclosureCaptureBecomesRadiationEnclosurePolygon)
{
    MaterialsLibrary::DB materials;
    materials.add(makeSolidMaterial("enclosure-1", "Radiation Enclosure"));

    ThermFile::ThermModel model;
    model.polygons.push_back(makePolygon("poly-1", "enclosure-1"));

    ThermFile::LegacyMaterialsCapture capture;
    capture.radiationEnclosures.insert("enclosure-1");

    ThermFile::applyFrameCavityMigration(capture, materials, model);

    EXPECT_TRUE(materials.getMaterials().empty());

    const auto & polygon = model.polygons[0];
    EXPECT_EQ(polygon.polygonType, ThermFile::PolygonType::RadiationEnclosure);
    EXPECT_TRUE(polygon.materialUUID.empty());
    EXPECT_FALSE(polygon.cavity.has_value());
}

TEST_F(TestFrameCavityMigration, VentilatedVariantStandardIsNormalizedAndVentilationFlagSet)
{
    MaterialsLibrary::DB materials;
    materials.add(makeSolidMaterial("cavity-vent", "Frame Cavity ISO Ventilated"));

    ThermFile::ThermModel model;
    model.polygons.push_back(makePolygon("poly-1", "cavity-vent"));

    auto capture = captureWithCavity("cavity-vent", "ISO15099Ventilated", "Argon");

    ThermFile::applyFrameCavityMigration(capture, materials, model);

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
    MaterialsLibrary::DB materials;
    materials.add(makeSolidMaterial("cavity-nfrc", "Frame Cavity NFRC"));

    ThermFile::ThermModel model;
    model.polygons.push_back(makePolygon("poly-1", "cavity-nfrc"));

    auto capture = captureWithCavity("cavity-nfrc", "NFRC", "Air");

    ThermFile::applyFrameCavityMigration(capture, materials, model);

    const auto & properties = model.properties.calculationOptions.frameCavityProperties;
    EXPECT_EQ(properties.standard, ThermFile::CavityStandard::ISO15099);
}

TEST_F(TestFrameCavityMigration, MajorityVoteAcrossMultipleCavities)
{
    MaterialsLibrary::DB materials;

    ThermFile::ThermModel model;
    model.polygons.push_back(makePolygon("poly-1", "cavity-iso"));
    model.polygons.push_back(makePolygon("poly-2", "cavity-iso"));
    model.polygons.push_back(makePolygon("poly-3", "cavity-cen"));

    ThermFile::LegacyMaterialsCapture capture;
    capture.cavities["cavity-iso"] = ThermFile::LegacyCavityEntry{"ISO15099", "Air"};
    capture.cavities["cavity-cen"] = ThermFile::LegacyCavityEntry{"CEN", "Argon"};

    ThermFile::applyFrameCavityMigration(capture, materials, model);

    const auto & properties = model.properties.calculationOptions.frameCavityProperties;
    EXPECT_EQ(properties.standard, ThermFile::CavityStandard::ISO15099);
    EXPECT_EQ(properties.defaultGas, "Air");
}

TEST_F(TestFrameCavityMigration, TiePrefersISO15099AndAirAndFalse)
{
    MaterialsLibrary::DB materials;

    ThermFile::ThermModel model;
    model.polygons.push_back(makePolygon("poly-1", "cavity-iso"));
    model.polygons.push_back(makePolygon("poly-2", "cavity-cen"));

    ThermFile::LegacyMaterialsCapture capture;
    capture.cavities["cavity-iso"] = ThermFile::LegacyCavityEntry{"ISO15099", "Air"};
    capture.cavities["cavity-cen"] = ThermFile::LegacyCavityEntry{"CENVentilated", "Argon"};

    ThermFile::applyFrameCavityMigration(capture, materials, model);

    const auto & properties = model.properties.calculationOptions.frameCavityProperties;
    EXPECT_EQ(properties.standard, ThermFile::CavityStandard::ISO15099);
    EXPECT_EQ(properties.defaultGas, "Air");
    EXPECT_FALSE(properties.defaultVentilated);
}

TEST_F(TestFrameCavityMigration, CaptureLegacyMaterialsParsesCavityAndRadiationEnclosure)
{
    const std::string materialsXml = R"(<Materials>
<Version>1</Version>
<Material>
    <UUID>solid-uuid</UUID>
    <Name>Steel</Name>
    <Protected>false</Protected>
    <Color>0x808080</Color>
    <Solid>
        <HygroThermal>
            <ThermalConductivityDry>50</ThermalConductivityDry>
        </HygroThermal>
    </Solid>
</Material>
<Material>
    <UUID>cavity-uuid</UUID>
    <Name>Frame Cavity NFRC 100</Name>
    <Protected>false</Protected>
    <Color>0x00FF00</Color>
    <Cavity>
        <CavityStandard>ISO15099Ventilated</CavityStandard>
        <Gas>Argon</Gas>
        <EmissivitySide1>0.84</EmissivitySide1>
        <EmissivitySide2>0.84</EmissivitySide2>
    </Cavity>
</Material>
<Material>
    <UUID>enclosure-uuid</UUID>
    <Name>Radiation Enclosure</Name>
    <Protected>false</Protected>
    <Color>0xFFFF00</Color>
    <RadiationEnclosure>
        <EmissivityDefault>0.9</EmissivityDefault>
    </RadiationEnclosure>
</Material>
</Materials>
)";

    const auto capture = ThermFile::captureLegacyMaterials(materialsXml);

    ASSERT_EQ(capture.cavities.size(), 1u);
    ASSERT_TRUE(capture.cavities.contains("cavity-uuid"));
    EXPECT_EQ(capture.cavities.at("cavity-uuid").cavityStandard, "ISO15099Ventilated");
    EXPECT_EQ(capture.cavities.at("cavity-uuid").gas, "Argon");

    ASSERT_EQ(capture.radiationEnclosures.size(), 1u);
    EXPECT_TRUE(capture.radiationEnclosures.contains("enclosure-uuid"));
}

TEST_F(TestFrameCavityMigration, CaptureLegacyMaterialsReturnsEmptyForCleanXml)
{
    const std::string materialsXml = R"(<Materials>
<Version>1</Version>
<Material>
    <UUID>solid-only</UUID>
    <Name>Steel</Name>
    <Protected>false</Protected>
    <Color>0x808080</Color>
    <Solid>
        <HygroThermal>
            <ThermalConductivityDry>50</ThermalConductivityDry>
        </HygroThermal>
    </Solid>
</Material>
</Materials>
)";

    const auto capture = ThermFile::captureLegacyMaterials(materialsXml);

    EXPECT_TRUE(capture.cavities.empty());
    EXPECT_TRUE(capture.radiationEnclosures.empty());
}
