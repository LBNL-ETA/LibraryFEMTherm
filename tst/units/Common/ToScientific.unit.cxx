#include "gtest/gtest.h"

#include "LibraryUtilities/Util.hxx"

class ToScientificTest : public testing::Test
{
protected:
    void SetUp() override
    {}
};

TEST(ToScientificTest, PositiveNumber)
{
    double value = 12345.6789;
    std::string result = Util::toScientific(value);
    EXPECT_EQ("1.234568e+04", result);
}

TEST(ToScientificTest, NegativeNumber)
{
    double value = -12345.6789;
    std::string result = Util::toScientific(value);
    EXPECT_EQ("-1.234568e+04", result);
}

TEST(ToScientificTest, Zero)
{
    double value = 0.0;
    std::string result = Util::toScientific(value);
    EXPECT_EQ("0.000000e+00", result);
}

TEST(ToScientificTest, SmallNumber)
{
    double value = 0.0000123456789;
    std::string result = Util::toScientific(value);
    EXPECT_EQ("1.234568e-05", result);
}

TEST(ToScientificTest, LargeNumber)
{
    double value = 1.23456789e+20;
    std::string result = Util::toScientific(value);
    EXPECT_EQ("1.234568e+20", result);
}