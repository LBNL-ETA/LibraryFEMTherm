#pragma once

#include <string>
#include <optional>

#include <fileParse/FileDataHandler.hxx>

#include "../THMZ/ZipModule/ZipModule.hxx"

namespace Common
{
    template<typename T>
    std::optional<T>
      loadFromZipFile(const std::string & zipFileName, const std::string & fileName, const std::string & nodeTypeName)
    {
        auto contents = ThermZip::unzipFile(zipFileName, fileName);
        return loadFromXMLString<T>(contents, nodeTypeName);
    }

    template<typename T>
    int saveToZIPFile(const T & object,
                      std::string_view fileName,
                      std::string_view zipFileName,
                      const std::string & nodeName)

    {
        return ThermZip::addToZipFile(zipFileName, fileName, saveToXMLString(object, nodeName));
    }
}   // namespace Common