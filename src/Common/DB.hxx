#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <filesystem>
#include <fstream>
#include <sstream>

#include <fileParse/FileDataHandler.hxx>

#include "../THMZ/ZipModule/ZipModule.hxx"

namespace Common
{
    //! FileParse's XML writer emits a UTF-8 BOM, but its content-based format sniffing
    //! does not skip one, so a BOM-carrying file or string reads as Unknown. Every
    //! content-based dispatch below strips the BOM first.
    [[nodiscard]] inline std::string_view stripUTF8BOM(std::string_view content)
    {
        constexpr std::string_view bomMarker{"\xEF\xBB\xBF"};
        if(content.starts_with(bomMarker))
        {
            return content.substr(bomMarker.size());
        }
        return content;
    }

    //! BOM-tolerant replacement for getTopNodeFromFile: reads the file once, strips the
    //! BOM, and lets the content decide between XML and JSON regardless of extension.
    [[nodiscard]] inline std::optional<NodeAdapter> getLibraryTopNodeFromFile(std::string_view fileName,
                                                                              std::string_view topNodeName)
    {
        const std::ifstream file{std::string{fileName}};
        if(!file.good())
        {
            return std::nullopt;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        const std::string content{stripUTF8BOM(buffer.str())};

        return getTopNodeFromString(content, topNodeName);
    }
    //! Callers pass the extension-free entry base name; the funnel probes the JSON
    //! spelling first and falls back to XML, with the content deciding which parser runs.
    template<typename T>
    std::optional<T>
      loadFromZipFile(const std::string & zipFileName, const std::string & fileName, const std::string & nodeTypeName)
    {
        if(!std::filesystem::exists(zipFileName))
        {
            return std::nullopt;
        }

        try
        {
            const auto candidates{ThermZip::entryNameCandidates(fileName)};
            const auto entries{ThermZip::unzipFiles(zipFileName, candidates)};
            for(const auto & candidate : candidates)
            {
                if(const auto entry{entries.find(candidate)}; entry != entries.end())
                {
                    return loadFromString<T>(std::string{stripUTF8BOM(entry->second)}, nodeTypeName);
                }
            }
            return std::nullopt;
        }
        catch(const std::runtime_error &)
        {
            return std::nullopt;
        }
    }


    template<typename T>
    int saveToZIPFile(const T & object,
                      std::string_view fileName,
                      std::string_view zipFileName,
                      const std::string & nodeName,
                      FileParse::FileFormat format = FileParse::FileFormat::XML)

    {
        const std::string canonicalName{fileName};
        const auto entryName{ThermZip::entryNameForFormat(canonicalName, format)};
        auto obsoleteNames{ThermZip::entryNameCandidates(canonicalName)};
        std::erase(obsoleteNames, entryName);
        return ThermZip::addToZipFile(zipFileName, entryName, saveToString(object, nodeName, format), obsoleteNames);
    }
}   // namespace Common