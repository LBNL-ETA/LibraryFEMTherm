#pragma once

#include <string>
#include <optional>

#include "THMZ/SteadyStateResults/Results.hxx"

namespace ThermFile
{
    std::optional<SteadyStateResults> loadSteadyStateResultsFromFile(const std::string & fileName);
    int saveToFile(const SteadyStateResults & results, std::string_view fileName);

    std::optional<SteadyStateResults> loadSteadyStateResultsFromZipFile(const std::string & fileName);
    int saveToZipFile(const SteadyStateResults & results, std::string_view zipFileName);

    std::optional<SteadyStateResults> loadSteadyStateResultsFromString(const std::string & data);
    std::string saveToString(const SteadyStateResults & results);
}