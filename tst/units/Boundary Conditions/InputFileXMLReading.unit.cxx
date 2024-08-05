#include <filesystem>
#include <gtest/gtest.h>

#include "BCTransient/Timestep.hxx"
#include "LibraryUtilities/FileManipulation.hxx"

#include "InputFileTestDatabase.hxx"

class TestInputFileXMLReading : public testing::Test
{
private:
    const std::string fileName_{"TestRead.xml"};

protected:

    void SetUp() override
    {
        std::filesystem::remove(fileName_);
    }

    void TearDown() override
    {
        std::filesystem::remove(fileName_);
    }

public:
    [[nodiscard]] std::string fileName() const
    {
        return fileName_;
    }
};

TEST_F(TestInputFileXMLReading, TARPInside)
{
    const std::string fileContent{InputXMLBoundaryTest::tarpInside()};

    File::createFileFromString(fileName(), fileContent);

    BCInputFileLibrary::BoundaryConditionTimestep inputFile;
    inputFile.loadFromXMLFile(fileName());

    EXPECT_EQ(inputFile.name, "Comprehensive Natural Model Exterior");

    EXPECT_EQ(inputFile.convection.tarp.size(), 5u);

    std::vector<double> correctTemperatures{-18, -19.8, -20, -20, -18.5};
    std::vector<double> correctHumidities{0.2, 0.2, 0.3, 0.3, 0.3};

    for(size_t i = 0u; i < inputFile.convection.tarp.size(); ++i)
    {
        EXPECT_EQ(correctTemperatures[i], inputFile.convection.tarp[i].temperature);
        EXPECT_EQ(correctHumidities[i], inputFile.convection.tarp[i].humidity);
    }
}

TEST_F(TestInputFileXMLReading, ASHRAEInside)
{
    const std::string fileContent{InputXMLBoundaryTest::ashraeInside()};

    File::createFileFromString(fileName(), fileContent);

    BCInputFileLibrary::BoundaryConditionTimestep inputFile;
    inputFile.loadFromXMLFile(fileName());

    EXPECT_EQ(inputFile.name, "Comprehensive Natural Model Exterior");

    EXPECT_EQ(inputFile.convection.ashraeInside.size(), 5u);

    std::vector<double> correctTemperatures{18, 19.8, 20, 20, 18.5};
    std::vector<double> correctHumidities{0.2, 0.2, 0.3, 0.3, 0.3};

    for(size_t i = 0u; i < inputFile.convection.ashraeInside.size(); ++i)
    {
        EXPECT_EQ(correctTemperatures[i], inputFile.convection.ashraeInside[i].temperature);
        EXPECT_EQ(correctHumidities[i], inputFile.convection.ashraeInside[i].humidity);
    }
}

TEST_F(TestInputFileXMLReading, ASHRAEOutside)
{
    const std::string fileContent{InputXMLBoundaryTest::ashraeOutside()};

    File::createFileFromString(fileName(), fileContent);

    BCInputFileLibrary::BoundaryConditionTimestep inputFile;
    inputFile.loadFromXMLFile(fileName());

    EXPECT_EQ(inputFile.name, "Comprehensive Natural Model Exterior");

    EXPECT_EQ(inputFile.convection.ashraeOutside.size(), 5u);

    std::vector<double> correctTemperatures{-18, -19.8, -20, -20, -18.5};
    std::vector<double> correctHumidities{0.23, 0.2, 0.3, 0.3, 0.3};
    std::vector<double> correctWindSpeed{1.2, 2.2, 2.7, 3.0, 3.0};

    for(size_t i = 0u; i < inputFile.convection.ashraeOutside.size(); ++i)
    {
        EXPECT_EQ(correctTemperatures[i], inputFile.convection.ashraeOutside[i].temperature);
        EXPECT_EQ(correctHumidities[i], inputFile.convection.ashraeOutside[i].humidity);
        EXPECT_EQ(correctWindSpeed[i], inputFile.convection.ashraeOutside[i].windSpeed);
    }
}

