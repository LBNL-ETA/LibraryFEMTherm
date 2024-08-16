#include "DB.hxx"

#include "THMX.hxx"
#include "Serializers.hxx"

#include "Common/DB.hxx"

namespace ThermFile
{
    ThermModel loadThermModelFromFile(std::string_view fileName)
    {
        return Common::loadFromXMLFile<ThermModel>(fileName, "ThermModel");
    }

    ThermModel loadThermModelFromZipFile(std::string const & fileName)
    {
        return Common::loadFromZipFile<ThermModel>(fileName, ThermZip::ModelFileName, "ThermModel");
    }

    int saveToFile(const ThermModel & model, std::string_view fileName)
    {
        return Common::saveToXMLFile(model, fileName, "ThermModel");
    }
}   // namespace ThermFile
