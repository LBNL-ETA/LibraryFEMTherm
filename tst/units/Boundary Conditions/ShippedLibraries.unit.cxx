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
        EXPECT_FALSE(record.channels.empty());
    }

    const auto ashraeInside{database.getByName("ASHRAEInside")};
    ASSERT_TRUE(ashraeInside.has_value());
    EXPECT_TRUE(TimeSeriesLibrary::hasRole(ashraeInside.value(),
                                           TimeSeriesLibrary::ChannelRole::AirTemperature));
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
