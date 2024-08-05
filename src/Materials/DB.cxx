#include <algorithm>
#include <fstream>
#include <set>

#include <fileParse/XMLNodeAdapter.hxx>

#include "DB.hxx"
#include "Serializers.hxx"
#include "Tags.hxx"

#include "Common/Common.hxx"

#include "LibraryUtilities/FileManipulation.hxx"
#include "LibraryUtilities/Common.hxx"
#include "LibraryUtilities/StringRoutines.hxx"

namespace MaterialsLibrary
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    ///   DB
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    DB::DB(const std::string & xmlFileName) : m_FileName(xmlFileName)
    {
        // Check if file exists and create an empty one if it doesn't.
        if(std::ifstream f(xmlFileName); !f.good())
        {
            const std::string fileContent{Common::generateXmlContent("Materials", "Materials.xsd", m_Version)};
            File::createFileFromString(xmlFileName, fileContent);
        }

        try
        {
            m_Materials = loadMaterialsFromXMLFile(xmlFileName);
        }
        catch(...)
        {
            throw;
        }
    }

    std::optional<Material> DB::getMaterialByName(std::string_view materialName)
    {
        return getMaterialByPredicate([&materialName](const Material & mat) { return mat.Name == materialName; });
    }


    std::optional<Material> DB::getMaterialByDisplayName(std::string_view materialName)
    {
        return getMaterialByPredicate(
          [&materialName](const Material & mat) { return LibraryCommon::DisplayName(mat) == materialName; });
    }

    Material DB::getDefaultRecord() const
    {
        return m_Materials[0];
    }

    std::optional<Material> DB::getMaterialByUUID(const std::string & uuid)
    {
        std::optional<Material> result;

        if(auto it = std::find_if(
             m_Materials.begin(), m_Materials.end(), [&](const Material & obj) { return obj.UUID == uuid; });
           it != m_Materials.end())
        {
            result = *it;
        }

        if(uuid.empty())
        {
            result = m_Materials[m_DefaultRecordIndex];
        }

        return result;
    }

    std::vector<std::string> DB::getMaterialNames() const
    {
        std::vector<std::string> names;
        names.reserve(m_Materials.size());

        for(const auto & material : m_Materials)
        {
            names.push_back(material.Name);
        }

        return names;
    }

    std::vector<std::string> DB::getMaterialDisplayNames() const
    {
        std::vector<std::string> names;
        names.reserve(m_Materials.size());

        for(const auto & material : m_Materials)
        {
            names.push_back(LibraryCommon::DisplayName(material));
        }
        return names;
    }

    void DB::addMaterial(const Material & material)
    {
        m_Materials.emplace_back(material);
    }

    void DB::updateMaterial(const Material & material)
    {
        for(auto & mat : m_Materials)
        {
            if(mat.UUID == material.UUID)
            {
                mat = material;
            }
        }
    }

    void DB::updateOrAddMaterial(const Material & material)
    {
        const auto mat{getMaterialByUUID(material.UUID)};
        mat.has_value() ? updateMaterial(material) : addMaterial(material);
    }

    void DB::deleteMaterialWithUUID(std::string_view uuid)
    {
        m_Materials.erase(std::remove_if(std::begin(m_Materials),
                                         std::end(m_Materials),
                                         [&](Material const & u) { return u.UUID == uuid; }),
                          std::end(m_Materials));
    }

    int DB::saveToFile() const
    {
        using MaterialsLibrary::operator<<;

        XMLNodeAdapter node{createTopNode(MaterialsLibrary::materialsString())};

        node << FileParse::Child{"Version", m_Version};
        node << FileParse::Child{MaterialsLibrary::materialString(), m_Materials};

        return node.writeToFile(m_FileName);
    }

    std::string DB::getFileName() const
    {
        return m_FileName;
    }

    std::vector<Material> DB::loadMaterialsFromXMLFile(const std::string & materialXMLFileName)
    {
        using MaterialsLibrary::operator>>;

        const auto xmlNode{getTopNodeFromFile(materialXMLFileName, MaterialsLibrary::materialsString())};

        std::vector<Material> materials;

        if(xmlNode.has_value())
        {
            xmlNode.value() >> FileParse::Child{"Version", m_Version};
            xmlNode.value() >> FileParse::Child{MaterialsLibrary::materialString(), materials};
        }

        return materials;
    }

    std::vector<Material> & DB::getMaterials()
    {
        return m_Materials;
    }

    void DB::deleteRecordsWithProjectName(const std::string & projectName)
    {
        m_Materials.erase(std::remove_if(std::begin(m_Materials),
                                         std::end(m_Materials),
                                         [&](Material const & u) { return u.ProjectName == projectName; }),
                          std::end(m_Materials));
    }

    void DB::setDefaultRecord(std::string_view materialName)
    {
        size_t index = 0;
        for(const auto & material : m_Materials)
        {
            if(material.Name == materialName)
            {
                m_DefaultRecordIndex = index;
            }
            ++index;
        }
    }

    std::optional<Material> DB::getMaterialByPredicate(std::function<bool(const Material &)> predicate) const
    {
        auto it = std::find_if(m_Materials.begin(), m_Materials.end(), predicate);

        if(it != m_Materials.end())
        {
            return *it;
        }

        return std::nullopt;
    }

    std::set<std::string, std::less<>> getGasNames(const std::vector<Material> & materials)
    {
        std::set<std::string, std::less<>> gasNames;
        for(const auto & material : materials)
        {
            if(MaterialsLibrary::isCavity(material))
            {
                gasNames.insert(MaterialsLibrary::getCavity(material)->GasName);
            }
        }

        return gasNames;
    }
}   // namespace MaterialsLibrary
