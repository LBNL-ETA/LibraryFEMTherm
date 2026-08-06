#pragma once

#include <map>
#include <string>
#include <optional>

#include <fileParse/FileFormat.hxx>

#include "THMZ/Common/DeferredParse.hxx"
#include "THMZ/SteadyStateMeshResults/Results.hxx"

namespace ThermFile
{
    std::optional<MeshResults> loadSteadyStateMeshResultsFromFile(const std::string & fileName);

    //! Starts parsing the mesh-results entry from already-extracted archive entries on a
    //! worker thread; returns an empty handle when the archive carries no such entry.
    [[nodiscard]] DeferredParse<MeshResults>
      deferredMeshResultsFromEntries(const std::map<std::string, std::string> & entries);
    int saveToFile(const MeshResults & results,
                   std::string_view fileName,
                   FileParse::FileFormat format = FileParse::FileFormat::XML);

    std::optional<MeshResults> loadSteadyStateMeshResultsFromZipFile(const std::string & fileName);
    int saveToZipFile(const MeshResults & results,
                      std::string_view zipFileName,
                      FileParse::FileFormat format = FileParse::FileFormat::XML);

    std::optional<MeshResults> loadSteadyStateMeshResultsFromString(const std::string & str);
    std::string saveToString(const MeshResults & results,
                             FileParse::FileFormat format = FileParse::FileFormat::XML);
}
