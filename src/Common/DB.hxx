#pragma once

#include <string>
#include <fstream>
#include <optional>

#include <fileParse/XMLNodeAdapter.hxx>

#include "LibraryUtilities/StringRoutines.hxx"
#include "LibraryUtilities/FileManipulation.hxx"
#include "../THMZ/ZipModule/ZipModule.hxx"

namespace Common
{
    template<typename T>
    T loadFromXMLFile(std::string_view fileName, const std::string & nodeTypeName)
    {
        // Convert std::string_view to std::string for file operations
        std::string fileNameStr(fileName);

        // Check if file exists and is accessible; if not, create it with default content
        if(std::ifstream f(fileNameStr.c_str()); !f.good())
        {
            const std::string fileContent = "<" + nodeTypeName + ">\n</" + nodeTypeName + ">";
            File::createFileFromString(fileNameStr, fileContent);
        }

        // Attempt to load the top node for the given type
        const auto xmlNode = getTopNodeFromFile(fileNameStr, nodeTypeName);

        // Create an instance of the type
        T model;
        if(xmlNode.has_value())
        {
            // Assume that `operator>>` is overloaded for T and xmlNode type
            xmlNode.value() >> model;
        }

        return model;
    }

    template<typename T>
    T loadFromXMLString(const std::string & data, const std::string & nodeTypeName)
    {

        // Attempt to load the top node for the given type
        const auto xmlNode = getTopNodeFromString(data, nodeTypeName);

        // Create an instance of the type
        T model;
        if(xmlNode.has_value())
        {
            // Assume that `operator>>` is overloaded for T and xmlNode type
            xmlNode.value() >> model;
        }

        return model;
    }

    template<typename T>
    T loadFromZipFile(const std::string & zipFileName,
                      const std::string & fileName,
                      const std::string & nodeTypeName)
    {
        auto contents = ThermZip::unzipFile(zipFileName, fileName);
        return loadFromXMLString<T>(contents, nodeTypeName);
    }

    template<typename T>
    int saveToXMLFile(const T & object, std::string_view fileName, const std::string & nodeName)

    {
        auto node = createTopNode(nodeName);

        node << object;

        return node.writeToFile(fileName.data());
    }
}   // namespace Common