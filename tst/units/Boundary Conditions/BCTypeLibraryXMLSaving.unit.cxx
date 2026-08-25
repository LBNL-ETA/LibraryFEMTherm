#include <filesystem>
#include <gtest/gtest.h>

#include "Legacy/Step1/BCTransient/Types.hxx"

#include "bctype1.hxx"

class TestBCLibraryXMLSaving : public testing::Test
{
protected:
    void SetUp() override
    {}
};

TEST_F(TestBCLibraryXMLSaving, SaveBCLibraryXML1)
{
    const std::string fileName{"TestSave.xml"};

    BCTypesLibrary::DB bcLibraryDB{fileName};

    const std::string uuid{"7a863ad6-c537-11ea-87d0-0242ac130003"};

    BCTypesLibrary::TypeRecord record{uuid};

    bcLibraryDB.add(record);
    const auto error{bcLibraryDB.saveToFile()};
    EXPECT_EQ(error, 0);

    BCTypesLibrary::DB testDB{fileName};

    const auto testRecord{testDB.getByUUID(uuid)};

    const auto recordExists{testRecord.has_value()};
    EXPECT_EQ(recordExists, true);

    const auto name{testRecord->Name};
    EXPECT_EQ(name, "Default Name");

    const auto prot{testRecord->Protected};
    EXPECT_EQ(prot, false);

    const auto bcType{testRecord->bcType};
    EXPECT_EQ(bcType, BCTypesLibrary::BCType::Transient);

    const auto bcModel{testRecord->bcModel};
    EXPECT_EQ(bcModel, BCTypesLibrary::BCModel::Neumann);

    const auto convectionBC{testRecord->ConvectionBc};
    EXPECT_EQ(convectionBC.has_value(), true);
    EXPECT_EQ(convectionBC->Model, BCTypesLibrary::ConvectionModel::Fixed_Convection_Coefficient);

    const auto useHeatFlux{testRecord->UseHeatFlux};
    EXPECT_EQ(useHeatFlux, false);

    const auto useTemperature{testRecord->UseTemperature};
    EXPECT_EQ(useTemperature, false);

    const auto useHumidity{testRecord->UseHumidity};
    EXPECT_EQ(useHumidity, false);

    EXPECT_EQ(testRecord->Color, "FFFFFF");

    std::filesystem::remove(fileName);
}

TEST_F(TestBCLibraryXMLSaving, VersionRoundTrip)
{
    // The shipped legacy library files carry a Version element that this DB used to
    // drop on load and omit on save - the single outlier among the library DBs.
    const std::string content{"<BoundaryConditionsType>\n"
                              "\t<Version>7</Version>\n"
                              "</BoundaryConditionsType>"};

    BCTypesLibrary::DB bcLibraryDB;
    bcLibraryDB.loadFromString(content);

    const auto saved{bcLibraryDB.saveToString()};
    EXPECT_NE(saved.find("<Version>7</Version>"), std::string::npos);
}
