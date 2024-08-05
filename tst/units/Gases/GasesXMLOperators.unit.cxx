#include "gtest/gtest.h"

#include "Gases/Gases.hxx"
#include "Gases/Operators.hxx"

class TestGasesOperators : public testing::Test
{
protected:
    void SetUp() override
    {}
};

TEST_F(TestGasesOperators, Coefficients)
{
    const GasesLibrary::Coefficients coefficients1{1, 2, 3};
    const GasesLibrary::Coefficients coefficients2{1, 2, 3};

    const bool result{coefficients1 == coefficients2};
    EXPECT_EQ(result, true);
}

TEST_F(TestGasesOperators, CoefficientsNotEqual)
{
    const GasesLibrary::Coefficients coefficients1{1, 2, 3};
    const GasesLibrary::Coefficients coefficients2{1, 2, 4};

    const bool result{coefficients1 == coefficients2};
    EXPECT_EQ(result, false);
}

TEST_F(TestGasesOperators, GasProperties)
{
    const GasesLibrary::GasProperties gasProperties1{1, 2, {3, 4, 5}, {6, 7, 8}, {9, 10, 11}};
    const GasesLibrary::GasProperties gasProperties2{1, 2, {3, 4, 5}, {6, 7, 8}, {9, 10, 11}};

    const bool result{gasProperties1 == gasProperties2};
    EXPECT_EQ(result, true);
}

TEST_F(TestGasesOperators, GasPropertiesNotEqual)
{
    const GasesLibrary::GasProperties gasProperties1{1, 2, {3, 4, 5}, {6, 7, 8}, {9, 10, 11}};
    const GasesLibrary::GasProperties gasProperties2{1, 2, {3, 4, 5}, {6, 7, 8}, {9, 10, 12}};

    const bool result{gasProperties1 == gasProperties2};
    EXPECT_EQ(result, false);
}

TEST_F(TestGasesOperators, GasCommon)
{
    const GasesLibrary::GasCommon gasCommon1{"uuid1", "Gas1", false};
    const GasesLibrary::GasCommon gasCommon2{"uuid1", "Gas1", false};

    const bool result{gasCommon1 == gasCommon2};

    EXPECT_EQ(result, true);
}

TEST_F(TestGasesOperators, GasCommonNotEqual)
{
    const GasesLibrary::GasCommon gasCommon1{"uuid1", "Gas1", false};
    const GasesLibrary::GasCommon gasCommon2{"uuid1", "Gas1", true};

    const bool result{gasCommon1 == gasCommon2};

    EXPECT_EQ(result, false);
}

TEST_F(TestGasesOperators, PureGas)
{
    const GasesLibrary::PureGas pureGas1{"uuid1", "Gas1", false, {1, 2, {3, 4, 5}, {6, 7, 8}, {9, 10, 11}}};
    const GasesLibrary::PureGas pureGas2{"uuid1", "Gas1", false, {1, 2, {3, 4, 5}, {6, 7, 8}, {9, 10, 11}}};

    const bool result{pureGas1 == pureGas2};

    EXPECT_EQ(result, true);
}

TEST_F(TestGasesOperators, PureGasNotEqual)
{
    const GasesLibrary::PureGas pureGas1{"uuid1", "Gas1", false, {1, 2, {3, 4, 5}, {6, 7, 8}, {9, 10, 11}}};
    const GasesLibrary::PureGas pureGas2{"uuid1", "Gas1", false, {1, 2, {3, 4, 5}, {6, 7, 8}, {9, 10, 12}}};

    const bool result{pureGas1 == pureGas2};

    EXPECT_EQ(result, false);
}

TEST_F(TestGasesOperators, Component)
{
    const GasesLibrary::Component component1{1, "Gas1"};
    const GasesLibrary::Component component2{1, "Gas1"};

    const bool result{component1 == component2};

    EXPECT_EQ(result, true);
}

TEST_F(TestGasesOperators, ComponentNotEqual)
{
    const GasesLibrary::Component component1{1, "Gas1"};
    const GasesLibrary::Component component2{1, "Gas2"};

    const bool result{component1 == component2};

    EXPECT_EQ(result, false);
}

TEST_F(TestGasesOperators, Gas)
{
    const GasesLibrary::Gas gas1{"uuid1", "Gas1", false, {{0.1, "Air"}, {0.9, "Argon"}}};
    const GasesLibrary::Gas gas2{"uuid1", "Gas1", false, {{0.1, "Air"}, {0.9, "Argon"}}};

    const bool result{gas1 == gas2};

    EXPECT_EQ(result, true);
}

