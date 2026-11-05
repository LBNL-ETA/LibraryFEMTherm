#pragma once

#include <string>

namespace BCLibrary::Import
{
    //! \brief The name of the first element, past the XML declaration and any comment. Every
    //! library is an .xml file, so this is what names its format. Empty when there is none.
    [[nodiscard]] std::string topElement(const std::string & text);
}   // namespace BCLibrary::Import
