#include "DB.hxx"

#include "Tags.hxx"
#include "Serializers.hxx"

#include "Common/DB.hxx"

namespace CMALibrary
{
    CMAData loadCMADataFromFile(const std::string & fileName)
    {
        CMALibrary::Tags tags;
        return Common::loadFromXMLFile<CMAData>(fileName, tags.rootTag());
    }

    CMAData loadCMADataFromZipFile(const std::string & fileName)
    {
        CMALibrary::Tags tags;
        return Common::loadFromZipFile<CMAData>(fileName, ThermZip::CMALibrary, tags.rootTag());
    }
}   // namespace CMALibrary