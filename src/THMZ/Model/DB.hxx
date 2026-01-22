#pragma once

#include <string>
#include <optional>

#include <fileParse/FileFormat.hxx>

#include "THMZ/Model/THMX.hxx"

namespace ThermFile
{
    std::optional<ThermModel> loadThermModelFromFile(std::string_view fileName);
    int saveToFile(const ThermModel & model,
                   std::string_view fileName,
                   FileParse::FileFormat format = FileParse::FileFormat::XML);

    std::optional<ThermModel> loadThermModelFromZipFile(const std::string & zipFileName);
    int saveToZipFile(const ThermModel & model, std::string_view zipFileName);

    std::optional<ThermModel> loadThermModelFromString(const std::string & str,
                                                       FileParse::FileFormat format = FileParse::FileFormat::XML);
    std::string saveToString(const ThermModel & model,
                             FileParse::FileFormat format = FileParse::FileFormat::XML);
}   // namespace ThermFile