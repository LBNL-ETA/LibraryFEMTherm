#include <filesystem>
#include <gtest/gtest.h>

#include "Gases/Gases.hxx"
#include "Gases/DB.hxx"
#include "LibraryUtilities/FileManipulation.hxx"
#include "LibraryUtilities/Common.hxx"

#include "gases1.hxx"

class TestGasesXMLReading : public testing::Test
{
protected:
    void SetUp() override
    {}
};

void testInterpolationCoefficients(const GasesLibrary::Coefficients & correct,
                                   const GasesLibrary::Coefficients & coefficients,
                                   const double tolerance)
{
    EXPECT_NEAR(correct.A, coefficients.A, tolerance);
    EXPECT_NEAR(correct.B, coefficients.B, tolerance);
    EXPECT_NEAR(correct.C, coefficients.C, tolerance);
}

TEST_F(TestGasesXMLReading, ReadPureGasByUUID_NonExistent)
{
    const std::string fileContent{TestGases::testDatabase()};
    const std::string fileName{"TestRead.xm"};

    File::createFileFromString(fileName, fileContent);

    GasesLibrary::DB gasesDB{fileName};
    const auto aGas{gasesDB.getPureGasByUUID("444d94e4-326e-4c1c-aef1-666771b569ce")};

    EXPECT_EQ(aGas.has_value(), false);

    std::filesystem::remove(fileName);
}

TEST_F(TestGasesXMLReading, ReadPureGasByUUID)
{
    constexpr auto tolerance{1e-6};
    const std::string fileContent{TestGases::testDatabase()};
    const std::string fileName{"TestRead.xm"};

    File::createFileFromString(fileName, fileContent);

    GasesLibrary::DB gasesDB{fileName};
    const auto aGas{gasesDB.getPureGasByUUID("444d94e4-326e-4c1c-aef1-666771b569cd")};

    EXPECT_EQ(aGas.has_value(), true);

    const auto & gas{aGas.value()};

    EXPECT_EQ(gas.Name, "Argon");
    const auto & gasProperties{gas.Properties};
    EXPECT_NEAR(gasProperties.MolecularWeight, 39.948, tolerance);
    testInterpolationCoefficients({0.002285, 0.00005149, 0}, gasProperties.Conductivity, tolerance);
    testInterpolationCoefficients({0.000003379, 6.451E-08, 0}, gasProperties.Viscosity, tolerance);
    testInterpolationCoefficients({521.9285, 0, 0}, gasProperties.SpecificHeat, tolerance);

    std::filesystem::remove(fileName);
}

TEST_F(TestGasesXMLReading, ReadGasByDisplayName)
{
    GasesLibrary::Gas gas;
    gas.Name = "Argon";
    gas.ProjectName = "Test Project";

    EXPECT_EQ(LibraryCommon::DisplayName(gas), "Test Project:Argon");
}

TEST_F(TestGasesXMLReading, ReadGasMixture_AirArgon)
{
    constexpr auto tolerance{1e-6};
    const std::string fileContent{TestGases::testDatabase()};
    const std::string fileName{"TestRead.xm"};

    File::createFileFromString(fileName, fileContent);

    GasesLibrary::DB gasesDB{fileName};
    const auto aGas{gasesDB.getGasByName("Air 10%/Argon 90%")};

    EXPECT_EQ(aGas.has_value(), true);

    const auto & gas{aGas.value()};

    EXPECT_EQ(gas.Name, "Air 10%/Argon 90%");
    constexpr auto correctNumberOfComponents{2u};
    EXPECT_EQ(correctNumberOfComponents, gas.Components.size());

    const std::vector<GasesLibrary::Component> correctComponents{{0.1, "Air"}, {0.9, "Argon"}};

    for(auto i{0u}; i < correctNumberOfComponents; ++i)
    {
        EXPECT_EQ(correctComponents[i].PureGasName, gas.Components[i].PureGasName);
        EXPECT_NEAR(correctComponents[i].Fraction, gas.Components[i].Fraction, tolerance);
    }

    std::filesystem::remove(fileName);
}

