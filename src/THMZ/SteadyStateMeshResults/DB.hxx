#pragma once

#include <string>

namespace ThermFile
{
    struct MeshResults;

    MeshResults loadSteadyStateMeshResultsFromFile(const std::string & fileName);
    MeshResults loadSteadyStateMeshResultsFromZipFile(const std::string & fileName);
    int saveToFile(const MeshResults & results, std::string_view fileName);
}