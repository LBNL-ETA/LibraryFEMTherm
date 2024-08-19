#pragma once

#include <string>

#include "THMZ/SteadyStateMeshResults/Results.hxx"

namespace ThermFile
{
    MeshResults loadSteadyStateMeshResultsFromFile(const std::string & fileName);
    MeshResults loadSteadyStateMeshResultsFromZipFile(const std::string & fileName);
    int saveToFile(const MeshResults & results, std::string_view fileName);

    std::string saveToString(const MeshResults & results);
    int saveToZipFile(const MeshResults & results, std::string_view zipFileName);
}