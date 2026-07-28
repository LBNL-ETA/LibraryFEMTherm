#include <filesystem>
#include <fstream>

#include <fileParse/FileDataHandler.hxx>
#include <fileParse/Vector.hxx>

#include <lbnl/algorithm.hxx>

#include "THMZ/ZipModule/ZipModule.hxx"

#include "DB.hxx"

#include "Serializers.hxx"
#include "Tags.hxx"

#include "Common/Common.hxx"
#include "LibraryUtilities/Common.hxx"
#include "LibraryUtilities/FileManipulation.hxx"

namespace BCLibrary
{
    DB::DB(const std::string & xmlFileName) : m_FileName(xmlFileName)
    {
        if(std::ifstream file(xmlFileName.c_str()); !file.good())
        {
            Tags tags;
            const std::string fileContent{
              Common::generateXmlContent(tags.boundaryConditions(), "BoundaryConditions.xsd", m_Version)};
            File::createFileFromString(xmlFileName, fileContent);
        }

        m_BoundaryConditions = loadBoundaryConditionsFromFile(xmlFileName);
    }

    void DB::loadFromString(const std::string & str)
    {
        Tags tags;
        auto node{Common::getTopNodeFromString(str, tags.boundaryConditions())};

        if(node.has_value())
        {
            std::visit(
              [this, &tags](auto & adapter) {
                  adapter >> FileParse::Child{tags.version(), m_Version};
                  adapter >> FileParse::Child{tags.boundaryCondition(), m_BoundaryConditions};
              },
              node.value());
        }
    }

    std::string DB::saveToString(FileParse::FileFormat format) const
    {
        Tags tags;
        auto node{Common::createTopNode(tags.boundaryConditions(), format)};

        std::string content;
        std::visit(
          [this, &tags, &content](auto & adapter) {
              adapter << FileParse::Child{tags.version(), m_Version};
              adapter << FileParse::Child{tags.boundaryCondition(), m_BoundaryConditions};
              content = adapter.getContent();
          },
          node);

        return content;
    }

    void DB::loadFromZipFile(const std::string & zipFileName)
    {
        if(!std::filesystem::exists(zipFileName))
        {
            return;
        }

        try
        {
            loadFromString(ThermZip::unzipFile(zipFileName, ThermZip::BoundaryConditionsFileName));
        }
        catch(const std::runtime_error &)
        {
            // Entry absent: pre-consolidation archive, nothing to load.
        }
    }

    int DB::saveToZipFile(std::string_view zipFileName) const
    {
        return ThermZip::addToZipFile(zipFileName, ThermZip::BoundaryConditionsFileName, saveToString());
    }

    int DB::saveToFile(FileParse::FileFormat format) const
    {
        Tags tags;
        auto node{Common::createTopNode(tags.boundaryConditions(), format)};

        int result = 0;
        std::visit(
          [this, &tags, &result](auto & adapter) {
              adapter << FileParse::Child{tags.version(), m_Version};
              adapter << FileParse::Child{tags.boundaryCondition(), m_BoundaryConditions};
              result = adapter.writeToFile(m_FileName);
          },
          node);

        return result;
    }

    std::vector<BoundaryCondition> DB::loadBoundaryConditionsFromFile(const std::string & xmlFileName)
    {
        Tags tags;
        const auto topNode{getXMLTopNodeFromFile(xmlFileName, tags.boundaryConditions())};

        std::vector<BoundaryCondition> boundaryConditions;
        if(topNode.has_value())
        {
            topNode.value() >> FileParse::Child{tags.version(), m_Version};
            topNode.value() >> FileParse::Child{tags.boundaryCondition(), boundaryConditions};
        }

        return boundaryConditions;
    }

    std::optional<BoundaryCondition> DB::getByUUID(std::string_view uuid) const
    {
        return lbnl::find_element(
          m_BoundaryConditions, [uuid](const BoundaryCondition & record) { return record.UUID == uuid; });
    }

    std::optional<BoundaryCondition> DB::getByName(std::string_view name) const
    {
        return lbnl::find_element(
          m_BoundaryConditions, [name](const BoundaryCondition & record) { return record.Name == name; });
    }

    std::optional<BoundaryCondition> DB::getByDisplayName(std::string_view displayName) const
    {
        return lbnl::find_element(m_BoundaryConditions, [displayName](const BoundaryCondition & record) {
            return LibraryCommon::DisplayName(record) == displayName;
        });
    }

    std::vector<BoundaryCondition> & DB::getBoundaryConditions()
    {
        return m_BoundaryConditions;
    }

    std::vector<std::string> DB::getNames() const
    {
        return lbnl::transform_to_vector(m_BoundaryConditions,
                                         [](const BoundaryCondition & record) { return record.Name; });
    }

    std::vector<std::string> DB::getDisplayNames() const
    {
        return lbnl::transform_to_vector(
          m_BoundaryConditions,
          [](const BoundaryCondition & record) { return LibraryCommon::DisplayName(record); });
    }

    std::string DB::getFileName() const
    {
        return m_FileName;
    }

    void DB::add(const BoundaryCondition & condition)
    {
        m_BoundaryConditions.emplace_back(condition);
    }

    void DB::update(const BoundaryCondition & condition)
    {
        for(auto & record : m_BoundaryConditions)
        {
            if(record.UUID == condition.UUID)
            {
                record = condition;
            }
        }
    }

    void DB::updateOrAdd(const BoundaryCondition & condition)
    {
        const auto record{getByUUID(condition.UUID)};
        record.has_value() ? update(condition) : add(condition);
    }

    void DB::deleteWithUUID(std::string_view uuid)
    {
        m_BoundaryConditions.erase(std::ranges::remove_if(m_BoundaryConditions,
                                                          [uuid](const BoundaryCondition & record) {
                                                              return record.UUID == uuid;
                                                          })
                                     .begin(),
                                   m_BoundaryConditions.end());
    }

    void DB::deleteRecordsWithProjectName(std::string_view projectName)
    {
        m_BoundaryConditions.erase(std::ranges::remove_if(m_BoundaryConditions,
                                                          [projectName](const BoundaryCondition & record) {
                                                              return record.ProjectName == projectName;
                                                          })
                                     .begin(),
                                   m_BoundaryConditions.end());
    }

    void DB::deleteTemporaryRecords()
    {
        LibraryCommon::removeTemporaryRecords(m_BoundaryConditions);
    }

    std::optional<BoundaryCondition> DB::getDefaultRadiationSurface() const
    {
        return lbnl::find_element(m_BoundaryConditions, [](const BoundaryCondition & record) {
            const auto * surface{std::get_if<RadiationSurface>(&record.data)};
            return surface != nullptr && surface->isDefault;
        });
    }

    std::optional<BoundaryCondition> DB::getDefaultRecord() const
    {
        if(m_BoundaryConditions.empty())
        {
            return std::nullopt;
        }
        return m_BoundaryConditions[0];
    }
}   // namespace BCLibrary
