#include <gtest/gtest.h>

#include "Legacy/Step1/BCTransient/Types.hxx"

class TestBCTypeLibraryLookup : public testing::Test
{};

TEST_F(TestBCTypeLibraryLookup, GetByDisplayName)
{
    BCTypesLibrary::DB bcLibraryDB;

    BCTypesLibrary::TypeRecord permanent{"uuid-permanent"};
    permanent.Name = "Interior";
    bcLibraryDB.add(permanent);

    BCTypesLibrary::TypeRecord temporary{"uuid-temporary"};
    temporary.Name = "Interior";
    temporary.ProjectName = "Project";
    bcLibraryDB.add(temporary);

    const auto plain{bcLibraryDB.getByDisplayName("Interior")};
    ASSERT_TRUE(plain.has_value());
    EXPECT_EQ("uuid-permanent", plain->UUID);

    const auto decorated{bcLibraryDB.getByDisplayName("Project:Interior")};
    ASSERT_TRUE(decorated.has_value());
    EXPECT_EQ("uuid-temporary", decorated->UUID);

    EXPECT_FALSE(bcLibraryDB.getByDisplayName("Missing").has_value());
}
