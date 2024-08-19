#include "DB.hxx"

#include "Mesh.hxx"
#include "Serializers.hxx"

#include "Common/DB.hxx"

namespace ThermFile::Mesh
{
    const std::string topNodeName = "Mesh";

    Mesh loadMeshFromFile(std::string_view fileName)
    {
        return Common::loadFromXMLFile<Mesh>(fileName, topNodeName);
    }

    Mesh loadMeshFromZipFile(std::string const& fileName)
    {
        return Common::loadFromZipFile<Mesh>(fileName, ThermZip::MeshName, topNodeName);
    }

    int saveToFile(const Mesh & model, std::string_view fileName)
    {
        return Common::saveToXMLFile(model, fileName, topNodeName);
    }

    std::string saveToString(const Mesh & mesh)
    {
        return Common::saveToXMLString(mesh, topNodeName);
    }

    int saveToZipFile(const Mesh & mesh, std::string_view zipFileName)
    {
        return Common::saveToZIPFile(mesh, ThermZip::SteadyStateResultsName, zipFileName, topNodeName);
    }
}
