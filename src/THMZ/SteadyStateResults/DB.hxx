#pragma once

#include <map>
#include <string>
#include <optional>

#include <fileParse/FileFormat.hxx>

#include "THMZ/Common/DeferredParse.hxx"
#include "THMZ/SteadyStateResults/Results.hxx"

namespace ThermFile
{
    std::optional<SteadyStateResults> loadSteadyStateResultsFromFile(const std::string & fileName);

    //! Starts parsing the steady-state results entry from already-extracted archive
    //! entries on a worker thread; empty handle when the archive carries no such entry.
    [[nodiscard]] DeferredParse<SteadyStateResults>
      deferredResultsFromEntries(const std::map<std::string, std::string> & entries);
    int saveToFile(const SteadyStateResults & results,
                   std::string_view fileName,
                   FileParse::FileFormat format = FileParse::FileFormat::XML);

    std::optional<SteadyStateResults> loadSteadyStateResultsFromZipFile(const std::string & fileName);
    int saveToZipFile(const SteadyStateResults & results,
                      std::string_view zipFileName,
                      FileParse::FileFormat format = FileParse::FileFormat::XML);

    //! Unknown format means: let the content decide (XML or JSON).
    std::optional<SteadyStateResults> loadSteadyStateResultsFromString(const std::string & str,
                                                                       FileParse::FileFormat format = FileParse::FileFormat::Unknown);
    std::string saveToString(const SteadyStateResults & results,
                             FileParse::FileFormat format = FileParse::FileFormat::XML);
}