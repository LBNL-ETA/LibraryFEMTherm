#include "DB.hxx"

#include "THMX.hxx"
#include "Serializers.hxx"

#include "Common/DB.hxx"

namespace ThermFile
{
    const std::string topNodeName = "ThermModel";

    ThermModel loadThermModelFromFile(std::string_view fileName)
    {
        return Common::loadFromXMLFile<ThermModel>(fileName, topNodeName);
    }

    ThermModel loadThermModelFromZipFile(std::string const & zipFileName)
    {
        return Common::loadFromZipFile<ThermModel>(zipFileName, ThermZip::ModelFileName, topNodeName);
    }

    int saveToFile(const ThermModel & model, std::string_view fileName)
    {
        return Common::saveToXMLFile(model, fileName, topNodeName);
    }

    std::string saveToString(const ThermModel & model)
    {
        return Common::saveToXMLString(model, topNodeName);
    }

    int saveToZipFile(const ThermModel & model, std::string_view zipFileName)
    {
        return Common::saveToZIPFile(model, ThermZip::ModelFileName, zipFileName, topNodeName);
    }

    ThermModel loadThermModelFromString(const std::string & xmlString)
    {
        return Common::loadFromXMLString<ThermModel>(xmlString, topNodeName);
    }
}   // namespace ThermFile
