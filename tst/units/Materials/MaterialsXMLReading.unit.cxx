#include <filesystem>
#include <gtest/gtest.h>

#include "Materials/Materials.hxx"
#include "Materials/DB.hxx"
#include "LibraryUtilities/FileManipulation.hxx"

#include "HelperUtility.hxx"
#include "material1.hxx"

class TestMaterialsXMLReading : public testing::Test {
private:
    const std::string fileName{"TestRead.xm"};
protected:

    void SetUp() override {
        std::filesystem::remove(fileName);
    }

    void TearDown() override {
        std::filesystem::remove(fileName);
    }

public:
    [[nodiscard]] std::string getFileName() const {
        return fileName;
    }
};

TEST_F(TestMaterialsXMLReading, ReadMaterial1ByUUID) {
    const std::string fileContent{TestMaterial::testDatabase()};

    File::createFileFromString(getFileName(), fileContent);

    MaterialsLibrary::DB materialDB{getFileName()};
    auto aMaterial{materialDB.getMaterialByUUID("26fc05d4-a7aa-43a1-b473-c984caefea5b")};

    EXPECT_EQ(aMaterial.has_value(), true);

    auto material{aMaterial.value()};

    const std::string correctName{"Test Material 1"};
    EXPECT_EQ(material.Name, correctName);

    EXPECT_EQ(true, material.Protected);

    ASSERT_TRUE(MaterialsLibrary::isSolid(material));

    const auto solid{MaterialsLibrary::getSolid(material)};

    ASSERT_TRUE(solid->hygroThermal.has_value());

    EXPECT_EQ(solid->hygroThermal->DefaultThickness.has_value(), false);

    const double bulkDensityCorrect{1330};
    EXPECT_EQ(solid->hygroThermal->BulkDensity.has_value(), true);
    EXPECT_NEAR(solid->hygroThermal->BulkDensity.value(), bulkDensityCorrect, 1e-6);

    const double porosityCorrect{0.5};
    EXPECT_EQ(solid->hygroThermal->Porosity.has_value(), true);
    EXPECT_NEAR(solid->hygroThermal->Porosity.value(), porosityCorrect, 1e-6);

    const double specificHeatCapacityDryCorrect{850};
    EXPECT_EQ(solid->hygroThermal->SpecificHeatCapacityDry.has_value(), true);
    EXPECT_NEAR(solid->hygroThermal->SpecificHeatCapacityDry.value(), specificHeatCapacityDryCorrect, 1e-6);

    ASSERT_EQ(true, solid->hygroThermal->Roughness.has_value());
    MaterialsLibrary::MaterialRoughness correctRoughness{MaterialsLibrary::MaterialRoughness::MediumRough};
    EXPECT_EQ(correctRoughness, solid->hygroThermal->Roughness.value());
}

TEST_F(TestMaterialsXMLReading, ReadShade1ByUUID) {
    static const auto tolerance{1e-6};
    const std::string fileContent{TestMaterial::testDatabase()};

    File::createFileFromString(getFileName(), fileContent);

    MaterialsLibrary::DB materialDB{getFileName()};
    auto aMaterial{materialDB.getMaterialByUUID("08eeb0cc-bb61-4a45-8746-27d450868de6")};

    EXPECT_EQ(aMaterial.has_value(), true);

    auto material{aMaterial.value()};

    const std::string correctName{"Test Shade"};
    EXPECT_EQ(material.Name, correctName);

    EXPECT_EQ(true, material.Protected);

    ASSERT_TRUE(MaterialsLibrary::isSolid(material));

    const auto solid{MaterialsLibrary::getSolid(material)};

    ASSERT_TRUE(solid->hygroThermal->ThermalConductivityDry.has_value());
    EXPECT_NEAR(1.2, solid->hygroThermal->ThermalConductivityDry.value(), tolerance);

    ASSERT_TRUE(solid->optical.has_value());
    ASSERT_TRUE(solid->optical->integrated.has_value());
    auto & IR{solid->optical->integrated->IR};

    ASSERT_TRUE(IR.Transmittance.has_value());
    EXPECT_NEAR(0.0, IR.Transmittance.value(), tolerance);
    ASSERT_TRUE(IR.EmissivityFront.has_value());
    EXPECT_NEAR(0.9, IR.EmissivityFront.value(), tolerance);
    ASSERT_TRUE(IR.EmissivityBack.has_value());
    EXPECT_NEAR(0.9, IR.EmissivityBack.value(), tolerance);

    ASSERT_TRUE(solid->optical.has_value());
    ASSERT_TRUE(solid->optical->integrated->Solar.has_value());
    auto & solar{solid->optical->integrated->Solar.value()};
    EXPECT_NEAR(0.1, solar.Direct.Front.Transmittance, tolerance);
    EXPECT_NEAR(0.2, solar.Direct.Back.Transmittance, tolerance);
    EXPECT_NEAR(0.3, solar.Direct.Front.Reflectance, tolerance);
    EXPECT_NEAR(0.4, solar.Direct.Back.Reflectance, tolerance);
    EXPECT_NEAR(0.11, solar.Diffuse.Front.Transmittance, tolerance);
    EXPECT_NEAR(0.22, solar.Diffuse.Back.Transmittance, tolerance);
    EXPECT_NEAR(0.33, solar.Diffuse.Front.Reflectance, tolerance);
    EXPECT_NEAR(0.44, solar.Diffuse.Back.Reflectance, tolerance);

    ASSERT_TRUE(solid->optical->integrated->Visible.has_value());
    auto & visible{solid->optical->integrated->Visible.value()};
    EXPECT_NEAR(0.2, visible.Direct.Front.Transmittance, tolerance);
    EXPECT_NEAR(0.3, visible.Direct.Back.Transmittance, tolerance);
    EXPECT_NEAR(0.4, visible.Direct.Front.Reflectance, tolerance);
    EXPECT_NEAR(0.5, visible.Direct.Back.Reflectance, tolerance);
    EXPECT_NEAR(0.22, visible.Diffuse.Front.Transmittance, tolerance);
    EXPECT_NEAR(0.33, visible.Diffuse.Back.Transmittance, tolerance);
    EXPECT_NEAR(0.44, visible.Diffuse.Front.Reflectance, tolerance);
    EXPECT_NEAR(0.55, visible.Diffuse.Back.Reflectance, tolerance);
}

