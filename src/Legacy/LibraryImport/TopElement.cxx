#include "TopElement.hxx"

namespace BCLibrary::Import
{
    std::string topElement(const std::string & text)
    {
        auto start{text.find('<')};
        while(start != std::string::npos && start + 1u < text.size()
              && (text[start + 1u] == '?' || text[start + 1u] == '!'))
        {
            start = text.find('<', start + 1u);
        }
        if(start == std::string::npos)
        {
            return {};
        }
        return text.substr(start + 1u, text.find_first_of(" \t\r\n/>", start + 1u) - start - 1u);
    }
}   // namespace BCLibrary::Import
