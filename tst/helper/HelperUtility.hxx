#pragma once

#include <set>
#include <chrono>
#include <ctime>

namespace Helper
{
    template<typename T>
    inline bool areSetsEqual(const std::set<T, std::less<>> & set1, const std::set<T, std::less<>> & set2)
    {
        if(set1.size() != set2.size())
        {
            return false;
        }
        return std::equal(set1.begin(), set1.end(), set2.begin());
    }

    template<typename T>
    inline void
      assertOptionalNear(const std::optional<T> & correct, const std::optional<T> & calculated, double tolerance)
    {
        EXPECT_EQ(calculated.has_value(), correct.has_value());
        if(correct.has_value() && calculated.has_value())
        {
            EXPECT_NEAR(correct.value(), calculated.value(), tolerance);
        }
    }

    template<typename T>
    inline void assertOptionalNear(const T & correct, const std::optional<T> & calculated, double tolerance)
    {
        ASSERT_TRUE(calculated.has_value());
        if(calculated.has_value())
        {
            EXPECT_NEAR(correct, calculated.value(), tolerance);
        }
    }

    // Overload for comparing a non-optional value with std::optional (with tolerance for floating-point numbers)
    template<typename T>
    inline void assertOptionalNear(const T & expected, const T & actual, double tolerance)
    {
        EXPECT_NEAR(expected, actual, tolerance);
    }

    template<typename T>
    inline void assertOptionalEq(const std::optional<T> & correct, const std::optional<T> & calculated)
    {
        EXPECT_EQ(calculated.has_value(), correct.has_value());
        if(correct.has_value() && calculated.has_value())
        {
            EXPECT_EQ(correct.value(), calculated.value());
        }
    }

    template<typename T>
    inline void assertOptionalEq(const T & correct, const std::optional<T> & calculated)
    {
        ASSERT_TRUE(calculated.has_value());
        if(calculated.has_value())
        {
            assertOptionalEq(correct, calculated.value());
        }
    }

    template<typename T>
    inline void assertOptionalEq(const T & expected, const T & actual)
    {
        EXPECT_EQ(expected, actual);
    }

    inline void assertOptionalEq(const std::wstring & expected, const std::optional<std::wstring> & actual)
    {
        ASSERT_TRUE(actual.has_value());
        EXPECT_EQ(expected, actual.value());
    }

    // Overload for comparing two wstring values directly
    inline void assertOptionalEq(const std::wstring & expected, const std::wstring & actual)
    {
        EXPECT_EQ(expected, actual);
    }

    template<typename T>
    void checkVectorValues(const std::vector<T> & correct, const std::vector<T> & values, T tolerance)
    {
        ASSERT_EQ(correct.size(), values.size())
          << "Vectors have different sizes: expected " << correct.size() << ", but got " << values.size();

        for(size_t i = 0u; i < correct.size(); ++i)
        {
            std::ostringstream message;
            message << "At index " << i << ": expected " << correct[i] << ", but got " << values[i]
                    << " with tolerance " << tolerance;
            EXPECT_NEAR(correct[i], values[i], tolerance) << message.str();
        }
    }

    template<typename T>
    typename std::enable_if<!std::is_enum<T>::value, void>::type checkVectorEquality(const std::vector<T> & expected,
                                                                                     const std::vector<T> & actual)
    {
        ASSERT_EQ(expected.size(), actual.size())
          << "Vectors have different sizes: expected " << expected.size() << ", but got " << actual.size();

        for(size_t i = 0u; i < expected.size(); ++i)
        {
            EXPECT_EQ(expected[i], actual[i])
              << "At index " << i << ": expected " << expected[i] << ", but got " << actual[i];
        }
    }

    template<typename T>
    typename std::enable_if<std::is_enum<T>::value, void>::type checkVectorEquality(const std::vector<T> & expected,
                                                                                    const std::vector<T> & actual)
    {
        ASSERT_EQ(expected.size(), actual.size())
          << "Vectors have different sizes: expected " << expected.size() << ", but got " << actual.size();

        for(size_t i = 0u; i < expected.size(); ++i)
        {
            // Using static_cast to convert enum to its underlying type for comparison
            ASSERT_EQ(static_cast<typename std::underlying_type<T>::type>(expected[i]),
                      static_cast<typename std::underlying_type<T>::type>(actual[i]))
              << "At index " << i << ": expected " << static_cast<typename std::underlying_type<T>::type>(expected[i])
              << ", but got " << static_cast<typename std::underlying_type<T>::type>(actual[i]);
        }
    }

    template<typename T, typename ConverterFunction>
    void
      checkVectorEquality(const std::vector<T> & expected, const std::vector<T> & actual, ConverterFunction converter)
    {
        ASSERT_EQ(expected.size(), actual.size())
          << "Vectors have different sizes: expected " << expected.size() << ", but got " << actual.size();

        for(size_t i = 0u; i < expected.size(); ++i)
        {
            EXPECT_EQ(expected[i], actual[i])
              << "At index " << i << ": expected " << converter(expected[i]) << ", but got " << converter(actual[i]);
        }
    }
}   // namespace Helper