#pragma once

#include <string>

namespace ThermFile::Mesh
{
   struct Mesh;

    Mesh loadMeshFromFile(std::string_view fileName);
    Mesh loadMeshFromZipFile(std::string const & fileName);
    int saveToFile(const Mesh & model, std::string_view fileName);
}   // namespace ThermFile