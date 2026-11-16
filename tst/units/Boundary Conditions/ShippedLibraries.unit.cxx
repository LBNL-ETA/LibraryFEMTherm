#include <filesystem>

#include <gtest/gtest.h>

#include "BoundaryConditions/DB.hxx"
#include "TimeSeriesData/DB.hxx"

//! The shipped default libraries under src/Schemas are release artifacts: they must stay
//! loadable by the current serializers. Round-trip tests cannot catch a drift between the
//! code's element names and the files on disk (they save and load through the same code),
//! so these tests load the actual shipped files.

namespace
{
    std::string schemasDir()
    {
        return std::filesystem::path(TEST_DATA_DIR).parent_path().append("src/Schemas").string();
    }
}   // namespace

TEST(TestShippedLibraries, TimeSeriesDefaultLibraryLoads)
{
    const auto libraryPath{schemasDir() + "/Time Series Data/TimeSeriesData.xml"};
    ASSERT_TRUE(std::filesystem::exists(libraryPath));

    TimeSeriesLibrary::DB database(libraryPath);

    const auto & records{database.getTimeSeriesData()};
    ASSERT_FALSE(records.empty());

    for(const auto & record : records)
    {
        EXPECT_FALSE(record.UUID.empty());
        EXPECT_FALSE(record.series.empty());
    }

    const auto minneapolis{database.getByName("USA_MN_Minneapolis-St.Paul.Intl.AP.726580_TMY3")};
    ASSERT_TRUE(minneapolis.has_value());
    EXPECT_TRUE(TimeSeriesLibrary::hasRole(minneapolis.value(),
                                           TimeSeriesLibrary::SeriesRole::AirTemperature));
}

TEST(TestShippedLibraries, BoundaryConditionDefaultLibraryLoads)
{
    const auto libraryPath{schemasDir() + "/Boundary Conditions/BoundaryConditions.xml"};
    ASSERT_TRUE(std::filesystem::exists(libraryPath));

    BCLibrary::DB database(libraryPath);

    const auto & records{database.getBoundaryConditions()};
    ASSERT_FALSE(records.empty());

    for(const auto & record : records)
    {
        EXPECT_FALSE(record.UUID.empty());
        EXPECT_FALSE(record.Name.empty());
    }
}
