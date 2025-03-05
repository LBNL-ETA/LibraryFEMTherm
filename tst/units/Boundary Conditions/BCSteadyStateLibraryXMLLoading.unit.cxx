#include <filesystem>
#include <gtest/gtest.h>

#include "BCSteadyState/DB.hxx"
#include "LibraryUtilities/FileManipulation.hxx"

#include "BoundaryConditionsSteadyStateTestUtils.hxx"
#include "bcsteadyState1.hxx"

class TestBCSteadyStateLibraryXMLLoading : public testing::Test
{
private:
    const std::string fileName{"TestSave.xml"};

protected:
    void SetUp() override
    {
        std::filesystem::remove(fileName);
    }

    void TearDown() override
    {
        std::filesystem::remove(fileName);
    }

public:
    [[nodiscard]] std::string getFileName() const
    {
        return fileName;
    }
};

TEST_F(TestBCSteadyStateLibraryXMLLoading, GetRecordByUUID_Comprehensive)
{
    const std::string fileContent{TestSteadyStateBC::testDatabase()};

    File::createFileFromString(getFileName(), fileContent);
    BCSteadyStateLibrary::DB bcLibraryDB{getFileName()};

    const std::string uuid{"4c179ded-3f34-4102-b8d6-46d8e6d1c6d9"};

    const BCSteadyStateLibrary::BoundaryCondition correct{
      uuid,
      "Interior (20 tilt) Aluminum Frame (convection only)",
      true,
      "0xFF0000",
      BCSteadyStateLibrary::Comprehensive{
        0.5,
        BCSteadyStateLibrary::Convection{21.0, 4.65},
        std::nullopt,
        BCSteadyStateLibrary::Radiation{BCSteadyStateLibrary::AutomaticEnclosure{21.0, 1.0}}},
      ""};

    const auto bc{bcLibraryDB.getByUUID(uuid)};

    EXPECT_EQ(bc.has_value(), true);

    constexpr auto tolerance{1e-6};

    validateSteadyStateBoundaryCondition(correct, bc.value(), tolerance);
}

TEST_F(TestBCSteadyStateLibraryXMLLoading, GetRecordByUUID_Comprehensive_InvalidRadiation)
{
    const std::string fileContent{TestSteadyStateBC::testDatabase()};

    File::createFileFromString(getFileName(), fileContent);
    BCSteadyStateLibrary::DB bcLibraryDB{getFileName()};

    const std::string uuid{"4c179ded-3f34-4102-b8d6-46d8e6d1c6d9"};

    const BCSteadyStateLibrary::BoundaryCondition correct{
      uuid,
      "Interior (20 tilt) Aluminum Frame (convection only)",
      true,
      "0xFF0000",
      BCSteadyStateLibrary::Comprehensive{0.5,
                                          BCSteadyStateLibrary::Convection{21.0, 4.65},
                                          std::nullopt,
                                          BCSteadyStateLibrary::Radiation{BCSteadyStateLibrary::ManualEnclosure{}}},
      ""};

    const auto bc{bcLibraryDB.getByUUID(uuid)};

    EXPECT_EQ(bc.has_value(), true);

    constexpr auto tolerance{1e-6};

    EXPECT_THROW(validateSteadyStateBoundaryCondition(correct, bc.value(), tolerance), MismatchRadiationTypes);
}

TEST_F(TestBCSteadyStateLibraryXMLLoading, GetRecordByUUID_InvalidBCType)
{
    const std::string fileContent{TestSteadyStateBC::testDatabase()};

    File::createFileFromString(getFileName(), fileContent);
    BCSteadyStateLibrary::DB bcLibraryDB{getFileName()};

    const std::string uuid{"4c179ded-3f34-4102-b8d6-46d8e6d1c6d9"};

    const BCSteadyStateLibrary::BoundaryCondition correct{
      uuid,
      "Interior (20 tilt) Aluminum Frame (convection only)",
      true,
      "0xFF0000",
      BCSteadyStateLibrary::RadiationSurface{.temperature = 21.0, .emissivity = 1.0},
      ""};

    const auto bc{bcLibraryDB.getByUUID(uuid)};

    EXPECT_EQ(bc.has_value(), true);

    constexpr auto tolerance{1e-6};

    EXPECT_THROW(validateSteadyStateBoundaryCondition(correct, bc.value(), tolerance), MismatchBoundaryConditionType);
}


TEST_F(TestBCSteadyStateLibraryXMLLoading, GetRecordByName_Simplified)
{
    const std::string fileContent{TestSteadyStateBC::testDatabase()};

    File::createFileFromString(getFileName(), fileContent);
    BCSteadyStateLibrary::DB bcLibraryDB{getFileName()};

    const std::string name{"Adiabatic"};

    const BCSteadyStateLibrary::BoundaryCondition correct{
      "61d7bd1c-22c6-4ea0-8720-0696e8c194ad", name, true, "0xFFFFFF", BCSteadyStateLibrary::Simplified{0, 0, 0.5}, ""};

    const auto bc{bcLibraryDB.getByName(name)};

    EXPECT_EQ(bc.has_value(), true);

    constexpr auto tolerance{1e-6};

    validateSteadyStateBoundaryCondition(correct, bc.value(), tolerance);
}

TEST_F(TestBCSteadyStateLibraryXMLLoading, GetRecordByName_RadiationSurface)
{
    const std::string fileContent{TestSteadyStateBC::testDatabase()};

    File::createFileFromString(getFileName(), fileContent);
    BCSteadyStateLibrary::DB bcLibraryDB{getFileName()};

    const std::string name{"Radiation Surface"};

    const BCSteadyStateLibrary::BoundaryCondition correct{
      "d2539cd7-c6e6-4265-b2b9-dc482b1e1798",
      name,
      true,
      "0x00FF00",
      BCSteadyStateLibrary::RadiationSurface{.temperature = 21.0, .emissivity = 1.0},
      ""};

    const auto bc{bcLibraryDB.getByName(name)};

    EXPECT_EQ(bc.has_value(), true);

    constexpr auto tolerance{1e-6};

    validateSteadyStateBoundaryCondition(correct, bc.value(), tolerance);
}

TEST_F(TestBCSteadyStateLibraryXMLLoading, GetDefaultRadiationSurface)
{
    const std::string fileContent{TestSteadyStateBC::testDatabase()};

    File::createFileFromString(getFileName(), fileContent);
    BCSteadyStateLibrary::DB bcLibraryDB{getFileName()};

    const BCSteadyStateLibrary::BoundaryCondition correct{
      "04e294fb-beaa-4838-b663-a05e0942bb72",
      "Default Radiation Surface",
      true,
      "0xFF0000",
      BCSteadyStateLibrary::RadiationSurface{.temperature = 21.0, .emissivity = 1.0},
      ""};

    const auto bc{bcLibraryDB.getDefaultRadiationSurface()};

    EXPECT_EQ(bc.has_value(), true);

    constexpr auto tolerance{1e-6};

    validateSteadyStateBoundaryCondition(correct, bc.value(), tolerance);
}