#pragma once

#include <string>
#include <optional>

#include "Mesh.hxx"

namespace ThermFile::Mesh
{
   struct Mesh;

    std::optional<Mesh> loadMeshFromFile(std::string_view fileName);
    int saveToFile(const Mesh & model, std::string_view fileName);

    std::optional<Mesh> loadMeshFromZipFile(std::string const & fileName);
    int saveToZipFile(const Mesh & mesh, std::string_view zipFileName);

    std::optional<Mesh> loadMeshFromString(const std::string & xmlString);
    std::string saveToString(const Mesh & mesh);
}   // namespace ThermFile