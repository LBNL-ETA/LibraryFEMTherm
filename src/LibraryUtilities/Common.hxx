#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <span>

namespace LibraryCommon
{
    template<typename EnumType, std::size_t N>
    EnumType enumFromString(std::string_view name, const std::array<std::string, N> & values, EnumType defaultValue)
    {
        for(std::size_t i = 0; i < values.size(); ++i)
        {
            if(values[i] == name)
            {
                return static_cast<EnumType>(i);
            }
        }
        return defaultValue;
    }

    template<typename EnumType, std::size_t N>
    std::string enumToString(EnumType value, const std::array<std::string, N> & values)
    {
        std::size_t index = static_cast<std::size_t>(value);
        if(index < values.size())
        {
            return values[index];
        }
        return "Unknown";
    }

    template<typename T>
    std::string enumToString(const T enumType, const std::map<T, std::string> & enumMap)
    {
        if(enumMap.count(enumType) > 0)
        {
            return enumMap.at(enumType);
        }
        return "Unknown";
    }

    template<typename T>
    T enumFromString(std::string_view enumString, const std::map<T, std::string> & enumMap)
    {
        if(!enumString.empty())
        {
            auto it = std::find_if(
              enumMap.begin(), enumMap.end(), [&enumString](const auto & pair) { return pair.second == enumString; });

            if(it != enumMap.end())
            {
                return it->first;
            }
        }

        return enumMap.begin()->first;
    }

    template<typename T>
    std::vector<std::string> stringVector(const std::map<T, std::string> & enumMap)
    {
        std::vector<std::string> result;

        for(const auto & [key, val] : enumMap)
        {
            result.emplace_back(val);
        }

        return result;
    }

    template<typename T>
    bool isRecordTemporary(const T & record)
    {
        return record.ProjectName && !record.ProjectName->empty();
    }


    template<typename T>
    std::string DisplayName(const T & record)
    {
        return !record.ProjectName ? record.Name : record.ProjectName.value() + ":" + record.Name;
    }

    template<typename T, typename std::enable_if<std::is_floating_point<T>::value>::type * = nullptr>
    bool isEqual(T value1, T value2, const double tolerance = 1e-9)
    {
        if(std::abs(value1 - value2) <= tolerance)
        {
            return true;
        }

        return false;
    }

    template<typename T, typename std::enable_if<std::is_floating_point<T>::value>::type * = nullptr>
    bool isEqual(const std::optional<T> & value1, const std::optional<T> & value2, const double tolerance = 1e-9)
    {
        if(value1 && value2)
        {
            return isEqual(*value1, *value2, tolerance);
        }

        return (!value1 && !value2);
    }
}   // namespace LibraryCommon
