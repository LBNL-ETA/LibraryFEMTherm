#pragma once

#include <string>
#include <optional>

#include "THMZ/SteadyStateMeshResults/Results.hxx"

namespace ThermFile
{
    std::optional<MeshResults> loadSteadyStateMeshResultsFromFile(const std::string & fileName);
    int saveToFile(const MeshResults & results, std::string_view fileName);

    std::optional<MeshResults> loadSteadyStateMeshResultsFromZipFile(const std::string & fileName);
    int saveToZipFile(const MeshResults & results, std::string_view zipFileName);

    std::optional<MeshResults> loadSteadyStateMeshResultsFromString(const std::string & xmlString);
    std::string saveToString(const MeshResults & results);
}