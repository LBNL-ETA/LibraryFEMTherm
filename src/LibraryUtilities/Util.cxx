#include "Util.hxx"

#include <sstream>
#include <ostream>

namespace Util
{
    std::string toScientific(double value)
    {
        std::ostringstream out;
        out << std::scientific << value;
        return out.str();
    }
}