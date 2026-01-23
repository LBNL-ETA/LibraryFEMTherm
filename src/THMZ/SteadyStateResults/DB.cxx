#include "DB.hxx"

#include "Results.hxx"
#include "Serializers.hxx"

#include <fileParse/FileFormat.hxx>

#include "Common/DB.hxx"

namespace ThermFile
{
    const std::string topNodeName = "Results";

    std::optional<SteadyStateResults> loadSteadyStateResultsFromFile(const std::string & fileName)
    {
        return Common::loadFromXMLFile<SteadyStateResults>(fileName, topNodeName);
    }

    std::optional<SteadyStateResults> loadSteadyStateResultsFromZipFile(const std::string & fileName)
    {
        return Common::loadFromZipFile<SteadyStateResults>(fileName, ThermZip::SteadyStateResultsName, topNodeName);
    }

    std::optional<SteadyStateResults> loadSteadyStateResultsFromString(const std::string & data,
                                                                       FileParse::FileFormat format)
    {
        return Common::loadFromString<SteadyStateResults>(data, topNodeName, format);
    }

    int saveToFile(const SteadyStateResults & results,
                   std::string_view fileName,
                   FileParse::FileFormat format)
    {
        switch(format)
        {
            case FileParse::FileFormat::XML:
                return Common::saveToXMLFile(results, fileName, topNodeName);
            case FileParse::FileFormat::JSON:
                return Common::saveToJSONFile(results, fileName, topNodeName);
            default:
                return -1;
        }
    }

    std::string saveToString(const SteadyStateResults & results, FileParse::FileFormat format)
    {
        return Common::saveToString(results, topNodeName, format);
    }

    int saveToZipFile(const SteadyStateResults & results, std::string_view zipFileName)
    {
        return Common::saveToZIPFile(results, ThermZip::SteadyStateResultsName, zipFileName, topNodeName);
    }
}   // namespace ThermFile