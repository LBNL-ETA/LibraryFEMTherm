#pragma once

#include <map>
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

    //! Loads the model (including migrations) from already-extracted archive entries,
    //! so a caller that holds the archive in memory pays for no further extraction.
    std::optional<ThermModel> loadThermModelFromEntries(const std::map<std::string, std::string> & entries);

    std::optional<ThermModel> loadThermModelFromZipFile(const std::string & zipFileName);
    int saveToZipFile(const ThermModel & model,
                      std::string_view zipFileName,
                      FileParse::FileFormat format = FileParse::FileFormat::XML);

    //! Unknown format means: let the content decide (XML or JSON).
    std::optional<ThermModel> loadThermModelFromString(const std::string & str,
                                                       FileParse::FileFormat format = FileParse::FileFormat::Unknown);
    std::string saveToString(const ThermModel & model,
                             FileParse::FileFormat format = FileParse::FileFormat::XML);
}   // namespace ThermFile