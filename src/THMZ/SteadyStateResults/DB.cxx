#include "DB.hxx"

#include "Results.hxx"
#include "Serializers.hxx"

#include "Common/DB.hxx"

namespace ThermFile
{
    const std::string topNodeName = "Results";
    SteadyStateResults loadSteadyStateResultsFromFile(const std::string & fileName)
    {
        return Common::loadFromXMLFile<SteadyStateResults>(fileName, topNodeName);
    }

    SteadyStateResults loadSteadyStateResultsFromZipFile(const std::string & fileName)
    {
        return Common::loadFromZipFile<SteadyStateResults>(fileName, ThermZip::SteadyStateResultsName, topNodeName);
    }

    SteadyStateResults loadSteadyStateResultsFromString(const std::string & data)
    {
        return Common::loadFromXMLString<SteadyStateResults>(data, topNodeName);
    }

    int saveToFile(const SteadyStateResults & results, std::string_view fileName)
    {
        return Common::saveToXMLFile(results, fileName, topNodeName);
    }

    std::string saveToString(const SteadyStateResults & results)
    {
        return Common::saveToXMLString(results, topNodeName);
    }

    int saveToZipFile(const SteadyStateResults & results, std::string_view zipFileName)
    {
        return Common::saveToZIPFile(results, ThermZip::SteadyStateResultsName, zipFileName, topNodeName);
    }
}   // namespace ThermFile