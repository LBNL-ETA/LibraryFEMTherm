#include "DB.hxx"

#include "Results.hxx"
#include "Serializers.hxx"

#include "Common/DB.hxx"

namespace ThermFile
{
    const std::string topNodeName = "MeshResults";

    std::optional<MeshResults> loadSteadyStateMeshResultsFromFile(const std::string & fileName)
    {
        return Common::loadFromXMLFile<MeshResults>(fileName, topNodeName);
    }

    std::optional<MeshResults> loadSteadyStateMeshResultsFromZipFile(const std::string & fileName)
    {
        return Common::loadFromZipFile<MeshResults>(fileName, ThermZip::SteadyStateMeshResultsName, topNodeName);
    }

    int saveToFile(const MeshResults & results, std::string_view fileName)
    {
        return Common::saveToXMLFile(results, fileName, topNodeName);
    }

    std::string saveToString(const MeshResults & results)
    {
        return Common::saveToXMLString(results, topNodeName);
    }

    int saveToZipFile(const MeshResults & results, std::string_view zipFileName)
    {
        return Common::saveToZIPFile(results, ThermZip::SteadyStateMeshResultsName, zipFileName, topNodeName);
    }

    std::optional<MeshResults> loadSteadyStateMeshResultsFromString(const std::string & xmlString)
    {
        return Common::loadFromXMLString<MeshResults>(xmlString, topNodeName);
    }
}   // namespace ThermFile