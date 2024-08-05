#include <filesystem>

#include <gtest/gtest.h>

#include "Materials/Materials.hxx"
#include "Materials/DB.hxx"

#include "LibraryUtilities/FileManipulation.hxx"

#include "material1.hxx"

class TestMaterialsXMLSaving : public testing::Test
{
protected:
    void SetUp() override
    {}
};

TEST_F(TestMaterialsXMLSaving, SaveMaterialXML)
{
    const std::string fileName{"TestSave.xm"};

    MaterialsLibrary::DB materialDB{fileName};

    const std::string uuid{"7a863ad6-c537-11ea-87d0-0242ac130003"};

    MaterialsLibrary::Material record = MaterialsLibrary::generate(uuid, MaterialsLibrary::MaterialType::Solid);

    materialDB.addMaterial(record);
    const auto error{materialDB.saveToFile()};
    EXPECT_EQ(error, 0);

    MaterialsLibrary::DB testDB{fileName};

    const auto testRecord{testDB.getMaterialByUUID(uuid)};

    EXPECT_EQ(testRecord.has_value(), true);

    EXPECT_EQ(testRecord->Name, "Default Name");
    EXPECT_EQ(testRecord->Protected, false);
    EXPECT_EQ(testRecord->Color, "0xFFFFFF");

    std::filesystem::remove(fileName);
}

TEST_F(TestMaterialsXMLSaving, UpdateMaterialXMLProperties)
{
    const std::string fileName{"TestSave.xm"};

    MaterialsLibrary::DB materialDB{fileName};

    const std::string uuid{"7a863ad6-c537-11ea-87d0-0242ac130004"};

    MaterialsLibrary::Material record = MaterialsLibrary::generate(uuid, MaterialsLibrary::MaterialType::Solid);

    materialDB.addMaterial(record);

    record.Name = "Test Name";

    auto solid{MaterialsLibrary::getSolid(record)};

    ASSERT_TRUE(solid->hygroThermal.has_value());

    solid->hygroThermal->Roughness = MaterialsLibrary::MaterialRoughness::Smooth;

    materialDB.updateMaterial(record);

    const auto error{materialDB.saveToFile()};
    EXPECT_EQ(error, 0);

    MaterialsLibrary::DB testDB{fileName};

    const auto testRecord{testDB.getMaterialByUUID(uuid)};
    ASSERT_TRUE(testRecord.has_value());

    ASSERT_TRUE(MaterialsLibrary::isSolid(testRecord.value()));

    const auto testSolid{MaterialsLibrary::getSolid(testRecord.value())};

    EXPECT_EQ(testRecord->Name, "Test Name");
    EXPECT_EQ(testRecord->Protected, false);
    EXPECT_EQ(testRecord->Color, "0xFFFFFF");
    ASSERT_EQ(true, testSolid->hygroThermal->Roughness.has_value());
    EXPECT_EQ(MaterialsLibrary::MaterialRoughness::Smooth, testSolid->hygroThermal->Roughness.value());

    std::filesystem::remove(fileName);
}

TEST_F(TestMaterialsXMLSaving, AddMoistureStorageFunction)
{
    const std::string fileName{"TestSave.xm"};

    MaterialsLibrary::DB materialDB{fileName};

    const std::string uuid{"7a863ad6-c537-11ea-87d0-0242ac130004"};

    MaterialsLibrary::Material record = MaterialsLibrary::generate(uuid, MaterialsLibrary::MaterialType::Solid);

    std::vector<MaterialsLibrary::point> moistureStorageFunction{{0, 0}, {1, 1}};

    ASSERT_TRUE(MaterialsLibrary::isSolid(record));

    auto solid{MaterialsLibrary::getSolid(record)};

    ASSERT_TRUE(solid->hygroThermal.has_value());

    solid->hygroThermal->MoistureStorageFunction = moistureStorageFunction;

    materialDB.addMaterial(record);

    const auto error{materialDB.saveToFile()};
    EXPECT_EQ(error, 0);

    MaterialsLibrary::DB testDB{fileName};

    const auto testRecord{testDB.getMaterialByUUID(uuid)};

    ASSERT_TRUE(testRecord.has_value());
    ASSERT_TRUE(MaterialsLibrary::isSolid(testRecord.value()));

    EXPECT_EQ(testRecord->Name, "Default Name");
    EXPECT_EQ(testRecord->Protected, false);
    EXPECT_EQ(testRecord->Color, "0xFFFFFF");

    const auto msfSolid{MaterialsLibrary::getSolid(testRecord.value())};

    const auto loadedMSF{msfSolid->hygroThermal->MoistureStorageFunction};
    EXPECT_EQ(loadedMSF.has_value(), true);
    EXPECT_EQ(loadedMSF->size(), 2u);

    const std::vector<double> correctX{0, 1};
    const std::vector<double> correctY{0, 1};

    for(size_t i = 0u; i < loadedMSF->size(); ++i)
    {
        EXPECT_EQ(loadedMSF.value()[i].x, correctX[i]);
        EXPECT_EQ(loadedMSF.value()[i].y, correctY[i]);
    }

    std::filesystem::remove(fileName);
}

TEST_F(TestMaterialsXMLSaving, DeleteMaterialXML)
{
    const std::string fileContent{TestMaterial::testDatabase()};
    const std::string fileName{"TestDeleteMaterial.xm"};

    File::createFileFromString(fileName, fileContent);

    MaterialsLibrary::DB materialDB{fileName};

    const std::string uuid{"26fc05d4-a7aa-43a1-b473-c984caefea5b"};

    materialDB.deleteMaterialWithUUID(uuid);

    const auto error{materialDB.saveToFile()};
    EXPECT_EQ(error, 0);

    MaterialsLibrary::DB testDB{fileName};

    const auto testRecord{testDB.getMaterialByUUID(uuid)};

    EXPECT_EQ(testRecord.has_value(), false);

    std::filesystem::remove(fileName);
}
