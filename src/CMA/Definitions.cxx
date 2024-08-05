#include "Definitions.hxx"

#include "LibraryUtilities/Exceptions.hxx"

namespace CMALibrary
{

    std::string optionToString(Option option)
    {
        const std::map<Option, std::string> aMap = {{Option::Low, "Low"}, {Option::High, "High"}};

        return aMap.at(option);
    }

    Option optionFromString(std::string_view optionName)
    {
        const std::map<std::string, Option> aMap{{"Low", Option::Low}, {"High", Option::High}};

        if(aMap.count(optionName.data()) > 0)
        {
            return aMap.at(optionName.data());
        }

        return Option::Low;
    }
}   // namespace CMALibrary