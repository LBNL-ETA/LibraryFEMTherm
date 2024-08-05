#pragma once

#include <map>
#include <optional>
#include <string>

namespace CMALibrary
{
    enum class Option
    {
        Low,
        High
    };

    std::string optionToString(Option option);

    Option optionFromString(std::string_view optionName);

}   // namespace CMALibrary