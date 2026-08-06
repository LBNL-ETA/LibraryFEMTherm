#pragma once

#include <map>
#include <string>
#include <optional>

#include <fileParse/FileFormat.hxx>

#include "THMZ/Common/DeferredParse.hxx"
#include "Mesh.hxx"

namespace ThermFile::Mesh
{
   struct Mesh;

    std::optional<Mesh> loadMeshFromFile(std::string_view fileName);

    //! Starts parsing the mesh entry from already-extracted archive entries on a worker
    //! thread; returns an empty handle when the archive carries no mesh entry.
    [[nodiscard]] DeferredParse<Mesh> deferredMeshFromEntries(const std::map<std::string, std::string> & entries);
    int saveToFile(const Mesh & model,
                   std::string_view fileName,
                   FileParse::FileFormat format = FileParse::FileFormat::XML);

    std::optional<Mesh> loadMeshFromZipFile(std::string const & fileName);
    int saveToZipFile(const Mesh & mesh,
                      std::string_view zipFileName,
                      FileParse::FileFormat format = FileParse::FileFormat::XML);

    //! Unknown format means: let the content decide (XML or JSON).
    std::optional<Mesh> loadMeshFromString(const std::string & str,
                                           FileParse::FileFormat format = FileParse::FileFormat::Unknown);
    std::string saveToString(const Mesh & mesh,
                             FileParse::FileFormat format = FileParse::FileFormat::XML);
}   // namespace ThermFile