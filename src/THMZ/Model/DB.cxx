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

    int saveToFile(const ThermModel & model, std::string_view fileName)
    {
        return Common::saveToXMLFile(model, fileName, "ThermModel");
    }
}
