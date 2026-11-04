#include <algorithm>
#include <cerrno>
#include <charconv>
#include <cstdlib>
#include <string>
#include <system_error>
#include <version>

#include "ParseNumber.hxx"

namespace Common
{
#if defined(__cpp_lib_to_chars)
    std::optional<ParsedDouble> parseDouble(const char * first, const char * last)
    {
        double value{0.0};
        const auto [next, errc]{std::from_chars(first, last, value)};
        if(errc != std::errc{})
        {
            return std::nullopt;
        }
        return ParsedDouble{.value = value, .next = next};
    }
#else
    namespace
    {
        //! The characters strtod may read: it needs a terminated copy of the number alone, and
        //! copying only these keeps a long line from being copied once per number.
        [[nodiscard]] bool endsNumber(const char letter)
        {
            return letter == ',' || letter == ' ' || letter == '\t' || letter == '\r' || letter == '\n';
        }
    }   // namespace

    std::optional<ParsedDouble> parseDouble(const char * first, const char * last)
    {
        const std::string number{first, std::find_if(first, last, endsNumber)};
        if(number.empty() || number.front() == '+')
        {
            return std::nullopt;
        }

        char * stopped{nullptr};
        errno = 0;
        const double value{std::strtod(number.c_str(), &stopped)};
        if(stopped == number.c_str() || errno == ERANGE)
        {
            return std::nullopt;
        }
        return ParsedDouble{.value = value, .next = first + (stopped - number.c_str())};
    }
#endif

    std::optional<double> wholeDouble(std::string_view text)
    {
        const char * const last{text.data() + text.size()};
        const auto parsed{parseDouble(text.data(), last)};
        if(!parsed.has_value() || parsed->next != last)
        {
            return std::nullopt;
        }
        return parsed->value;
    }
}   // namespace Common
