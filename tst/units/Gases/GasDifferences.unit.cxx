#include <algorithm>
#include <variant>

#include <gtest/gtest.h>

#include "Gases/Gases.hxx"
#include "Gases/Operators.hxx"

namespace
{
    bool hasField(const std::vector<LibraryCommon::PropertyDifference> & diffs, const std::string & name)
    {
        return std::any_of(diffs.begin(), diffs.end(), [&](const auto & diff) { return diff.name == name; });
    }

    GasesLibrary::GasesData singleComponentAir()
    {
        GasesLibrary::GasesData data;
        GasesLibrary::PureGas air;
        air.Name = "Air";
        air.Properties.MolecularWeight = 28.97;
        air.Properties.Conductivity = {0.0, 0.0, 0.0};
        data.components.push_back(air);
        data.gas.Components.push_back({.Fraction = 1.0, .PureGasName = "Air"});
        return data;
    }
}   // namespace

class TestGasDifferences : public testing::Test
{};

TEST_F(TestGasDifferences, EmptyWhenPhysicallyEqual)
{
    const auto lib = singleComponentAir();
    const auto file = singleComponentAir();

    ASSERT_TRUE(GasesLibrary::arePhysicalPropertiesEqual(lib, file));
    EXPECT_TRUE(GasesLibrary::physicalDifferences(lib, file).empty());
}

TEST_F(TestGasDifferences, ComponentPropertyDifferenceIsItemized)
{
    auto lib = singleComponentAir();
    auto file = singleComponentAir();
    file.components[0]->Properties.MolecularWeight = 40.0;

    const auto diffs = GasesLibrary::physicalDifferences(lib, file);
    EXPECT_FALSE(diffs.empty());
    EXPECT_TRUE(hasField(diffs, "Air-Molecular weight"));
}

// Coefficients are nested sub-records; the diff must reach into them (and respect the tighter
// 1e-9 tolerance on the B coefficient via the shared field list).
TEST_F(TestGasDifferences, ConductivityCoefficientDifferenceIsItemized)
{
    auto lib = singleComponentAir();
    auto file = singleComponentAir();
    file.components[0]->Properties.Conductivity.A = 0.0241;

    const auto diffs = GasesLibrary::physicalDifferences(lib, file);
    EXPECT_TRUE(hasField(diffs, "Air-Conductivity-A"));
}

TEST_F(TestGasDifferences, InvariantNonEmptyWhenPhysicallyDifferent)
{
    auto lib = singleComponentAir();
    auto file = singleComponentAir();
    file.components[0]->Properties.MolecularWeight = 40.0;

    ASSERT_FALSE(GasesLibrary::arePhysicalPropertiesEqual(lib, file));
    EXPECT_FALSE(GasesLibrary::physicalDifferences(lib, file).empty());
}
