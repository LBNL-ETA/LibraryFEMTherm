#pragma once

#include <string>

#include "Mesh.hxx"

namespace ThermFile::Mesh
{
   struct Mesh;

    Mesh loadMeshFromFile(std::string_view fileName);
    Mesh loadMeshFromZipFile(std::string const & fileName);
    int saveToFile(const Mesh & model, std::string_view fileName);

    std::string saveToString(const Mesh & mesh);
    int saveToZipFile(const Mesh & mesh, std::string_view zipFileName);
}   // namespace ThermFile