TEST_F(TestGasesXMLReading, ReadGasDataMixture_AirArgon)
{
    constexpr auto tolerance{1e-6};
    const std::string fileContent{TestGases::testDatabase()};
    const std::string fileName{"TestRead.xm"};

    File::createFileFromString(fileName, fileContent);

    GasesLibrary::DB gasesDB{fileName};
    const auto aGas{gasesDB.getGasDataByName("Air 10%/Argon 90%")};

    EXPECT_EQ(aGas.has_value(), true);

    const auto & gas{aGas.value()};

    EXPECT_EQ(gas.gas.Name, "Air 10%/Argon 90%");
    EXPECT_EQ(gas.gas.ProjectName, "");
    constexpr auto correctNumberOfComponents{2u};
    EXPECT_EQ(correctNumberOfComponents, gas.gas.Components.size());

    const std::vector<GasesLibrary::Component> correctComponents{{0.1, "Air"}, {0.9, "Argon"}};

    for(auto i{0u}; i < correctNumberOfComponents; ++i)
    {
        EXPECT_EQ(correctComponents[i].PureGasName, gas.gas.Components[i].PureGasName);
        EXPECT_NEAR(correctComponents[i].Fraction, gas.gas.Components[i].Fraction, tolerance);
    }

    EXPECT_EQ(gas.components.size(), correctNumberOfComponents);

    GasesLibrary::PureGas air;
    air.Name = "Air";
    air.Properties.MolecularWeight = 28.97;
    air.Properties.Conductivity = {0.002873, 0.0000776, 0};
    air.Properties.Viscosity = {0.000003723, 4.94E-08, 0};
    air.Properties.SpecificHeat = {1002.737, 0.012324, 0};

    GasesLibrary::PureGas argon;
    argon.Name = "Argon";
    argon.Properties.MolecularWeight = 39.948;
    argon.Properties.Conductivity = {0.002285, 0.00005149, 0};
    argon.Properties.Viscosity = {0.000003379, 6.451E-08, 0};
    argon.Properties.SpecificHeat = {521.9285, 0, 0};

    const std::vector<GasesLibrary::PureGas> correctPureGases{air, argon};

    for(auto i{0u}; i < correctNumberOfComponents; ++i)
    {
        EXPECT_EQ(correctPureGases[i].Name, gas.components[i]->Name);
        EXPECT_NEAR(
          correctPureGases[i].Properties.MolecularWeight, gas.components[i]->Properties.MolecularWeight, tolerance);
        testInterpolationCoefficients(
          correctPureGases[i].Properties.Conductivity, gas.components[i]->Properties.Conductivity, tolerance);
        testInterpolationCoefficients(
          correctPureGases[i].Properties.Viscosity, gas.components[i]->Properties.Viscosity, tolerance);
        testInterpolationCoefficients(
          correctPureGases[i].Properties.SpecificHeat, gas.components[i]->Properties.SpecificHeat, tolerance);
    }

    std::filesystem::remove(fileName);
}

TEST_F(TestGasesXMLReading, ReadGasMixture_AirArgonByUUID)
{
    constexpr auto tolerance{1e-6};
    const std::string fileContent{TestGases::testDatabase()};
    const std::string fileName{"TestRead.xm"};

    File::createFileFromString(fileName, fileContent);

    GasesLibrary::DB gasesDB{fileName};
    const auto aGas{gasesDB.getGasDataByUUID("37881919-7cdf-4428-bf5c-10702bc78415")};

    EXPECT_EQ(aGas.has_value(), true);

    const auto & gas{aGas.value()};

    EXPECT_EQ(gas.gas.Name, "Air 10%/Argon 90%");
    EXPECT_EQ(gas.gas.ProjectName, "");
    constexpr auto correctNumberOfComponents{2u};
    EXPECT_EQ(correctNumberOfComponents, gas.gas.Components.size());

    const std::vector<GasesLibrary::Component> correctComponents{{0.1, "Air"}, {0.9, "Argon"}};

    for(auto i{0u}; i < correctNumberOfComponents; ++i)
    {
        EXPECT_EQ(correctComponents[i].PureGasName, gas.gas.Components[i].PureGasName);
        EXPECT_NEAR(correctComponents[i].Fraction, gas.gas.Components[i].Fraction, tolerance);
    }

    EXPECT_EQ(gas.components.size(), correctNumberOfComponents);

    GasesLibrary::PureGas air;
    air.Name = "Air";
    air.Properties.MolecularWeight = 28.97;
    air.Properties.Conductivity = {0.002873, 0.0000776, 0};
    air.Properties.Viscosity = {0.000003723, 4.94E-08, 0};
    air.Properties.SpecificHeat = {1002.737, 0.012324, 0};

    GasesLibrary::PureGas argon;
    argon.Name = "Argon";
    argon.Properties.MolecularWeight = 39.948;
    argon.Properties.Conductivity = {0.002285, 0.00005149, 0};
    argon.Properties.Viscosity = {0.000003379, 6.451E-08, 0};
    argon.Properties.SpecificHeat = {521.9285, 0, 0};

    const std::vector<GasesLibrary::PureGas> correctPureGases{air, argon};

    for(auto i{0u}; i < correctNumberOfComponents; ++i)
    {
        EXPECT_EQ(correctPureGases[i].Name, gas.components[i]->Name);
        EXPECT_NEAR(
          correctPureGases[i].Properties.MolecularWeight, gas.components[i]->Properties.MolecularWeight, tolerance);
        testInterpolationCoefficients(
          correctPureGases[i].Properties.Conductivity, gas.components[i]->Properties.Conductivity, tolerance);
        testInterpolationCoefficients(
          correctPureGases[i].Properties.Viscosity, gas.components[i]->Properties.Viscosity, tolerance);
        testInterpolationCoefficients(
          correctPureGases[i].Properties.SpecificHeat, gas.components[i]->Properties.SpecificHeat, tolerance);
    }

    std::filesystem::remove(fileName);
}

