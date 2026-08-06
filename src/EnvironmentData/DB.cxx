#include <algorithm>
#include <filesystem>
#include <fstream>

#include <fileParse/FileDataHandler.hxx>
#include <fileParse/Vector.hxx>

#include <lbnl/algorithm.hxx>

#include "DB.hxx"

#include "Serializers.hxx"
#include "Tags.hxx"

#include "Common/Common.hxx"
#include "Common/DB.hxx"
#include "LibraryUtilities/Common.hxx"
#include "LibraryUtilities/FileManipulation.hxx"
#include "THMZ/ZipModule/ZipModule.hxx"

namespace EnvironmentDataLibrary
{
    DB::DB(const std::string & xmlFileName) : m_FileName(xmlFileName)
    {
        if(std::ifstream file(xmlFileName.c_str()); !file.good())
        {
            Tags tags;
            const std::string fileContent{
              Common::generateLibraryContent(tags.environmentData(),
                                             "EnvironmentData.xsd",
                                             m_Version,
                                             FileParse::detectFileFormatFromExtension(xmlFileName))};
            File::createFileFromString(xmlFileName, fileContent);
        }

        m_EnvironmentData = loadEnvironmentDataFromFile(xmlFileName);
    }

    void DB::loadFromString(const std::string & str)
    {
        Tags tags;
        auto node{Common::getTopNodeFromString(Common::stripUTF8BOM(str), tags.environmentData())};

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
        auto topNode{Common::getLibraryTopNodeFromFile(xmlFileName, tags.environmentData())};

        std::vector<EnvironmentData> environments;
        if(topNode.has_value())
        {
            std::visit(
              [this, &tags, &environments](auto & adapter) {
                  adapter >> FileParse::Child{tags.version(), m_Version};
                  adapter >> FileParse::Child{tags.environment(), environments};
              },
              topNode.value());
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

    std::vector<EnvironmentData> loadDatasetsFromZipFile(const std::string & zipFileName)
    {
        std::vector<EnvironmentData> datasets;
        if(!std::filesystem::exists(zipFileName))
        {
            return datasets;
        }

        const std::string entryPrefix{ThermZip::EnvironmentDataDir + "/"};
        for(const auto & [entryName, content] : ThermZip::unzipFiles(zipFileName))
        {
            if(entryName.starts_with(entryPrefix))
            {
                DB entryDB;
                entryDB.loadFromString(content);
                std::ranges::copy(entryDB.getEnvironmentData(), std::back_inserter(datasets));
            }
        }

        return datasets;
    }

    int saveDatasetsToZipFile(const std::vector<EnvironmentData> & datasets,
                              const std::string & zipFileName,
                              FileParse::FileFormat format)
    {
        int written = 0;
        for(const auto & dataset : datasets)
        {
            DB entryDB;
            entryDB.add(dataset);
            const auto entryName{ThermZip::environmentDataEntryName(dataset.UUID, format)};
            auto obsoleteNames{ThermZip::entryNameCandidates(ThermZip::environmentDataEntryName(dataset.UUID))};
            std::erase(obsoleteNames, entryName);
            written += ThermZip::addToZipFile(zipFileName, entryName, entryDB.saveToString(format), obsoleteNames);
        }

        return written;
    }
}   // namespace EnvironmentDataLibrary
