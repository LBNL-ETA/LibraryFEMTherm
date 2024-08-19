#pragma once

#include <string>

#include "Mesh.hxx"

namespace ThermFile::Mesh
{
   struct Mesh;

    Mesh loadMeshFromFile(std::string_view fileName);
    int saveToFile(const Mesh & model, std::string_view fileName);

    Mesh loadMeshFromZipFile(std::string const & fileName);
    int saveToZipFile(const Mesh & mesh, std::string_view zipFileName);

    std::string saveToString(const Mesh & mesh);
    Mesh loadMeshFromString(const std::string & xmlString);
}   // namespace ThermFile