TEST_F(TestGasesXMLReading, ReadGasMixture_AirArgonByDisplayName)
{
    constexpr auto tolerance{1e-6};
    const std::string fileContent{TestGases::testDatabase()};
    const std::string fileName{"TestRead.xm"};

    File::createFileFromString(fileName, fileContent);

    GasesLibrary::DB gasesDB{fileName};
    const auto aGas{gasesDB.getGasDataByDisplayName("Air 10%/Argon 90%")};

    EXPECT_EQ(aGas.has_value(), true);

    const auto & gas{aGas.value()};

    EXPECT_EQ(gas.gas.Name, "Air 10%/Argon 90%");
    EXPECT_EQ(gas.gas.ProjectName, "");
    constexpr auto correctNumberOfComponents{2u};
    EXPECT_EQ(correctNumberOfComponents, gas.gas.Components.size());

    const std::vector<GasesLibrary::Component> correctComponents{{0.1, "Air"}, {0.9, "Argon"}};

    for(auto i{0u}; i < correctNumberOfComponents; ++i)
    {
        EXPECT_EQ(correctComponents[i].PureGasName, gas.gas.Components[i].PureGasName);
        EXPECT_NEAR(correctComponents[i].Fraction, gas.gas.Components[i].Fraction, tolerance);
    }

    EXPECT_EQ(gas.components.size(), correctNumberOfComponents);

    GasesLibrary::PureGas air;
    air.Name = "Air";
    air.Properties.MolecularWeight = 28.97;
    air.Properties.Conductivity = {0.002873, 0.0000776, 0};
    air.Properties.Viscosity = {0.000003723, 4.94E-08, 0};
    air.Properties.SpecificHeat = {1002.737, 0.012324, 0};

    GasesLibrary::PureGas argon;
    argon.Name = "Argon";
    argon.Properties.MolecularWeight = 39.948;
    argon.Properties.Conductivity = {0.002285, 0.00005149, 0};
    argon.Properties.Viscosity = {0.000003379, 6.451E-08, 0};
    argon.Properties.SpecificHeat = {521.9285, 0, 0};

    const std::vector<GasesLibrary::PureGas> correctPureGases{air, argon};

    for(auto i{0u}; i < correctNumberOfComponents; ++i)
    {
        EXPECT_EQ(correctPureGases[i].Name, gas.components[i]->Name);
        EXPECT_NEAR(
          correctPureGases[i].Properties.MolecularWeight, gas.components[i]->Properties.MolecularWeight, tolerance);
        testInterpolationCoefficients(
          correctPureGases[i].Properties.Conductivity, gas.components[i]->Properties.Conductivity, tolerance);
        testInterpolationCoefficients(
          correctPureGases[i].Properties.Viscosity, gas.components[i]->Properties.Viscosity, tolerance);
        testInterpolationCoefficients(
          correctPureGases[i].Properties.SpecificHeat, gas.components[i]->Properties.SpecificHeat, tolerance);
    }

    std::filesystem::remove(fileName);
}

TEST_F(TestGasesXMLReading, ReadGasDataMixture_AirSF6)
{
    constexpr auto tolerance{1e-6};
    const std::string fileContent{TestGases::testDatabase()};
    const std::string fileName{"TestRead.xm"};

    File::createFileFromString(fileName, fileContent);

    GasesLibrary::DB gasesDB{fileName};
    const auto aGas{gasesDB.getGasDataByName("Air 10%/SF6 90%")};

    EXPECT_EQ(aGas.has_value(), true);

    const auto & gas{aGas.value()};

    EXPECT_EQ(gas.gas.Name, "Air 10%/SF6 90%");
    EXPECT_EQ(gas.gas.ProjectName, "");
    constexpr auto correctNumberOfComponents{2u};
    EXPECT_EQ(correctNumberOfComponents, gas.gas.Components.size());

    const std::vector<GasesLibrary::Component> correctComponents{{0.1, "Air"}, {0.9, "SF6"}};

    for(auto i{0u}; i < correctNumberOfComponents; ++i)
    {
        EXPECT_EQ(correctComponents[i].PureGasName, gas.gas.Components[i].PureGasName);
        EXPECT_NEAR(correctComponents[i].Fraction, gas.gas.Components[i].Fraction, tolerance);
    }

    EXPECT_EQ(gas.components.size(), correctNumberOfComponents);

    const std::vector<bool> correctComponentFound{true, false};

    for(auto i{0u}; i < correctNumberOfComponents; ++i)
    {
        EXPECT_EQ(correctComponentFound[i], gas.components[i].has_value());
    }

    std::filesystem::remove(fileName);
}

