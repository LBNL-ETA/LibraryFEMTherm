#pragma once

#include <string>
#include <vector>

namespace StringRoutines
{
    std::string ltrim(std::string_view s);

    std::string rtrim(std::string_view s);

    std::string trim(std::string_view s);

    std::string toUpper(std::string_view value);

    std::string toLower(std::string_view value);

    double to_double(std::string_view value);

    std::string to_string(double value);
}   // namespace StringRoutines
