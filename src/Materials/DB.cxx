#include <algorithm>
#include <fstream>
#include <set>

#include <lbnl/algorithm.hxx>
#include <fileParse/FileDataHandler.hxx>

#include "DB.hxx"
#include "Operators.hxx"
#include "Serializers.hxx"
#include "Tags.hxx"

#include "Common/Common.hxx"
#include "Common/DB.hxx"

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
            const std::string fileContent{Common::generateLibraryContent(
              "Materials", "Materials.xsd", m_Version, FileParse::detectFileFormatFromExtension(xmlFileName))};
            File::createFileFromString(xmlFileName, fileContent);
        }

        m_Materials = loadMaterialsFromXMLFile(xmlFileName);
    }

    void DB::loadFromString(const std::string & str)
    {
        using MaterialsLibrary::operator>>;

        auto node{Common::getTopNodeFromString(Common::stripUTF8BOM(str), materialsString())};
        if(node.has_value())
        {
            std::visit(
              [this](auto & adapter) {
                  adapter >> FileParse::Child{"Version", m_Version};
                  adapter >> FileParse::Child{materialString(), m_Materials};
              },
              node.value());
        }
    }

    std::string DB::saveToString(FileParse::FileFormat format) const
    {
        using MaterialsLibrary::operator<<;

        auto node{Common::createTopNode(materialsString(), format)};

        std::string content;
        std::visit(
          [this, &content](auto & adapter) {
              adapter << FileParse::Child{"Version", m_Version};
              adapter << FileParse::Child{materialString(), m_Materials};
              content = adapter.getContent();
          },
          node);

        return content;
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

    std::optional<Material> DB::getDefaultRecord() const
    {
        if(m_DefaultRecordIndex >= m_Materials.size())
        {
            return std::nullopt;
        }
        return m_Materials[m_DefaultRecordIndex];
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
        m_Dirty = true;
    }

    void DB::update(const Material & material)
    {
        for(auto & existing : m_Materials)
        {
            if(existing.UUID == material.UUID && !(existing == material))
            {
                existing = material;
                m_Dirty = true;
            }
        }
    }

    void DB::updateOrAdd(const Material & material)
    {
        const auto mat{getByUUID(material.UUID)};
        mat.has_value() ? update(material) : add(material);
    }

    void DB::deleteWithUUID(std::string_view uuid)
    {
        const size_t sizeBefore{m_Materials.size()};
        m_Materials.erase(std::remove_if(std::begin(m_Materials),
                                         std::end(m_Materials),
                                         [&](Material const & u) { return u.UUID == uuid; }),
                          std::end(m_Materials));
        m_Dirty = m_Dirty || m_Materials.size() != sizeBefore;
    }

    int DB::saveToFile(FileParse::FileFormat format) const
    {
        using MaterialsLibrary::operator<<;

        auto node{Common::createTopNode(MaterialsLibrary::materialsString(), format)};

        int result = 0;
        std::visit(
          [this, &result](auto & adapter) {
              adapter << FileParse::Child{"Version", m_Version};
              adapter << FileParse::Child{MaterialsLibrary::materialString(), m_Materials};
              result = adapter.writeToFile(m_FileName);
          },
          node);

        return result;
    }

    std::string DB::getFileName() const
    {
        return m_FileName;
    }

    std::vector<Material> DB::loadMaterialsFromXMLFile(const std::string & materialXMLFileName)
    {
        using MaterialsLibrary::operator>>;

        auto topNode{Common::getLibraryTopNodeFromFile(materialXMLFileName, materialsString())};

        std::vector<Material> materials;

        if(topNode.has_value())
        {
            std::visit(
              [this, &materials](auto & adapter) {
                  adapter >> FileParse::Child{"Version", m_Version};
                  adapter >> FileParse::Child{materialString(), materials};
              },
              topNode.value());
        }

        return materials;
    }

    std::vector<Material> & DB::getMaterials()
    {
        return m_Materials;
    }

    const std::vector<Material> & DB::getMaterials() const
    {
        return m_Materials;
    }

    void DB::deleteRecordsWithProjectName(std::string_view projectName)
    {
        const size_t sizeBefore{m_Materials.size()};
        m_Materials.erase(
          std::ranges::remove_if(m_Materials, [&](Material const & u) { return u.ProjectName == projectName; }).begin(),
          std::end(m_Materials));
        m_Dirty = m_Dirty || m_Materials.size() != sizeBefore;
    }

    void DB::deleteTemporaryRecords()
    {
        const size_t sizeBefore{m_Materials.size()};
        LibraryCommon::removeTemporaryRecords(m_Materials);
        m_Dirty = m_Dirty || m_Materials.size() != sizeBefore;
    }

    void DB::renameRecordsWithProjectName(std::string_view oldName, std::string_view newName)
    {
        for(auto & material : m_Materials)
        {
            if(material.ProjectName == oldName)
            {
                material.ProjectName = std::string{newName};
                m_Dirty = true;
            }
        }
    }

    bool DB::isDirty() const
    {
        return m_Dirty;
    }

    int DB::saveIfDirty(FileParse::FileFormat format)
    {
        if(!m_Dirty)
        {
            return 0;
        }

        const int result{saveToFile(format)};
        if(result == 0)
        {
            m_Dirty = false;
        }

        return result;
    }

    void DB::setDefaultRecord(std::string_view materialName)
    {
        auto iter = std::ranges::find_if(
          m_Materials, [&materialName](const Material & mat) { return LibraryCommon::DisplayName(mat) == materialName; });

        if(iter != m_Materials.end())
        {
            m_DefaultRecordIndex = static_cast<size_t>(std::distance(m_Materials.begin(), iter));
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

}   // namespace MaterialsLibrary
