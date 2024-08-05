#include <filesystem>
#include <gtest/gtest.h>

#include "BCSteadyState/DB.hxx"

class TestBCSteadyStateLibraryXMLSaving : public testing::Test
{
protected:
    void SetUp() override
    {}
};

TEST_F(TestBCSteadyStateLibraryXMLSaving, Simplified)
{
    const std::string fileName{"TestSave.xml"};

    BCSteadyStateLibrary::DB bcLibraryDB{fileName};

    const std::string uuid{"7a863ad6-c537-11ea-87d0-0242ac130003"};

    BCSteadyStateLibrary::BoundaryCondition record{uuid};
    record.Name = "Test Name";
    record.Color = "0xFAB2A6";

    BCSteadyStateLibrary::Simplified simplified;
    record.data = simplified;

    bcLibraryDB.addBoundaryCondition(record);
    const auto error{bcLibraryDB.saveToFile()};
    EXPECT_EQ(error, 0);

    BCSteadyStateLibrary::DB testDB{fileName};

    const auto testRecord{testDB.getBoundaryConditionByUUID(uuid)};

    const auto recordExists{testRecord.has_value()};
    EXPECT_EQ(recordExists, true);

    const auto name{testRecord->Name};
    EXPECT_EQ(name, "Test Name");

    const auto isProtected{testRecord->Protected};
    EXPECT_EQ(isProtected, false);

    const auto color{testRecord->Color};
    EXPECT_EQ(color, "0xFAB2A6");

    ASSERT_TRUE(std::holds_alternative<BCSteadyStateLibrary::Simplified>(testRecord->data));

    if(std::holds_alternative<BCSteadyStateLibrary::Simplified>(testRecord->data))
    {
        constexpr auto tolerance{1e-6};
        const auto & data = std::get<BCSteadyStateLibrary::Simplified>(testRecord->data);
        EXPECT_NEAR(data.temperature, 0.0, tolerance);
        EXPECT_NEAR(data.filmCoefficient, 0.0, tolerance);
        EXPECT_NEAR(data.relativeHumidity, 0.5, tolerance);
    }


    std::filesystem::remove(fileName);
}

TEST_F(TestBCSteadyStateLibraryXMLSaving, RadiationSurface)
{
    const std::string fileName{"TestSave.xml"};

    BCSteadyStateLibrary::DB bcLibraryDB{fileName};

    const std::string uuid{"7a863ad6-c537-11ea-87d0-0242ac130003"};

    BCSteadyStateLibrary::BoundaryCondition record{uuid};
    record.Name = "Test Name";
    record.Color = "0xFAB2A6";

    BCSteadyStateLibrary::RadiationSurface radiation;
    record.data = radiation;

    bcLibraryDB.addBoundaryCondition(record);
    const auto error{bcLibraryDB.saveToFile()};
    EXPECT_EQ(error, 0);

    BCSteadyStateLibrary::DB testDB{fileName};

    const auto testRecord{testDB.getBoundaryConditionByUUID(uuid)};

    const auto recordExists{testRecord.has_value()};
    EXPECT_EQ(recordExists, true);

    const auto name{testRecord->Name};
    EXPECT_EQ(name, "Test Name");

    const auto isProtected{testRecord->Protected};
    EXPECT_EQ(isProtected, false);

    const auto color{testRecord->Color};
    EXPECT_EQ(color, "0xFAB2A6");

    ASSERT_TRUE(std::holds_alternative<BCSteadyStateLibrary::RadiationSurface>(testRecord->data));

    if(std::holds_alternative<BCSteadyStateLibrary::RadiationSurface>(testRecord->data))
    {
        constexpr auto tolerance{1e-6};
        const auto & data = std::get<BCSteadyStateLibrary::RadiationSurface>(testRecord->data);
        EXPECT_NEAR(data.temperature, 0.0, tolerance);
        EXPECT_NEAR(data.emissivity, 0.0, tolerance);
    }


    std::filesystem::remove(fileName);
}

TEST_F(TestBCSteadyStateLibraryXMLSaving, Comprehensive)
{
    const std::string fileName{"TestSave.xml"};

    BCSteadyStateLibrary::DB bcLibraryDB{fileName};

    const std::string uuid{"7a863ad6-c537-11ea-87d0-0242ac130003"};

    BCSteadyStateLibrary::BoundaryCondition record{uuid};
    record.Name = "Test Name";
    record.Color = "0xFAB2A6";

    BCSteadyStateLibrary::Comprehensive comprehensive;
    BCSteadyStateLibrary::Convection convection;
    BCSteadyStateLibrary::ConstantFlux constantFlux;
    BCSteadyStateLibrary::Radiation radiation;
    comprehensive.convection = convection;
    comprehensive.constantFlux = constantFlux;
    comprehensive.radiation = radiation;
    record.data = comprehensive;

    bcLibraryDB.addBoundaryCondition(record);
    const auto error{bcLibraryDB.saveToFile()};
    EXPECT_EQ(error, 0);

    BCSteadyStateLibrary::DB testDB{fileName};

    const auto testRecord{testDB.getBoundaryConditionByUUID(uuid)};

    const auto recordExists{testRecord.has_value()};
    EXPECT_EQ(recordExists, true);

    const auto name{testRecord->Name};
    EXPECT_EQ(name, "Test Name");

    const auto isProtected{testRecord->Protected};
    EXPECT_EQ(isProtected, false);

    const auto color{testRecord->Color};
    EXPECT_EQ(color, "0xFAB2A6");

    ASSERT_TRUE(std::holds_alternative<BCSteadyStateLibrary::Comprehensive>(testRecord->data));

    if(std::holds_alternative<BCSteadyStateLibrary::Comprehensive>(testRecord->data))
    {
        constexpr auto tolerance{1e-6};
        const auto & data = std::get<BCSteadyStateLibrary::Comprehensive>(testRecord->data);
        EXPECT_NEAR(data.relativeHumidity, 0.5, tolerance);
        EXPECT_EQ(data.convection.has_value(), true);
        if(data.convection.has_value())
        {
            EXPECT_NEAR(data.convection->temperature, 0.0, tolerance);
            EXPECT_NEAR(data.convection->filmCoefficient, 0.0, tolerance);
        }

        EXPECT_EQ(data.constantFlux.has_value(), true);
        if(data.constantFlux.has_value())
        {
            EXPECT_NEAR(data.constantFlux->flux, 0.0, tolerance);
        }

        EXPECT_EQ(data.radiation.has_value(), true);
    }

    std::filesystem::remove(fileName);
}
