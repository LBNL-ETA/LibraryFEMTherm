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

    int saveToFile(const Mesh & model, std::string_view fileName)
    {
        return Common::saveToXMLFile(model, fileName, "Mesh");
    }
}
