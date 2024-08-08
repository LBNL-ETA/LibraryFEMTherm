#include "DB.hxx"

#include "Results.hxx"
#include "Serializers.hxx"

#include "Common/DB.hxx"

namespace ThermFile
{
    MeshResults loadSteadyStateMeshResultsFromFile(const std::string & fileName)
    {
        return Common::loadFromXMLFile<MeshResults>(fileName, "MeshResults");
    }

    MeshResults loadSteadyStateMeshResultsFromZipFile(const std::string & fileName)
    {
        return Common::loadFromZipFile<MeshResults>(fileName, "SteadyStateMeshResults.xml", "Results");
    }

    int saveToFile(const MeshResults & results, std::string_view fileName)
    {
        return Common::saveToXMLFile(results, fileName, "MeshResults");
    }
}   // namespace ThermFile