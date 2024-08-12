#pragma once

#include <string>

#include "THMZ/SteadyStateMeshResults/Results.hxx"

namespace ThermFile
{
    MeshResults loadSteadyStateMeshResultsFromFile(const std::string & fileName);
    int saveToFile(const MeshResults & results, std::string_view fileName);
}