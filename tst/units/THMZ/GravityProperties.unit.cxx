#include <gtest/gtest.h>

#include "THMZ/Properties/Properties.hxx"
#include "THMZ/Properties/GravityProperties.hxx"

class TestGravityProperties : public testing::Test
{
protected:
    static void
      assert_near(Geometry::GravityVector expectedVec, Geometry::GravityVector actualVec, double tolerance = 1e-6)
    {
        EXPECT_NEAR(expectedVec.x, actualVec.x, tolerance);
        EXPECT_NEAR(expectedVec.y, actualVec.y, tolerance);
        EXPECT_NEAR(expectedVec.z, actualVec.z, tolerance);
    }
};

TEST_F(TestGravityProperties, OrientationStringsTest)
{
    auto strings = ThermFile::Gravity::orientationStrings();
    std::vector<std::string> expectedStrings = {
      "Down", "Up", "Left", "Right", "Into Screen", "Out Of Screen"};

    ASSERT_EQ(strings.size(), expectedStrings.size()) << "The number of orientation strings does not match expected.";
    for(const auto & str : expectedStrings)
    {
        auto it = std::find(strings.begin(), strings.end(), str);
        ASSERT_NE(it, strings.end()) << "Expected string " << str << " was not found in orientation strings.";
    }
}

// Tests for orientationFromString() function
TEST_F(TestGravityProperties, OrientationFromStringTest)
{
    ASSERT_EQ(ThermFile::Gravity::orientationFromString("Down"), ThermFile::Gravity::Orientation::Down);
    ASSERT_EQ(ThermFile::Gravity::orientationFromString("Up"), ThermFile::Gravity::Orientation::Up);
    ASSERT_EQ(ThermFile::Gravity::orientationFromString("Left"), ThermFile::Gravity::Orientation::Left);
    ASSERT_EQ(ThermFile::Gravity::orientationFromString("Right"), ThermFile::Gravity::Orientation::Right);
    ASSERT_EQ(ThermFile::Gravity::orientationFromString("Into Screen"),
              ThermFile::Gravity::Orientation::IntoScreen);
    ASSERT_EQ(ThermFile::Gravity::orientationFromString("Out Of Screen"),
              ThermFile::Gravity::Orientation::OutOfScreen);
}

// Tests for orientationToString() function
TEST_F(TestGravityProperties, OrientationToStringTest)
{
    ASSERT_EQ(ThermFile::Gravity::orientationToString(ThermFile::Gravity::Orientation::Down), "Down");
    ASSERT_EQ(ThermFile::Gravity::orientationToString(ThermFile::Gravity::Orientation::Up), "Up");
    ASSERT_EQ(ThermFile::Gravity::orientationToString(ThermFile::Gravity::Orientation::Left), "Left");
    ASSERT_EQ(ThermFile::Gravity::orientationToString(ThermFile::Gravity::Orientation::Right), "Right");
    ASSERT_EQ(ThermFile::Gravity::orientationToString(ThermFile::Gravity::Orientation::IntoScreen), "Into Screen");
    ASSERT_EQ(ThermFile::Gravity::orientationToString(ThermFile::Gravity::Orientation::OutOfScreen), "Out Of Screen");
}

// Test for invalid orientation to string conversion
TEST_F(TestGravityProperties, InvalidOrientationToString)
{
    auto result = ThermFile::Gravity::orientationToString(
      static_cast<ThermFile::Gravity::Orientation>(999));   // Using an out-of-range enum value
    EXPECT_EQ(result, "Unknown");
}

TEST_F(TestGravityProperties, GravityVectorUp)
{
    Geometry::GravityVector expectedVector{0, 1, 0};
    auto vector = getGravityVector(ThermFile::Gravity::Orientation::Up);
    assert_near(vector, expectedVector, 1e-6);
}

TEST_F(TestGravityProperties, GravityVectorDown)
{
    Geometry::GravityVector expectedVector{0, -1, 0};
    auto vector = getGravityVector(ThermFile::Gravity::Orientation::Down);
    assert_near(vector, expectedVector, 1e-6);
}

TEST_F(TestGravityProperties, GravityVectorRight)
{
    Geometry::GravityVector expectedVector{1, 0, 0};
    auto vector = getGravityVector(ThermFile::Gravity::Orientation::Right);
    assert_near(vector, expectedVector, 1e-6);
}

TEST_F(TestGravityProperties, GravityVectorLeft)
{
    Geometry::GravityVector expectedVector{-1, 0, 0};
    auto vector = getGravityVector(ThermFile::Gravity::Orientation::Left);
    assert_near(vector, expectedVector, 1e-6);
}

TEST_F(TestGravityProperties, GravityVectorIntoScreen)
{
    Geometry::GravityVector expectedVector{0, 0, -1};
    auto vector = getGravityVector(ThermFile::Gravity::Orientation::IntoScreen);
    assert_near(vector, expectedVector, 1e-6);
}

TEST_F(TestGravityProperties, GravityVectorOutOfScreen)
{
    Geometry::GravityVector expectedVector{0, 0, 1};
    auto vector = getGravityVector(ThermFile::Gravity::Orientation::OutOfScreen);
    assert_near(vector, expectedVector, 1e-6);
}

TEST_F(TestGravityProperties, GravityVectorUndefinedOrientation)
{
    Geometry::GravityVector expectedVector{0, -1, 0};   // Default to Down
    auto vector = ThermFile::Gravity::getGravityVector(static_cast<ThermFile::Gravity::Orientation>(999));
    assert_near(vector, expectedVector, 1e-6);
}
