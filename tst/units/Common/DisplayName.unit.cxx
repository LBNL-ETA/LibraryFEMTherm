#include <gtest/gtest.h>
#include <optional>
#include <string>
#include "LibraryUtilities/Common.hxx"

struct TestRecord
{
    std::string Name;
    std::optional<std::string> ProjectName;
};

TEST(DisplayNameTest, WithoutProjectName)
{
    const TestRecord rec{"MaterialA", std::nullopt};
    EXPECT_EQ(LibraryCommon::DisplayName(rec), "MaterialA");
}

TEST(DisplayNameTest, WithProjectName)
{
    const TestRecord rec{"MaterialB", "ProjectX"};
    EXPECT_EQ(LibraryCommon::DisplayName(rec), "ProjectX:MaterialB");
}

TEST(DisplayNameTest, WithEmptyProjectName)
{
    const TestRecord rec{"MaterialC", ""};
    EXPECT_EQ(LibraryCommon::DisplayName(rec), "MaterialC");
}