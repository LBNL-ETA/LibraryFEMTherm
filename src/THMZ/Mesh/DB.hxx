#pragma once

#include <string>

namespace ThermFile::Mesh
{
   struct Mesh;

    Mesh loadMeshFromFile(const std::string_view fileName);
    int saveToFile(const Mesh & model, std::string_view fileName);
}   // namespace ThermFile