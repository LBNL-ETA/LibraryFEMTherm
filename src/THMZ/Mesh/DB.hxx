#pragma once

#include <string>
#include <optional>

#include <fileParse/FileFormat.hxx>

#include "Mesh.hxx"

namespace ThermFile::Mesh
{
   struct Mesh;

    std::optional<Mesh> loadMeshFromFile(std::string_view fileName);
    int saveToFile(const Mesh & model,
                   std::string_view fileName,
                   FileParse::FileFormat format = FileParse::FileFormat::XML);

    std::optional<Mesh> loadMeshFromZipFile(std::string const & fileName);
    int saveToZipFile(const Mesh & mesh, std::string_view zipFileName);

    std::optional<Mesh> loadMeshFromString(const std::string & str,
                                           FileParse::FileFormat format = FileParse::FileFormat::XML);
    std::string saveToString(const Mesh & mesh,
                             FileParse::FileFormat format = FileParse::FileFormat::XML);
}   // namespace ThermFile