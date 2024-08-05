#pragma once

#include <exception>
#include <string>

namespace Utilities
{
    class InvalidOptionException : public std::exception
    {
    public:
        explicit InvalidOptionException(const char * message);

        [[nodiscard]] const char * what() const noexcept override;

    private:
        std::string message_;
    };

}   // namespace Utilities