TEST_F(TestMaterialsXMLReading, Enclosure1ByUUID) {
    static const auto tolerance{1e-6};
    const std::string fileContent{TestMaterial::testDatabase()};

    File::createFileFromString(getFileName(), fileContent);

    MaterialsLibrary::DB materialDB{getFileName()};
    auto aMaterial{materialDB.getMaterialByUUID("c28a0554-3148-4a57-bb22-d93852b1604e")};

    EXPECT_EQ(aMaterial.has_value(), true);

    auto material{aMaterial.value()};

    const std::string correctName{"Radiation Enclosure"};
    EXPECT_EQ(material.Name, correctName);

    EXPECT_EQ(true, material.Protected);

    ASSERT_TRUE(MaterialsLibrary::isRadiationEnclosure(material));

    constexpr auto correct{1.0};
    EXPECT_EQ(correct, MaterialsLibrary::getRadiationEnclosure(material)->emissivityDefault);
}

TEST_F(TestMaterialsXMLReading, ReadMaterial1ByName) {
    const std::string fileContent{TestMaterial::testDatabase()};

    File::createFileFromString(getFileName(), fileContent);

    MaterialsLibrary::DB materialDB{getFileName()};
    auto aMaterial{materialDB.getMaterialByName("Test Material 1")};

    EXPECT_EQ(aMaterial.has_value(), true);

    auto material{aMaterial.value()};

    const std::string correctUUID{"26fc05d4-a7aa-43a1-b473-c984caefea5b"};
    EXPECT_EQ(material.UUID, correctUUID);

    EXPECT_EQ(true, material.Protected);

    ASSERT_TRUE(MaterialsLibrary::isSolid(material));

    const auto solid{MaterialsLibrary::getSolid(material)};

    ASSERT_TRUE(solid->hygroThermal.has_value());

    EXPECT_EQ(solid->hygroThermal->DefaultThickness.has_value(), false);

    const double bulkDensityCorrect{1330};
    EXPECT_EQ(solid->hygroThermal->BulkDensity.has_value(), true);
    EXPECT_NEAR(solid->hygroThermal->BulkDensity.value(), bulkDensityCorrect, 1e-6);

    const double porosityCorrect{0.5};
    EXPECT_EQ(solid->hygroThermal->Porosity.has_value(), true);
    EXPECT_NEAR(solid->hygroThermal->Porosity.value(), porosityCorrect, 1e-6);

    const double specificHeatCapacityDryCorrect{850};
    EXPECT_EQ(solid->hygroThermal->SpecificHeatCapacityDry.has_value(), true);
    EXPECT_NEAR(solid->hygroThermal->SpecificHeatCapacityDry.value(), specificHeatCapacityDryCorrect, 1e-6);
}

TEST_F(TestMaterialsXMLReading, GasNames) {
    const std::string fileContent{TestMaterial::testDatabase()};

    File::createFileFromString(getFileName(), fileContent);

    MaterialsLibrary::DB materialsDB{getFileName()};
    const auto & materials{materialsDB.getMaterials()};
    const auto gasNames{MaterialsLibrary::getGasNames(materials)};

    const std::set<std::string, std::less<>> expectedGasNames{"Air", "Argon"};

    EXPECT_TRUE(Helper::areSetsEqual(gasNames, expectedGasNames));
}

TEST_F(TestMaterialsXMLReading, ReadMaterialEmptyMoistureByName) {
    const std::string fileContent{TestMaterial::testDatabaseEmptyMoisture()};

    File::createFileFromString(getFileName(), fileContent);

    MaterialsLibrary::DB materialDB{getFileName()};
    auto aMaterial{materialDB.getMaterialByName("Test Material Empty Moisture 1")};

    EXPECT_EQ(aMaterial.has_value(), true);

    auto material{aMaterial.value()};

    const std::string correctUUID{"75716f29-288a-4eb0-acab-21faa5bd5807"};
    EXPECT_EQ(material.UUID, correctUUID);

    EXPECT_EQ(true, material.Protected);

    ASSERT_TRUE(MaterialsLibrary::isSolid(material));

    const auto solid{MaterialsLibrary::getSolid(material)};

    ASSERT_TRUE(solid->hygroThermal.has_value());

    EXPECT_EQ(solid->hygroThermal->DefaultThickness.has_value(), false);

    const double bulkDensityCorrect{1330};
    EXPECT_EQ(solid->hygroThermal->BulkDensity.has_value(), true);
    EXPECT_NEAR(solid->hygroThermal->BulkDensity.value(), bulkDensityCorrect, 1e-6);

    const double porosityCorrect{0.5};
    EXPECT_EQ(solid->hygroThermal->Porosity.has_value(), true);
    EXPECT_NEAR(solid->hygroThermal->Porosity.value(), porosityCorrect, 1e-6);

    const double specificHeatCapacityDryCorrect{850};
    EXPECT_EQ(solid->hygroThermal->SpecificHeatCapacityDry.has_value(), true);
    EXPECT_NEAR(solid->hygroThermal->SpecificHeatCapacityDry.value(), specificHeatCapacityDryCorrect, 1e-6);
}