TEST_F(TestInputFileXMLReading, FixedFilmCoefficient)
{
    const std::string fileContent{InputXMLBoundaryTest::fixedFilmCoefficient()};

    File::createFileFromString(fileName(), fileContent);

    BCInputFileLibrary::BoundaryConditionTimestep inputFile;
    inputFile.loadFromXMLFile(fileName());

    EXPECT_EQ(inputFile.name, "Fixed Film Coefficient Exterior");

    EXPECT_EQ(inputFile.convection.fixedFilmCoefficient.size(), 5u);

    std::vector<double> correctTemperatures{-18, -19.8, -20, -20, -18.5};
    std::vector<double> correctHumidities{0.2, 0.2, 0.3, 0.3, 0.3};
    std::vector<double> correctFilmCoefficent{8, 4, 4, 4, 1};

    for(size_t i = 0u; i < inputFile.convection.fixedFilmCoefficient.size(); ++i)
    {
        EXPECT_EQ(correctTemperatures[i], inputFile.convection.fixedFilmCoefficient[i].temperature);
        EXPECT_EQ(correctHumidities[i], inputFile.convection.fixedFilmCoefficient[i].humidity);
        EXPECT_EQ(correctFilmCoefficent[i], inputFile.convection.fixedFilmCoefficient[i].fixedFilmCoefficient);
    }
}

TEST_F(TestInputFileXMLReading, YazdanianKlems)
{
    const std::string fileContent{InputXMLBoundaryTest::yazdanianKlems()};

    File::createFileFromString(fileName(), fileContent);

    BCInputFileLibrary::BoundaryConditionTimestep inputFile;
    inputFile.loadFromXMLFile(fileName());

    EXPECT_EQ(inputFile.name, "Comprehensive Natural Model Exterior");

    EXPECT_EQ(inputFile.convection.yazdanianKlems.size(), 5u);

    std::vector<double> correctTemperatures{-18, -19.8, -20, -20, -18.5};
    std::vector<double> correctHumidities{0.2, 0.2, 0.3, 0.3, 0.3};
    std::vector<double> correctWindSpeed{1.2, 2.2, 2.7, 3.0, 3.0};
    std::vector<double> correctWindDirection{45, 35, 55, 115, 160};

    for(size_t i = 0u; i < inputFile.convection.yazdanianKlems.size(); ++i)
    {
        EXPECT_EQ(correctTemperatures[i], inputFile.convection.yazdanianKlems[i].temperature);
        EXPECT_EQ(correctHumidities[i], inputFile.convection.yazdanianKlems[i].humidity);
        EXPECT_EQ(correctWindSpeed[i], inputFile.convection.yazdanianKlems[i].windSpeed);
        EXPECT_EQ(correctWindDirection[i], inputFile.convection.yazdanianKlems[i].windDirection);
    }
}

TEST_F(TestInputFileXMLReading, Kimura)
{
    const std::string fileContent{InputXMLBoundaryTest::kimura()};

    File::createFileFromString(fileName(), fileContent);

    BCInputFileLibrary::BoundaryConditionTimestep inputFile;
    inputFile.loadFromXMLFile(fileName());

    EXPECT_EQ(inputFile.name, "Comprehensive Natural Model Exterior");

    EXPECT_EQ(inputFile.convection.kimura.size(), 5u);

    std::vector<double> correctTemperatures{-18, -19.8, -20, -20, -18.5};
    std::vector<double> correctHumidities{0.2, 0.2, 0.3, 0.3, 0.3};
    std::vector<double> correctWindSpeed{1.2, 2.2, 2.7, 3.0, 3.0};
    std::vector<double> correctWindDirection{220, 245, 200, 190, 165};

    for(size_t i = 0u; i < inputFile.convection.kimura.size(); ++i)
    {
        EXPECT_EQ(correctTemperatures[i], inputFile.convection.kimura[i].temperature);
        EXPECT_EQ(correctHumidities[i], inputFile.convection.kimura[i].humidity);
        EXPECT_EQ(correctWindSpeed[i], inputFile.convection.kimura[i].windSpeed);
        EXPECT_EQ(correctWindDirection[i], inputFile.convection.kimura[i].windDirection);
    }
}

TEST_F(TestInputFileXMLReading, Montazeri)
{
    const std::string fileContent{InputXMLBoundaryTest::montazeri()};

    File::createFileFromString(fileName(), fileContent);

    BCInputFileLibrary::BoundaryConditionTimestep inputFile;
    inputFile.loadFromXMLFile(fileName());

    EXPECT_EQ(inputFile.name, "Comprehensive Natural Model Exterior");

    EXPECT_EQ(inputFile.convection.montazeri.size(), 5u);

    std::vector<double> correctTemperatures{-18, -19.8, -20, -20, -18.5};
    std::vector<double> correctHumidities{0.2, 0.2, 0.3, 0.3, 0.3};
    std::vector<double> correctWindSpeed{1.2, 2.2, 2.7, 3.0, 3.0};
    std::vector<double> correctWindDirection{45, 35, 55, 115, 160};

    for(size_t i = 0u; i < inputFile.convection.montazeri.size(); ++i)
    {
        EXPECT_EQ(correctTemperatures[i], inputFile.convection.montazeri[i].temperature);
        EXPECT_EQ(correctHumidities[i], inputFile.convection.montazeri[i].humidity);
        EXPECT_EQ(correctWindSpeed[i], inputFile.convection.montazeri[i].windSpeed);
        EXPECT_EQ(correctWindDirection[i], inputFile.convection.montazeri[i].windDirection);
    }
}

