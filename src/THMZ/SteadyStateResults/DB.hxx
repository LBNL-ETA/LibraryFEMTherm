#pragma once

#include <string>

#include "THMZ/SteadyStateResults/Results.hxx"

namespace ThermFile
{
    SteadyStateResults loadSteadyStateResultsFromFile(const std::string & fileName);
    SteadyStateResults loadSteadyStateResultsFromZipFile(const std::string & fileName);
    SteadyStateResults loadSteadyStateResultsFromString(const std::string & data);
    int saveToFile(const SteadyStateResults & results, std::string_view fileName);
}