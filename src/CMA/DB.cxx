#include "DB.hxx"

#include "Tags.hxx"
#include "Serializers.hxx"

#include "Common/DB.hxx"

namespace CMALibrary
{
    std::optional<CMAData> loadCMADataFromFile(const std::string & fileName)
    {
        CMALibrary::Tags tags;
        return Common::loadFromXMLFile<CMAData>(fileName, tags.rootTag());
    }

    std::optional<CMAData> loadCMADataFromZipFile(const std::string & fileName)
    {
        CMALibrary::Tags tags;
        return Common::loadFromZipFile<CMAData>(fileName, ThermZip::CMALibrary, tags.rootTag());
    }
}   // namespace CMALibrary