#include "DB.hxx"

#include "Mesh.hxx"
#include "Serializers.hxx"

#include "Common/DB.hxx"

namespace ThermFile::Mesh
{
    const std::string topNodeName = "Mesh";

    std::optional<Mesh> loadMeshFromFile(std::string_view fileName)
    {
        return Common::loadFromFile<Mesh>(fileName, topNodeName);
    }

    std::optional<Mesh> loadMeshFromZipFile(std::string const & fileName)
    {
        return Common::loadFromZipFile<Mesh>(fileName, ThermZip::MeshName, topNodeName);
    }

    int saveToFile(const Mesh & model, std::string_view fileName, FileParse::FileFormat format)
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

    std::string saveToString(const Mesh & mesh, FileParse::FileFormat format)
    {
        return Common::saveToString(mesh, topNodeName, format);
    }

    int saveToZipFile(const Mesh & mesh, std::string_view zipFileName, FileParse::FileFormat format)
    {
        return Common::saveToZIPFile(mesh, ThermZip::MeshName, zipFileName, topNodeName, format);
    }

    std::optional<Mesh> loadMeshFromString(const std::string & str, FileParse::FileFormat format)
    {
        const std::string content{Common::stripUTF8BOM(str)};
        if(format == FileParse::FileFormat::Unknown)
        {
            return Common::loadFromString<Mesh>(content, topNodeName);
        }
        return Common::loadFromString<Mesh>(content, topNodeName, format);
    }
}   // namespace ThermFile::Mesh
