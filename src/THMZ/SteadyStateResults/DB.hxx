#pragma once

#include <string>
#include <optional>

#include <fileParse/FileFormat.hxx>

#include "THMZ/SteadyStateResults/Results.hxx"

namespace ThermFile
{
    std::optional<SteadyStateResults> loadSteadyStateResultsFromFile(const std::string & fileName);
    int saveToFile(const SteadyStateResults & results,
                   std::string_view fileName,
                   FileParse::FileFormat format = FileParse::FileFormat::XML);

    std::optional<SteadyStateResults> loadSteadyStateResultsFromZipFile(const std::string & fileName);
    int saveToZipFile(const SteadyStateResults & results, std::string_view zipFileName);

    std::optional<SteadyStateResults> loadSteadyStateResultsFromString(const std::string & str,
                                                                       FileParse::FileFormat format = FileParse::FileFormat::XML);
    std::string saveToString(const SteadyStateResults & results,
                             FileParse::FileFormat format = FileParse::FileFormat::XML);
}