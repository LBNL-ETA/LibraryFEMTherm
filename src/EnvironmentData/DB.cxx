#include <fstream>

#include <fileParse/FileDataHandler.hxx>
#include <fileParse/Vector.hxx>

#include <lbnl/algorithm.hxx>

#include "DB.hxx"

#include "Serializers.hxx"
#include "Tags.hxx"

#include "Common/Common.hxx"
#include "LibraryUtilities/Common.hxx"
#include "LibraryUtilities/FileManipulation.hxx"

namespace EnvironmentDataLibrary
{
    DB::DB(const std::string & xmlFileName) : m_FileName(xmlFileName)
    {
        if(std::ifstream file(xmlFileName.c_str()); !file.good())
        {
            Tags tags;
            const std::string fileContent{
              Common::generateXmlContent(tags.environmentData(), "EnvironmentData.xsd", m_Version)};
            File::createFileFromString(xmlFileName, fileContent);
        }

        m_EnvironmentData = loadEnvironmentDataFromFile(xmlFileName);
    }

    void DB::loadFromString(const std::string & str)
    {
        Tags tags;
        auto node{Common::getTopNodeFromString(str, tags.environmentData())};

        if(node.has_value())
        {
            std::visit(
              [this, &tags](auto & adapter) {
                  adapter >> FileParse::Child{tags.version(), m_Version};
                  adapter >> FileParse::Child{tags.environment(), m_EnvironmentData};
              },
              node.value());
        }
    }

    std::string DB::saveToString(FileParse::FileFormat format) const
    {
        Tags tags;
        auto node{Common::createTopNode(tags.environmentData(), format)};

        std::string content;
        std::visit(
          [this, &tags, &content](auto & adapter) {
              adapter << FileParse::Child{tags.version(), m_Version};
              adapter << FileParse::Child{tags.environment(), m_EnvironmentData};
              content = adapter.getContent();
          },
          node);

        return content;
    }

    int DB::saveToFile(FileParse::FileFormat format) const
    {
        Tags tags;
        auto node{Common::createTopNode(tags.environmentData(), format)};

        int result = 0;
        std::visit(
          [this, &tags, &result](auto & adapter) {
              adapter << FileParse::Child{tags.version(), m_Version};
              adapter << FileParse::Child{tags.environment(), m_EnvironmentData};
              result = adapter.writeToFile(m_FileName);
          },
          node);

        return result;
    }

    std::vector<EnvironmentData> DB::loadEnvironmentDataFromFile(const std::string & xmlFileName)
    {
        Tags tags;
        const auto topNode{getXMLTopNodeFromFile(xmlFileName, tags.environmentData())};

        std::vector<EnvironmentData> environments;
        if(topNode.has_value())
        {
            topNode.value() >> FileParse::Child{tags.version(), m_Version};
            topNode.value() >> FileParse::Child{tags.environment(), environments};
        }

        return environments;
    }

    std::optional<EnvironmentData> DB::getByUUID(std::string_view uuid) const
    {
        return lbnl::find_element(m_EnvironmentData,
                                  [uuid](const EnvironmentData & data) { return data.UUID == uuid; });
    }

    std::optional<EnvironmentData> DB::getByName(std::string_view name) const
    {
        return lbnl::find_element(m_EnvironmentData,
                                  [name](const EnvironmentData & data) { return data.Name == name; });
    }

    std::vector<EnvironmentData> & DB::getEnvironmentData()
    {
        return m_EnvironmentData;
    }

    std::vector<std::string> DB::getNames() const
    {
        return lbnl::transform_to_vector(m_EnvironmentData,
                                         [](const EnvironmentData & data) { return data.Name; });
    }

    std::string DB::getFileName() const
    {
        return m_FileName;
    }

    void DB::add(const EnvironmentData & data)
    {
        m_EnvironmentData.emplace_back(data);
    }

    void DB::update(const EnvironmentData & data)
    {
        for(auto & existing : m_EnvironmentData)
        {
            if(existing.UUID == data.UUID)
            {
                existing = data;
            }
        }
    }

    void DB::updateOrAdd(const EnvironmentData & data)
    {
        const auto existing{getByUUID(data.UUID)};
        existing.has_value() ? update(data) : add(data);
    }

    void DB::deleteWithUUID(std::string_view uuid)
    {
        m_EnvironmentData.erase(std::ranges::remove_if(m_EnvironmentData,
                                                       [uuid](const EnvironmentData & data) {
                                                           return data.UUID == uuid;
                                                       })
                                  .begin(),
                                m_EnvironmentData.end());
    }

    void DB::deleteRecordsWithProjectName(std::string_view projectName)
    {
        m_EnvironmentData.erase(std::ranges::remove_if(m_EnvironmentData,
                                                       [projectName](const EnvironmentData & data) {
                                                           return data.ProjectName == projectName;
                                                       })
                                  .begin(),
                                m_EnvironmentData.end());
    }

    void DB::deleteTemporaryRecords()
    {
        LibraryCommon::removeTemporaryRecords(m_EnvironmentData);
    }
}   // namespace EnvironmentDataLibrary
