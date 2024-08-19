#pragma once

#include <string>

#include "THMZ/SteadyStateResults/Results.hxx"

namespace ThermFile
{
    SteadyStateResults loadSteadyStateResultsFromFile(const std::string & fileName);
    int saveToFile(const SteadyStateResults & results, std::string_view fileName);

    SteadyStateResults loadSteadyStateResultsFromZipFile(const std::string & fileName);
    int saveToZipFile(const SteadyStateResults & results, std::string_view zipFileName);

    std::string saveToString(const SteadyStateResults & results);
    SteadyStateResults loadSteadyStateResultsFromString(const std::string & data);
}