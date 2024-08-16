#include "DB.hxx"

#include "Mesh.hxx"
#include "Serializers.hxx"

#include "Common/DB.hxx"

namespace ThermFile::Mesh
{
    Mesh loadMeshFromFile(std::string_view fileName)
    {
        return Common::loadFromXMLFile<Mesh>(fileName, "Mesh");
    }

    Mesh loadMeshFromZipFile(std::string const& fileName)
    {
        return Common::loadFromZipFile<Mesh>(fileName, ThermZip::MeshName, "Mesh");
    }

    int saveToFile(const Mesh & model, std::string_view fileName)
    {
        return Common::saveToXMLFile(model, fileName, "Mesh");
    }
}
