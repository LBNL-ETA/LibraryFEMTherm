#pragma once

#include <string>

#include "Mesh.hxx"

namespace ThermFile::Mesh
{
    Mesh loadMeshFromFile(std::string_view fileName);
    int saveToFile(const Mesh & model, std::string_view fileName);
}   // namespace ThermFile