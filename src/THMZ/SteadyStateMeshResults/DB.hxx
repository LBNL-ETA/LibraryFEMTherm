#pragma once

#include <string>

#include "THMZ/SteadyStateMeshResults/Results.hxx"

namespace ThermFile
{
    MeshResults loadSteadyStateMeshResultsFromFile(const std::string & fileName);
    int saveToFile(const MeshResults & results, std::string_view fileName);

    MeshResults loadSteadyStateMeshResultsFromZipFile(const std::string & fileName);
    int saveToZipFile(const MeshResults & results, std::string_view zipFileName);

    std::string saveToString(const MeshResults & results);
    MeshResults loadSteadyStateMeshResultsFromString(const std::string & xmlString);
}