TEST_F(TestGasesOperators, GasNotEqual)
{
    const GasesLibrary::Gas gas1{"uuid1", "Gas1", false, {{0.1, "Air"}, {0.9, "Argon"}}};
    const GasesLibrary::Gas gas2{"uuid1", "Gas1", false, {{0.1, "Air"}, {0.9, "Helium"}}};

    const bool result{gas1 == gas2};

    EXPECT_EQ(result, false);
}

TEST_F(TestGasesOperators, GasesData)
{
    const GasesLibrary::Gas gas1{"uuid1", "Gas1", false, {{0.1, "Air"}, {0.9, "Argon"}}};
    const GasesLibrary::Gas gas2{"uuid1", "Gas1", false, {{0.1, "Air"}, {0.9, "Argon"}}};
    const GasesLibrary::PureGas pureGas1{"uuid1", "Gas1", false, {1, 2, {3, 4, 5}, {6, 7, 8}, {9, 10, 11}}};
    const GasesLibrary::PureGas pureGas2{"uuid1", "Gas1", false, {1, 2, {3, 4, 5}, {6, 7, 8}, {9, 10, 11}}};

    const GasesLibrary::GasesData gasesData1{gas1, {pureGas1, pureGas2}};
    const GasesLibrary::GasesData gasesData2{gas2, {pureGas1, pureGas2}};

    const bool result{gasesData1 == gasesData2};

    EXPECT_EQ(result, true);
}

TEST_F(TestGasesOperators, GasesDataNotEqual)
{
    const GasesLibrary::Gas gas1{"uuid1", "Gas1", false, {{0.1, "Air"}, {0.9, "Argon"}}};
    const GasesLibrary::Gas gas2{"uuid1", "Gas1", false, {{0.1, "Air"}, {0.9, "SF6"}}};
    const GasesLibrary::PureGas pureGas1{"uuid1", "Gas1", false, {1, 2, {3, 4, 5}, {6, 7, 8}, {9, 10, 11}}};
    const GasesLibrary::PureGas pureGas2{"uuid1", "Gas1", false, {1, 2, {3, 4, 5}, {6, 7, 8}, {9, 10, 11}}};

    const GasesLibrary::GasesData gasesData1{gas1, {pureGas1, pureGas2}};
    const GasesLibrary::GasesData gasesData2{gas2, {pureGas1, pureGas2}};

    const bool result{gasesData1 == gasesData2};

    EXPECT_EQ(result, false);
}

TEST_F(TestGasesOperators, GasesDataEqualWithoutUUID)
{
    const GasesLibrary::Gas gas1{"uuid1", "Gas1", false, {{0.1, "Air"}, {0.9, "Argon"}}};
    const GasesLibrary::Gas gas2{"uuid2", "Gas1", false, {{0.1, "Air"}, {0.9, "Argon"}}};
    const GasesLibrary::PureGas pureGas1{"uuid1", "Gas1", false, {1, 2, {3, 4, 5}, {6, 7, 8}, {9, 10, 11}}};
    const GasesLibrary::PureGas pureGas2{"uuid1", "Gas1", false, {1, 2, {3, 4, 5}, {6, 7, 8}, {9, 10, 11}}};

    const GasesLibrary::GasesData gasesData1{gas1, {pureGas1, pureGas2}};
    const GasesLibrary::GasesData gasesData2{gas2, {pureGas1, pureGas2}};

    const bool result{arePhysicalPropertiesEqual(gasesData1, gasesData2)};

    EXPECT_EQ(result, true);
}

TEST_F(TestGasesOperators, GasesDataEqualWithoutUUID_NotEqual)
{
    const GasesLibrary::Gas gas1{"uuid1", "Gas1", false, {{0.1, "Air"}, {0.9, "Argon"}}};
    const GasesLibrary::Gas gas2{"uuid2", "Gas1", false, {{0.1, "Air"}, {0.9, "Argon"}}};
    const GasesLibrary::PureGas pureGas1{"uuid1", "Gas1", false, {1, 2, {3, 4, 5}, {6, 7, 8}, {9, 10, 12}}};
    const GasesLibrary::PureGas pureGas2{"uuid1", "Gas1", false, {1, 2, {3, 4, 5}, {6, 7, 8}, {9, 10, 11}}};

    const GasesLibrary::GasesData gasesData1{gas1, {pureGas1, pureGas2}};
    const GasesLibrary::GasesData gasesData2{gas2, {pureGas1, pureGas1}};

    const bool result{arePhysicalPropertiesEqual(gasesData1, gasesData2)};

    EXPECT_EQ(result, false);
}