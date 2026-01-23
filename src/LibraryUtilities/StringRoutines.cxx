#include <algorithm>
#include <locale>
#include <cctype>
#include <sstream>

#include "StringRoutines.hxx"

namespace StringRoutines
{
    std::string ltrim(std::string_view s)
    {
        std::string result{s};
        const auto it2 = std::find_if(
          result.begin(), result.end(), [](char ch) { return !std::isspace<char>(ch, std::locale::classic()); });
        result.erase(result.begin(), it2);
        return result;
    }

    std::string rtrim(std::string_view s)
    {
        std::string result{s};
        const auto it1 = std::find_if(
          result.rbegin(), result.rend(), [](char ch) { return !std::isspace<char>(ch, std::locale::classic()); });
        result.erase(it1.base(), result.end());
        return result;
    }

    std::string trim(std::string_view s)
    {
        return ltrim(rtrim(s));
    }


    std::string toUpper(std::string_view value)
    {
        std::string result{value};

        for(auto & chr : result)
        {
            chr = static_cast<char>(toupper(chr));
        }

        return result;
    }

    std::string toLower(std::string_view value)
    {
        std::string result{value};

        for(auto & chr : result)
        {
            chr = static_cast<char>(tolower(chr));
        }

        return result;
    }

    double to_double(std::string_view value)
    {
        std::istringstream os{value.data()};
        double d;
        os >> d;
        return d;
    }

    std::string to_string(double value)
    {
        std::ostringstream out;
        out << value;
        return out.str();
    }
}   // namespace StringRoutines