TEST_F(TestGasesXMLReading, AllGasesNames)
{
    const std::string fileContent{TestGases::testDatabase()};
    const std::string fileName{"TestRead.xm"};

    File::createFileFromString(fileName, fileContent);

    GasesLibrary::DB gasesDB{fileName};
    const auto gases{gasesDB.getGasNames()};

    const std::vector<std::string> correctGases{"Air 10%/Argon 90%", "Air 10%/SF6 90%"};

    EXPECT_EQ(correctGases.size(), gases.size());

    for(auto i{0u}; i < correctGases.size(); ++i)
    {
        EXPECT_EQ(correctGases[i], gases[i]);
    }

    std::filesystem::remove(fileName);
}

TEST_F(TestGasesXMLReading, AllGasesDisplayNames)
{
    const std::string fileContent{TestGases::testDatabase()};
    const std::string fileName{"TestRead.xm"};

    File::createFileFromString(fileName, fileContent);

    GasesLibrary::DB gasesDB{fileName};
    const auto gases{gasesDB.getGasDisplayNames()};

    const std::vector<std::string> correctGases{"Air 10%/Argon 90%", "Air 10%/SF6 90%"};

    EXPECT_EQ(correctGases.size(), gases.size());

    for(auto i{0u}; i < correctGases.size(); ++i)
    {
        EXPECT_EQ(correctGases[i], gases[i]);
    }

    std::filesystem::remove(fileName);
}

TEST_F(TestGasesXMLReading, AllPureComponentNames)
{
    const std::string fileContent{TestGases::testDatabase()};
    const std::string fileName{"TestRead.xm"};

    File::createFileFromString(fileName, fileContent);

    GasesLibrary::DB gasesDB{fileName};
    const auto pureComponents{gasesDB.getPureGasNames()};

    const std::vector<std::string> correctPureComponents{"Air", "Argon"};

    EXPECT_EQ(correctPureComponents.size(), pureComponents.size());

    for(auto i{0u}; i < correctPureComponents.size(); ++i)
    {
        EXPECT_EQ(correctPureComponents[i], pureComponents[i]);
    }

    std::filesystem::remove(fileName);
}

TEST_F(TestGasesXMLReading, AllGasesData)
{
    constexpr auto tolerance{1e-6};
    const std::string fileContent{TestGases::testDatabase()};
    const std::string fileName{"TestRead.xm"};

    File::createFileFromString(fileName, fileContent);

    GasesLibrary::DB gasesDB{fileName};
    const auto gases{gasesDB.getGasesData()};

    const std::vector<std::string> correctGases{"Air 10%/Argon 90%", "Air 10%/SF6 90%"};

    EXPECT_EQ(correctGases.size(), gases.size());

    for(auto i{0u}; i < correctGases.size(); ++i)
    {
        EXPECT_EQ(correctGases[i], gases[i].gas.Name);
    }

    const std::vector<std::vector<GasesLibrary::Component>> correctComponents{{{{0.1, "Air"}, {0.9, "Argon"}}},
                                                                              {{{0.1, "Air"}, {0.9, "SF6"}}}};

    for(auto i{0u}; i < correctGases.size(); ++i)
    {
        EXPECT_EQ(correctComponents[i].size(), gases[i].gas.Components.size());
        for(auto j{0u}; j < correctComponents[i].size(); ++j)
        {
            EXPECT_EQ(correctComponents[i][j].PureGasName, gases[i].gas.Components[j].PureGasName);
            EXPECT_NEAR(correctComponents[i][j].Fraction, gases[i].gas.Components[j].Fraction, tolerance);
        }
    }

    const std::vector<std::vector<bool>> correctComponentFound{{true, true}, {true, false}};
    for(auto i{0u}; i < correctGases.size(); ++i)
    {
        EXPECT_EQ(correctComponentFound[i].size(), gases[i].components.size());
        for(auto j{0u}; j < correctComponentFound[i].size(); ++j)
        {
            EXPECT_EQ(correctComponentFound[i][j], gases[i].components[j].has_value());
        }
    }

    std::filesystem::remove(fileName);
}