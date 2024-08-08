#pragma once

#include <string>

namespace ThermFile
{
    struct SteadyStateResults;
    
    SteadyStateResults loadSteadyStateResultsFromFile(const std::string & fileName);
    SteadyStateResults loadSteadyStateResultsFromZipFile(const std::string & fileName);
    SteadyStateResults loadSteadyStateResultsFromString(const std::string & data);
    int saveToFile(const SteadyStateResults & results, std::string_view fileName);
}