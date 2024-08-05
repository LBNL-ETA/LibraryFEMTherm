#pragma once

#include <gtest/gtest.h>
#include <unordered_map>
#include <map>
#include <type_traits>

namespace Helper
{
    template<typename T>
    struct is_valid_map : std::false_type
    {};

    template<typename K, typename V>
    struct is_valid_map<std::map<K, V>> : std::true_type
    {};

    template<typename K, typename V>
    struct is_valid_map<std::unordered_map<K, V>> : std::true_type
    {};

    // Check if the values in two maps are near within a tolerance
    template<typename MapType, typename ValueType>
    std::enable_if_t<is_valid_map<MapType>::value>
      checkMapValuesNear(const MapType & expected, const MapType & actual, ValueType tolerance)
    {
        ASSERT_EQ(expected.size(), actual.size());

        for(const auto & [expectedKey, expectedValue] : expected)
        {
            auto actualIter = actual.find(expectedKey);
            ASSERT_NE(actualIter, actual.end());
            EXPECT_NEAR(expectedValue, actualIter->second, tolerance);
        }
    }

    // General function for non-enum map values
    template<typename MapType>
    std::enable_if_t<!std::is_enum<typename MapType::mapped_type>::value> checkMapValuesEqual(const MapType & expected,
                                                                                              const MapType & actual)
    {
        ASSERT_EQ(expected.size(), actual.size());

        for(const auto & [expectedKey, expectedValue] : expected)
        {
            auto actualIter = actual.find(expectedKey);
            ASSERT_NE(actualIter, actual.end());
            EXPECT_EQ(expectedValue, actualIter->second);
        }
    }

    // Specialized function for enum map values
    template<typename MapType>
    std::enable_if_t<std::is_enum<typename MapType::mapped_type>::value> checkMapValuesEqual(const MapType & expected,
                                                                                             const MapType & actual)
    {
        ASSERT_EQ(expected.size(), actual.size());

        for(const auto & [expectedKey, expectedValue] : expected)
        {
            auto actualIter = actual.find(expectedKey);
            ASSERT_NE(actualIter, actual.end());
            EXPECT_EQ(static_cast<int>(expectedValue), static_cast<int>(actualIter->second));
        }
    }

}   // namespace Helper