TEST_F(TestInputFileXMLReading, LinearizedRadiation)
{
    const std::string fileContent{InputXMLBoundaryTest::linearizedRadiation()};

    File::createFileFromString(fileName(), fileContent);

    BCInputFileLibrary::BoundaryConditionTimestep inputFile;
    inputFile.loadFromXMLFile(fileName());

    EXPECT_EQ(inputFile.name, "Fixed Film Coefficient Interior");

    EXPECT_EQ(inputFile.radiation.fixedRadiation.size(), 5u);

    std::vector<double> correctTemperatures{20, 21, 23, 23, 23};
    std::vector<double> correctFilmCoefficient{2.3, 2.3, 2.6, 2.4, 2.5};

    for(size_t i = 0u; i < inputFile.radiation.fixedRadiation.size(); ++i)
    {
        EXPECT_EQ(correctTemperatures[i], inputFile.radiation.fixedRadiation[i].temperature);
        EXPECT_EQ(correctFilmCoefficient[i], inputFile.radiation.fixedRadiation[i].hr);
    }
}

TEST_F(TestInputFileXMLReading, BlackBodyRadiation)
{
    const std::string fileContent{InputXMLBoundaryTest::blackBodyRadiation()};

    File::createFileFromString(fileName(), fileContent);

    BCInputFileLibrary::BoundaryConditionTimestep inputFile;
    inputFile.loadFromXMLFile(fileName());

    EXPECT_EQ(inputFile.name, "Fixed Film Coefficient Interior");

    EXPECT_EQ(inputFile.radiation.blackBodyRadiation.size(), 5u);

    std::vector<double> correctTemperatures{20, 21, 23, 23, 23};
    std::vector<double> correctFilmCoefficient{0.9, 0.9, 0.9, 0.9, 0.9};

    for(size_t i = 0u; i < inputFile.radiation.blackBodyRadiation.size(); ++i)
    {
        EXPECT_EQ(correctTemperatures[i], inputFile.radiation.blackBodyRadiation[i].temperature);
        EXPECT_EQ(correctFilmCoefficient[i], inputFile.radiation.blackBodyRadiation[i].emissivity);
    }
}

TEST_F(TestInputFileXMLReading, FixedTemperatureAndHumidity)
{
    const std::string fileContent{InputXMLBoundaryTest::fixedTemperatureAndHumidity()};

    File::createFileFromString(fileName(), fileContent);

    BCInputFileLibrary::BoundaryConditionTimestep inputFile;
    inputFile.loadFromXMLFile(fileName());

    EXPECT_EQ(inputFile.name, "Fixed Film Coefficient Interior");

    EXPECT_EQ(inputFile.temperature.size(), 10u);

    std::vector<double> correctTemperatures{20, 21, 21, 21, 21, 22, 22, 23, 22, 20};

    for(size_t i = 0u; i < inputFile.temperature.size(); ++i)
    {
        EXPECT_EQ(correctTemperatures[i], inputFile.temperature[i].temperature);
    }

    EXPECT_EQ(inputFile.humidity.size(), 10u);

    std::vector<double> correctHumidities{0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    for(size_t i = 0u; i < inputFile.humidity.size(); ++i)
    {
        EXPECT_EQ(correctHumidities[i], inputFile.humidity[i].humidity);
    }
}

TEST_F(TestInputFileXMLReading, FixedHeatFlux)
{
    const std::string fileContent{InputXMLBoundaryTest::fixedHeatFlux()};

    File::createFileFromString(fileName(), fileContent);

    BCInputFileLibrary::BoundaryConditionTimestep inputFile;
    inputFile.loadFromXMLFile(fileName());

    EXPECT_EQ(inputFile.name, "Comprehensive Natural Model Exterior");

    EXPECT_EQ(inputFile.heatFlux.size(), 5u);

    std::vector<double> correctHeatFlux{20, 20, 20, 20, 20};

    for(size_t i = 0u; i < inputFile.heatFlux.size(); ++i)
    {
        EXPECT_EQ(correctHeatFlux[i], inputFile.heatFlux[i].heatFlux);
    }
}