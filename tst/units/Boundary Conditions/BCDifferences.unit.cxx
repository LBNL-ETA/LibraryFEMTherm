#include <algorithm>
#include <variant>

#include <gtest/gtest.h>

#include "BCSteadyState/SteadyState.hxx"
#include "BCSteadyState/Operators.hxx"

namespace
{
    bool hasField(const std::vector<LibraryCommon::PropertyDifference> & diffs, const std::string & name)
    {
        return std::any_of(diffs.begin(), diffs.end(), [&](const auto & diff) { return diff.name == name; });
    }
}   // namespace

class TestBCDifferences : public testing::Test
{};

TEST_F(TestBCDifferences, EmptyWhenPhysicallyEqual)
{
    const BCSteadyStateLibrary::BoundaryCondition lib;
    const BCSteadyStateLibrary::BoundaryCondition file;

    ASSERT_TRUE(BCSteadyStateLibrary::arePhysicalPropertiesEqual(lib, file));
    EXPECT_TRUE(BCSteadyStateLibrary::physicalDifferences(lib, file).empty());
}

TEST_F(TestBCDifferences, SimplifiedTemperatureCarriesRawValues)
{
    BCSteadyStateLibrary::Simplified lhsData;
    lhsData.temperature = 20.0;
    BCSteadyStateLibrary::Simplified rhsData;
    rhsData.temperature = 25.0;

    BCSteadyStateLibrary::BoundaryCondition lib;
    lib.data = lhsData;
    BCSteadyStateLibrary::BoundaryCondition file;
    file.data = rhsData;

    const auto diffs = BCSteadyStateLibrary::physicalDifferences(lib, file);
    const auto row =
      std::find_if(diffs.begin(), diffs.end(), [](const auto & diff) { return diff.name == "Temperature"; });
    ASSERT_NE(row, diffs.end());
    ASSERT_TRUE(std::holds_alternative<double>(row->lhs));
    EXPECT_DOUBLE_EQ(std::get<double>(row->lhs), 20.0);
    EXPECT_DOUBLE_EQ(std::get<double>(row->rhs), 25.0);
}

TEST_F(TestBCDifferences, DifferentTypeReportsTypeRow)
{
    BCSteadyStateLibrary::BoundaryCondition lib;
    lib.data = BCSteadyStateLibrary::Simplified{};
    BCSteadyStateLibrary::BoundaryCondition file;
    file.data = BCSteadyStateLibrary::RadiationSurface{};

    EXPECT_TRUE(hasField(BCSteadyStateLibrary::physicalDifferences(lib, file), "Type"));
}

// The radiation sub-variant was entirely missing from the old hand-written BC diff. Two
// comprehensive BCs differing only on radiation must still produce a row.
TEST_F(TestBCDifferences, RadiationDifferenceIsItemized)
{
    BCSteadyStateLibrary::Comprehensive withRadiation;
    BCSteadyStateLibrary::Radiation radiation;
    radiation.radiation = BCSteadyStateLibrary::BlackBodyRadiation{};
    withRadiation.radiation = radiation;

    BCSteadyStateLibrary::Comprehensive withoutRadiation;   // radiation stays nullopt

    BCSteadyStateLibrary::BoundaryCondition lib;
    lib.data = withRadiation;
    BCSteadyStateLibrary::BoundaryCondition file;
    file.data = withoutRadiation;

    ASSERT_FALSE(BCSteadyStateLibrary::arePhysicalPropertiesEqual(lib, file));
    EXPECT_FALSE(BCSteadyStateLibrary::physicalDifferences(lib, file).empty());
}

TEST_F(TestBCDifferences, InvariantNonEmptyWhenPhysicallyDifferent)
{
    BCSteadyStateLibrary::RadiationSurface lhsData;
    lhsData.emissivity = 0.84;
    BCSteadyStateLibrary::RadiationSurface rhsData;
    rhsData.emissivity = 0.90;

    BCSteadyStateLibrary::BoundaryCondition lib;
    lib.data = lhsData;
    BCSteadyStateLibrary::BoundaryCondition file;
    file.data = rhsData;

    ASSERT_FALSE(BCSteadyStateLibrary::arePhysicalPropertiesEqual(lib, file));
    EXPECT_FALSE(BCSteadyStateLibrary::physicalDifferences(lib, file).empty());
}
