#include "Exceptions.hxx"

namespace Utilities
{
    InvalidOptionException::InvalidOptionException(const char * message) : message_{message}
    {}

    const char * InvalidOptionException::what() const noexcept
    {
        return message_.c_str();
    }
}   // namespace Utilities