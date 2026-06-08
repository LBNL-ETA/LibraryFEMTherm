#include <algorithm>
#include <variant>

#include <gtest/gtest.h>

#include "Materials/Materials.hxx"
#include "Materials/Operators.hxx"

namespace
{
    constexpr auto uuid{"7a863ad6-c537-11ea-87d0-0242ac130003"};

    bool hasField(const std::vector<LibraryCommon::PropertyDifference> & diffs, const std::string & name)
    {
        return std::any_of(diffs.begin(), diffs.end(), [&](const auto & diff) { return diff.name == name; });
    }
}   // namespace

class TestMaterialDifferences : public testing::Test
{};

TEST_F(TestMaterialDifferences, EmptyWhenPhysicallyEqual)
{
    const auto lib = MaterialsLibrary::generate(uuid);
    const auto file = MaterialsLibrary::generate(uuid);

    ASSERT_TRUE(MaterialsLibrary::arePhysicalPropertiesEqual(lib, file));
    EXPECT_TRUE(MaterialsLibrary::physicalDifferences(lib, file).empty());
}

// The exact failure that shipped: a material differing only on Porosity was flagged as a
// conflict but produced no rows because the old hand-written diff did not check Porosity.
TEST_F(TestMaterialDifferences, PorosityDifferenceIsItemized)
{
    auto lib = MaterialsLibrary::generate(uuid);
    auto file = MaterialsLibrary::generate(uuid);
    ASSERT_TRUE(lib.data.hygroThermal.has_value());

    lib.data.hygroThermal->Porosity = 0.30;
    file.data.hygroThermal->Porosity = 0.60;

    const auto diffs = MaterialsLibrary::physicalDifferences(lib, file);
    EXPECT_FALSE(diffs.empty());
    EXPECT_TRUE(hasField(diffs, "Porosity"));
}

TEST_F(TestMaterialDifferences, RoughnessDifferenceIsItemized)
{
    auto lib = MaterialsLibrary::generate(uuid);
    auto file = MaterialsLibrary::generate(uuid);
    ASSERT_TRUE(lib.data.hygroThermal.has_value());

    lib.data.hygroThermal->Roughness = MaterialsLibrary::MaterialRoughness::VeryRough;
    file.data.hygroThermal->Roughness = MaterialsLibrary::MaterialRoughness::VerySmooth;

    EXPECT_TRUE(hasField(MaterialsLibrary::physicalDifferences(lib, file), "Roughness"));
}

TEST_F(TestMaterialDifferences, SolarOpticalDifferenceIsItemized)
{
    auto lib = MaterialsLibrary::generate(uuid);
    auto file = MaterialsLibrary::generate(uuid);
    ASSERT_TRUE(lib.data.optical.has_value() && lib.data.optical->integrated.has_value());

    lib.data.optical->integrated->Solar = std::nullopt;
    MaterialsLibrary::OpticalType solar;
    solar.Direct.Front.Transmittance = 0.6;
    file.data.optical->integrated->Solar = solar;

    // Presence mismatch drills into leaves with a stacked path; the absent side reads N/A.
    const auto diffs = MaterialsLibrary::physicalDifferences(lib, file);
    const auto row = std::find_if(
      diffs.begin(), diffs.end(), [](const auto & diff) { return diff.name == "Solar-Direct-Front-Transmittance"; });
    ASSERT_NE(row, diffs.end());
    EXPECT_TRUE(std::holds_alternative<std::monostate>(row->lhs));
    EXPECT_DOUBLE_EQ(std::get<double>(row->rhs), 0.6);
}

TEST_F(TestMaterialDifferences, ConductivityDifferenceCarriesRawValues)
{
    auto lib = MaterialsLibrary::generate(uuid);
    auto file = MaterialsLibrary::generate(uuid);
    ASSERT_TRUE(lib.data.hygroThermal.has_value());

    lib.data.hygroThermal->ThermalConductivityDry = 0.10;
    file.data.hygroThermal->ThermalConductivityDry = 0.25;

    const auto diffs = MaterialsLibrary::physicalDifferences(lib, file);
    const auto row = std::find_if(
      diffs.begin(), diffs.end(), [](const auto & diff) { return diff.name == "Thermal conductivity (dry)"; });
    ASSERT_NE(row, diffs.end());
    ASSERT_TRUE(std::holds_alternative<double>(row->lhs));
    EXPECT_DOUBLE_EQ(std::get<double>(row->lhs), 0.10);
    EXPECT_DOUBLE_EQ(std::get<double>(row->rhs), 0.25);
}

// The contract the whole feature relies on: a conflict (physically unequal) always yields at
// least one difference row, never an empty list. Shares the same field list as the equality
// check, so this cannot silently regress when a field is added.
TEST_F(TestMaterialDifferences, InvariantNonEmptyWhenPhysicallyDifferent)
{
    auto lib = MaterialsLibrary::generate(uuid);
    auto file = MaterialsLibrary::generate(uuid);
    ASSERT_TRUE(lib.data.hygroThermal.has_value());

    lib.data.hygroThermal->Porosity = 0.30;
    file.data.hygroThermal->Porosity = 0.60;

    ASSERT_FALSE(MaterialsLibrary::arePhysicalPropertiesEqual(lib, file));
    EXPECT_FALSE(MaterialsLibrary::physicalDifferences(lib, file).empty());
}
