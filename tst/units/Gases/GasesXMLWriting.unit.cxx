#include <filesystem>
#include <gtest/gtest.h>

#include "Gases/Gases.hxx"
#include "Gases/DB.hxx"

#include "LibraryUtilities/FileManipulation.hxx"

class TestGasesXMLWriting : public testing::Test
{
};

std::string correctPureGasesTestXML()
{
    return "\xEF\xBB\xBF<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
           "<Gases>\n"
           "\t<Version>1</Version>\n"
           "\t<PureGas>\n"
           "\t\t<UUID>Test UUID</UUID>\n"
           "\t\t<Name>Test name</Name>\n"
           "\t\t<Protected>false</Protected>\n"
           "\t\t<Properties>\n"
           "\t\t\t<MolecularWeight>1</MolecularWeight>\n"
           "\t\t\t<SpecificHeatRatio>1</SpecificHeatRatio>\n"
           "\t\t\t<Conductivity>\n"
           "\t\t\t\t<A>1</A>\n"
           "\t\t\t\t<B>2</B>\n"
           "\t\t\t\t<C>3</C>\n"
           "\t\t\t</Conductivity>\n"
           "\t\t\t<Viscosity>\n"
           "\t\t\t\t<A>4</A>\n"
           "\t\t\t\t<B>5</B>\n"
           "\t\t\t\t<C>6</C>\n"
           "\t\t\t</Viscosity>\n"
           "\t\t\t<SpecificHeat>\n"
           "\t\t\t\t<A>7</A>\n"
           "\t\t\t\t<B>8</B>\n"
           "\t\t\t\t<C>9</C>\n"
           "\t\t\t</SpecificHeat>\n"
           "\t\t</Properties>\n"
           "\t</PureGas>\n"
           "</Gases>\n";
}

TEST_F(TestGasesXMLWriting, WritePureGasesXML)
{
    GasesLibrary::PureGas pureGas;
    pureGas.Name = "Test name";
    pureGas.UUID = "Test UUID";
    pureGas.Properties.MolecularWeight = 1.0;
    pureGas.Properties.SpecificHeatRatio = 1.0;
    pureGas.Properties.Conductivity = {1, 2, 3};
    pureGas.Properties.Viscosity = {4, 5, 6};
    pureGas.Properties.SpecificHeat = {7, 8, 9};

    const std::string fileName{"TestWrite.xml"};
    std::filesystem::remove(fileName);
    GasesLibrary::DB gasesDB{fileName};

    gasesDB.addPureGas(pureGas);

    const auto error{gasesDB.saveToFile()};

    EXPECT_EQ(error, 0);

    const auto test{File::loadToString(fileName)};

    EXPECT_EQ(test, correctPureGasesTestXML());

    std::filesystem::remove(fileName);
}

TEST_F(TestGasesXMLWriting, WriteGasMixtureXML)
{
    GasesLibrary::Gas gas;

    const std::string correctName{"Test name"};
    const std::string correctUUID{"Test UUID"};

    gas.Name = correctName;
    gas.UUID = correctUUID;
    gas.Components.push_back({0.1, "Air"});
    gas.Components.push_back({0.9, "Argon"});

    const std::string fileName{"TestWrite.xm"};
    std::filesystem::remove(fileName);
    GasesLibrary::DB gasesDB{fileName};

    gasesDB.addGas(gas);

    const auto saveError{gasesDB.saveToFile()};

    EXPECT_EQ(saveError, 0);

    GasesLibrary::DB loadedDB{fileName};

    auto loadedGas = loadedDB.getGasByUUID(correctUUID);

    ASSERT_EQ(true, loadedGas.has_value());
    EXPECT_EQ(correctName, loadedGas->Name);
    EXPECT_EQ(correctUUID, loadedGas->UUID);

    const std::vector<GasesLibrary::Component> correct {{0.1, "Air"}, {0.9, "Argon"}};
    ASSERT_EQ(correct.size(), loadedGas->Components.size());

    constexpr auto tolerance{1e-6};
    for(size_t i = 0u; i < correct.size(); i++)
    {
        EXPECT_NEAR(correct[i].Fraction, loadedGas->Components[i].Fraction, tolerance);
        EXPECT_EQ(correct[i].PureGasName, loadedGas->Components[i].PureGasName);
    }

    std::filesystem::remove(fileName);
}