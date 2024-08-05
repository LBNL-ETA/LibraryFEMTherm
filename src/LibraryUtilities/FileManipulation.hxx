#pragma once

#include <string>
#include <vector>
#include <optional>
#include <set>

namespace File
{
    //! \brief Function to create file whit content of what is passed as string
    //!
    //! @fileName - file that will be created with content of passed string
    //! @fileContent - content of this string will be put into file
    void createFileFromString(std::string_view fileName, std::string_view fileContent);

    std::string loadToString(const std::string & fileName);

    //! \brief Function to load file to set. Every line of file will be one element of set. However
    //! if there are two lines with same content, only one will be in set. This will be useful
    //! for simple libraries which entries should be unique.
    std::set<std::string> loadFileToSet(const std::string & fileName);

    void saveSetToFile(const std::string & fileName, const std::set<std::string> & set);
}   // namespace File
