#include "DB.hxx"

#include "Results.hxx"
#include "Serializers.hxx"

#include "Common/DB.hxx"

namespace ThermFile
{
    const std::string topNodeName = "MeshResults";

    std::optional<MeshResults> loadSteadyStateMeshResultsFromFile(const std::string & fileName)
    {
        return Common::loadFromFile<MeshResults>(fileName, topNodeName);
    }

    std::optional<MeshResults> loadSteadyStateMeshResultsFromZipFile(const std::string & fileName)
    {
        return Common::loadFromZipFile<MeshResults>(fileName, ThermZip::SteadyStateMeshResultsName, topNodeName);
    }

    int saveToFile(const MeshResults & results, std::string_view fileName, FileParse::FileFormat format)
    {
        switch(format)
        {
            case FileParse::FileFormat::JSON:
                return Common::saveToJSONFile(results, fileName, topNodeName);
            case FileParse::FileFormat::XML:
            default:
                return Common::saveToXMLFile(results, fileName, topNodeName);
        }
    }

    std::string saveToString(const MeshResults & results, FileParse::FileFormat format)
    {
        return Common::saveToString(results, topNodeName, format);
    }

    int saveToZipFile(const MeshResults & results, std::string_view zipFileName, FileParse::FileFormat format)
    {
        return Common::saveToZIPFile(results, ThermZip::SteadyStateMeshResultsName, zipFileName, topNodeName, format);
    }

    std::optional<MeshResults> loadSteadyStateMeshResultsFromString(const std::string & str)
    {
        return Common::loadFromString<MeshResults>(std::string{Common::stripUTF8BOM(str)}, topNodeName);
    }
}   // namespace ThermFile
