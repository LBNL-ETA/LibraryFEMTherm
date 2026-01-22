#include "DB.hxx"

#include "THMX.hxx"
#include "Serializers.hxx"

#include "Common/DB.hxx"

namespace ThermFile
{
    const std::string topNodeName = "ThermModel";

    std::optional<ThermModel> loadThermModelFromFile(std::string_view fileName)
    {
        return Common::loadFromFile<ThermModel>(fileName, topNodeName);
    }

    std::optional<ThermModel> loadThermModelFromZipFile(std::string const & zipFileName)
    {
        return Common::loadFromZipFile<ThermModel>(zipFileName, ThermZip::ModelFileName, topNodeName);
    }

    int saveToFile(const ThermModel & model, std::string_view fileName, FileParse::FileFormat format)
    {
        switch(format)
        {
            case FileParse::FileFormat::JSON:
                return Common::saveToJSONFile(model, fileName, topNodeName);
            case FileParse::FileFormat::XML:
            default:
                return Common::saveToXMLFile(model, fileName, topNodeName);
        }
    }

    std::string saveToString(const ThermModel & model, FileParse::FileFormat format)
    {
        return Common::saveToString(model, topNodeName, format);
    }

    int saveToZipFile(const ThermModel & model, std::string_view zipFileName)
    {
        return Common::saveToZIPFile(model, ThermZip::ModelFileName, zipFileName, topNodeName);
    }

    std::optional<ThermModel> loadThermModelFromString(const std::string & str, FileParse::FileFormat format)
    {
        return Common::loadFromString<ThermModel>(str, topNodeName, format);
    }
}   // namespace ThermFile
