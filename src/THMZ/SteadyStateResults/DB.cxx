#include "DB.hxx"

#include "Results.hxx"
#include "Serializers.hxx"

#include "Common/DB.hxx"

namespace ThermFile
{
    SteadyStateResults loadSteadyStateResultsFromFile(const std::string & fileName)
    {
        return Common::loadFromXMLFile<SteadyStateResults>(fileName, "Results");
    }

    SteadyStateResults loadSteadyStateResultsFromZipFile(const std::string & fileName)
    {
        return Common::loadFromZipFile<SteadyStateResults>(fileName, ThermZip::SteadyStateResultsName, "Results");
    }

    SteadyStateResults loadSteadyStateResultsFromString(const std::string & data)
    {
        return Common::loadFromXMLString<SteadyStateResults>(data, "Results");
    }

    int saveToFile(const SteadyStateResults & results, std::string_view fileName)
    {
        return Common::saveToXMLFile(results, fileName, "Results");
    }

    std::string saveToString(const SteadyStateResults & results)
    {
        return Common::saveToXMLString(results, "Results");
    }

    int saveToZipFile(const SteadyStateResults & results, std::string_view zipFileName)
    {
        return Common::saveToZIPFile(results, ThermZip::SteadyStateResultsName, zipFileName, "Results");
    }
}   // namespace ThermFile