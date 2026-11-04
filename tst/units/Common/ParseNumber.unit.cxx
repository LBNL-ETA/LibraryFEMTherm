#include <cmath>
#include <string_view>

#include <gtest/gtest.h>

#include "Common/ParseNumber.hxx"

using Common::parseDouble;
using Common::wholeDouble;

TEST(ParseNumber, ReadsPlainNumbers)
{
    EXPECT_DOUBLE_EQ(wholeDouble("1.5").value(), 1.5);
    EXPECT_DOUBLE_EQ(wholeDouble("-273.15").value(), -273.15);
    EXPECT_DOUBLE_EQ(wholeDouble(".5").value(), 0.5);
    EXPECT_DOUBLE_EQ(wholeDouble("2.5e-3").value(), 2.5e-3);
    EXPECT_DOUBLE_EQ(wholeDouble("0").value(), 0.0);
}

TEST(ParseNumber, RefusesWhatIsNotOneWholeNumber)
{
    EXPECT_FALSE(wholeDouble("").has_value());
    EXPECT_FALSE(wholeDouble("abc").has_value());
    EXPECT_FALSE(wholeDouble("1.5abc").has_value());
    EXPECT_FALSE(wholeDouble("1,5").has_value());
    EXPECT_FALSE(wholeDouble("1.5 ").has_value());
    EXPECT_FALSE(wholeDouble(" 1.5").has_value());
    EXPECT_FALSE(wholeDouble("+1.5").has_value());
}

TEST(ParseNumber, RefusesANumberThatDoesNotFit)
{
    EXPECT_FALSE(wholeDouble("1e999").has_value());
}

TEST(ParseNumber, ReadsNotANumberAsAValue)
{
    EXPECT_TRUE(std::isnan(wholeDouble("nan").value()));
}

TEST(ParseNumber, ReadsTheTextWithoutATerminator)
{
    const std::string_view longer{"1.5999"};

    EXPECT_DOUBLE_EQ(wholeDouble(longer.substr(0, 3)).value(), 1.5);
}

TEST(ParseNumber, StopsAfterTheNumberAndSaysWhere)
{
    const std::string_view line{"12.5,3"};

    const auto parsed{parseDouble(line.data(), line.data() + line.size())};

    ASSERT_TRUE(parsed.has_value());
    EXPECT_DOUBLE_EQ(parsed->value, 12.5);
    EXPECT_EQ(parsed->next, line.data() + 4);
}

TEST(ParseNumber, WalksAWholeRow)
{
    const std::string_view line{"1,-2.5,3e2"};
    const char * cursor{line.data()};
    const char * const last{line.data() + line.size()};

    const auto first{parseDouble(cursor, last)};
    const auto second{parseDouble(first->next + 1, last)};
    const auto third{parseDouble(second->next + 1, last)};

    EXPECT_DOUBLE_EQ(first->value, 1.0);
    EXPECT_DOUBLE_EQ(second->value, -2.5);
    EXPECT_DOUBLE_EQ(third->value, 300.0);
    EXPECT_EQ(third->next, last);
}
