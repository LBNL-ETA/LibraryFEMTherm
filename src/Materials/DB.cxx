#include <algorithm>
#include <fstream>
#include <set>

#include <lbnl/algorithm.hxx>
#include <fileParse/XMLNodeAdapter.hxx>

#include "DB.hxx"
#include "Serializers.hxx"
#include "Tags.hxx"

#include "Common/Common.hxx"

#include "LibraryUtilities/FileManipulation.hxx"
#include "LibraryUtilities/Common.hxx"

namespace MaterialsLibrary
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    ///   DB
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    DB::DB(const std::string & xmlFileName) : m_FileName(xmlFileName)
    {
        // Check if the file exists and create an empty one if it doesn't.
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

    void DB::loadFromXMLString(const std::string & xmlString)
    {
        using MaterialsLibrary::operator>>;

        auto node{getTopNodeFromString(xmlString, MaterialsLibrary::materialsString())};
        if(node.has_value())
        {
            node.value() >> FileParse::Child{"Version", m_Version};
            node.value() >> FileParse::Child{MaterialsLibrary::materialString(), m_Materials};
        }
    }

    std::string DB::saveToXMLString() const
    {
        using MaterialsLibrary::operator<<;

        XMLNodeAdapter node{createTopNode(MaterialsLibrary::materialsString())};

        node << FileParse::Child{"Version", m_Version};
        node << FileParse::Child{MaterialsLibrary::materialString(), m_Materials};

        return node.getContent();
    }

    std::optional<Material> DB::getByName(std::string_view materialName)
    {
        return getMaterialByPredicate([&materialName](const Material & mat) { return mat.Name == materialName; });
    }


    std::optional<Material> DB::getByDisplayName(std::string_view materialName)
    {
        return getMaterialByPredicate(
          [&materialName](const Material & mat) { return LibraryCommon::DisplayName(mat) == materialName; });
    }

    Material DB::getDefaultRecord() const
    {
        return m_Materials[0];
    }

    std::optional<Material> DB::getByUUID(std::string_view uuid) const
    {
        return lbnl::find_element(m_Materials, [&uuid](const Material & mat) { return mat.UUID == uuid; });
    }

    std::vector<std::string> DB::getNames() const
    {
        return lbnl::transform_to_vector(m_Materials, [](const Material & mat) { return mat.Name; });
    }

    std::vector<std::string> DB::getDisplayNames() const
    {
        return lbnl::transform_to_vector(m_Materials, [](const Material & m) { return LibraryCommon::DisplayName(m); });
    }

    void DB::add(const Material & material)
    {
        m_Materials.emplace_back(material);
    }

    void DB::update(const Material & material)
    {
        m_Materials = lbnl::transform_if(m_Materials,
                                         [&](const Material & m) { return m.UUID == material.UUID; },
                                         [&](const Material &) { return material; });
    }

    void DB::updateOrAdd(const Material & material)
    {
        const auto mat{getByUUID(material.UUID)};
        mat.has_value() ? update(material) : add(material);
    }

    void DB::deleteWithUUID(std::string_view uuid)
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

    void DB::deleteRecordsWithProjectName(std::string_view projectName)
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

    std::set<std::string, std::less<>> getNames(const std::vector<Material> & materials)
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
