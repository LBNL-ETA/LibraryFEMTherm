#include "gtest/gtest.h"

#include <fileParse/Common.hxx>

#include "LibraryUtilities/StringRoutines.hxx"

#include "Common/Common.hxx"
#include "LibraryUtilities/TimeRoutines.hxx"

#include "MockNodeAdapter.hxx"

class TestStringConversion : public testing::Test
{
protected:
    void SetUp() override
    {}
};

TEST_F(TestStringConversion, ToUpper)
{
    const std::string value{"sOme smAll teXT"};

    auto result{StringRoutines::toUpper(value)};

    EXPECT_EQ(result, "SOME SMALL TEXT");
}

TEST_F(TestStringConversion, ToLower)
{
    const std::string value{"sOme smAll teXT"};

    auto result{StringRoutines::toLower(value)};

    EXPECT_EQ(result, "some small text");
}

TEST_F(TestStringConversion, TextToDecimalDouble)
{
    const std::string value{"0.123"};
    const auto result{StringRoutines::to_double(value)};
    EXPECT_EQ(0.123, result);
}

TEST_F(TestStringConversion, TextToScientificDouble)
{
    const std::string value{"5e-12"};
    const auto result{StringRoutines::to_double(value)};
    EXPECT_EQ(5e-12, result);
}

TEST_F(TestStringConversion, DecimalDoubleToString)
{
    const double value{0.123};
    const auto result{StringRoutines::to_string(value)};
    EXPECT_EQ("0.123", result);
}

TEST_F(TestStringConversion, ScientificDoubleToString)
{
    const double value{5e-12};
    const auto result{StringRoutines::to_string(value)};
    EXPECT_EQ("5e-12", result);
}

TEST_F(TestStringConversion, TimeToString)
{
    std::string value{"2023-12-31 23:59:59"};

    Helper::MockNode elementNode("TestNode");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << FileParse::Child{"DateTime", value};

    auto correctNodes = []() {
        Helper::MockNode node{"TestNode"};

        addChildNode(node, "DateTime", "2023-12-31 23:59:59");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestStringConversion, TimeFromString)
{
    auto mockData = []() {
        Helper::MockNode node{"TestNode"};

        addChildNode(node, "DateTime", "2023-12-31 23:59:59");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    std::string timePoint;
    adapter >> FileParse::Child{"DateTime", timePoint};

    EXPECT_EQ(timePoint, "2023-12-31 23:59:59");
}
