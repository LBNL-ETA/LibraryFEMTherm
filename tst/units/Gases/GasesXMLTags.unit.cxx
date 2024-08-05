#include <gtest/gtest.h>

#include "Gases/Gases.hxx"

class TestGasesXMLTags : public testing::Test
{
protected:

    void SetUp() override
    {}
};

TEST_F(TestGasesXMLTags, SolidTag)
{
    GasesLibrary::Tags tag;
    const auto result{tag.gases()};
    const auto expected{"Gases"};

    EXPECT_EQ(expected, result);
}

TEST_F(TestGasesXMLTags, UUIDTag)
{
    GasesLibrary::Tags tag;
    const auto result{tag.UUID()};
    const auto expected{"UUID"};

    EXPECT_EQ(expected, result);
}

TEST_F(TestGasesXMLTags, NameTag)
{
    GasesLibrary::Tags tag;
    const auto result{tag.name()};
    const auto expected{"Name"};

    EXPECT_EQ(expected, result);
}

TEST_F(TestGasesXMLTags, PurgeGasTag)
{
    GasesLibrary::Tags tag;
    const auto result{tag.pureGas()};
    const auto expected{"PureGas"};

    EXPECT_EQ(expected, result);
}

TEST_F(TestGasesXMLTags, MolecularWeightTag)
{
    GasesLibrary::Tags tag;
    const auto result{tag.molecularWeight()};
    const auto expected{"MolecularWeight"};

    EXPECT_EQ(expected, result);
}

TEST_F(TestGasesXMLTags, ConductivityTag)
{
    GasesLibrary::Tags tag;
    const auto result{tag.conductivity()};
    const auto expected{"Conductivity"};

    EXPECT_EQ(expected, result);
}

TEST_F(TestGasesXMLTags, ViscosityTag)
{
    GasesLibrary::Tags tag;
    const auto result{tag.viscosity()};
    const auto expected{"Viscosity"};

    EXPECT_EQ(expected, result);
}

TEST_F(TestGasesXMLTags, SpecificHeatTag)
{
    GasesLibrary::Tags tag;
    const auto result{tag.specificHeat()};
    const auto expected{"SpecificHeat"};

    EXPECT_EQ(expected, result);
}

TEST_F(TestGasesXMLTags, ATag)
{
    GasesLibrary::Tags tag;
    const auto result{tag.A()};
    const auto expected{"A"};

    EXPECT_EQ(expected, result);
}

TEST_F(TestGasesXMLTags, BTag)
{
    GasesLibrary::Tags tag;
    const auto result{tag.B()};
    const auto expected{"B"};

    EXPECT_EQ(expected, result);
}

TEST_F(TestGasesXMLTags, CTag)
{
    GasesLibrary::Tags tag;
    const auto result{tag.C()};
    const auto expected{"C"};

    EXPECT_EQ(expected, result);
}