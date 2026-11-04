#pragma once

#include <optional>
#include <string_view>

namespace Common
{
    //! A number read from text, and where the reading stopped.
    struct ParsedDouble
    {
        double value{0.0};
        const char * next{nullptr};
    };

    //! Reads the number at the start of [first, last). Empty when there is none, or when it does
    //! not fit a double. Unlike std::from_chars for double, it builds on every standard library:
    //! older Apple ones do not have that overload.
    [[nodiscard]] std::optional<ParsedDouble> parseDouble(const char * first, const char * last);

    //! The number when the whole text is one number, else empty.
    [[nodiscard]] std::optional<double> wholeDouble(std::string_view text);
}   // namespace Common
