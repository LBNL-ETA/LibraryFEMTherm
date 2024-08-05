#include <gtest/gtest.h>

#include "LibraryUtilities/StringRoutines.hxx"

class TestTrimming : public testing::Test
{
protected:

    void SetUp() override
    {}
};

TEST_F(TestTrimming, LeftTrim)
{
    const std::string testString{" test text "};

    const auto result{StringRoutines::ltrim(testString)};
    EXPECT_EQ(result, "test text ");
}

TEST_F(TestTrimming, RightTrim)
{
    const std::string testString{" test text "};

    const auto result{StringRoutines::rtrim(testString)};
    EXPECT_EQ(result, " test text");
}

TEST_F(TestTrimming, Trim)
{
    const std::string testString{" test text "};

    const auto result{StringRoutines::trim(testString)};
    EXPECT_EQ(result, "test text");
}