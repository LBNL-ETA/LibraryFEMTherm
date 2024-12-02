#include <filesystem>
#include <gtest/gtest.h>

#include "BCTransient/Types.hxx"
#include "LibraryUtilities/FileManipulation.hxx"

#include "bctype1.hxx"

class TestBCLibraryXMLReading : public testing::Test
{
protected:

    void SetUp() override
    {}
};

TEST_F(TestBCLibraryXMLReading, ReadBCLibraryXML)
{
    const std::string fileContent{TestBCType::testDatabase()};
    const std::string fileName{"TestRead.xml"};

    File::createFileFromString(fileName, fileContent);

    BCTypesLibrary::DB bcLibraryDB{fileName};
    auto aBC{bcLibraryDB.getByUUID("d5bea8e8-b241-11e9-a2a3-2a2ae2dbcce4")};

    EXPECT_EQ(aBC.has_value(), true);

    auto bc{aBC.value()};

    const std::string correctName{"SteadyState Interior"};
    EXPECT_EQ(bc.Name, correctName);

    EXPECT_EQ(true, bc.Protected);    

    EXPECT_EQ(bc.bcType, BCTypesLibrary::BCType::SteadyState);

    EXPECT_EQ(bc.bcModel, BCTypesLibrary::BCModel::Neumann);

    // Convection
    EXPECT_EQ(bc.ConvectionBc.has_value(), true);

    EXPECT_EQ(bc.ConvectionBc->Model, BCTypesLibrary::ConvectionModel::Fixed_Convection_Coefficient);

    EXPECT_EQ(bc.ConvectionBc->FilmCoefficient.has_value(), true);
    const double fixedFilmCoefficient{2.5};
    EXPECT_NEAR(bc.ConvectionBc->FilmCoefficient.value(), fixedFilmCoefficient, 1e-6);

    EXPECT_EQ(bc.ConvectionBc->Temperature.has_value(), true);
    const double temperature{21};
    EXPECT_NEAR(bc.ConvectionBc->Temperature.value(), temperature, 1e-6);

    EXPECT_EQ(bc.ConvectionBc->WindSpeed.has_value(), false);
    EXPECT_EQ(bc.ConvectionBc->WindDirection.has_value(), false);

    // Radiation
    EXPECT_EQ(bc.RadiationBc.has_value(), true);
    EXPECT_EQ(bc.RadiationBc->Model, BCTypesLibrary::RadiationModel::Automatic_Enclosure);

    EXPECT_EQ(bc.RadiationBc->Temperature.has_value(), true);
    const double radiationTemperature{10};
    EXPECT_NEAR(bc.RadiationBc->Temperature.value(), radiationTemperature, 1e-6);

    EXPECT_EQ(bc.RadiationBc->SurfaceEmissivity.has_value(), true);
    const double surfaceEmissivity{0.9};
    EXPECT_NEAR(bc.RadiationBc->SurfaceEmissivity.value(), surfaceEmissivity, 1e-6);

    EXPECT_EQ(bc.RadiationBc->ViewFactor.has_value(), true);
    const double viewFactor{1.0};
    EXPECT_NEAR(bc.RadiationBc->ViewFactor.value(), viewFactor, 1e-6);

    EXPECT_EQ(bc.UseHeatFlux, false);
    EXPECT_EQ(bc.UseTemperature, false);
    EXPECT_EQ(bc.UseHumidity, false);

    std::filesystem::remove(fileName);
}