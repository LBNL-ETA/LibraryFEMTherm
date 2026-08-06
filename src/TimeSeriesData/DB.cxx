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

namespace TimeSeriesLibrary
{
    DB::DB(const std::string & xmlFileName) : m_FileName(xmlFileName)
    {
        if(std::ifstream file(xmlFileName.c_str()); !file.good())
        {
            Tags tags;
            const std::string fileContent{
              Common::generateLibraryContent(tags.timeSeriesData(),
                                             "TimeSeriesData.xsd",
                                             m_Version,
                                             FileParse::detectFileFormatFromExtension(xmlFileName))};
            File::createFileFromString(xmlFileName, fileContent);
        }

        m_TimeSeriesData = loadTimeSeriesDataFromFile(xmlFileName);
    }

    void DB::loadFromString(const std::string & str)
    {
        Tags tags;
        auto node{Common::getTopNodeFromString(Common::stripUTF8BOM(str), tags.timeSeriesData())};

        if(node.has_value())
        {
            std::visit(
              [this, &tags](auto & adapter) {
                  adapter >> FileParse::Child{tags.version(), m_Version};
                  adapter >> FileParse::Child{tags.timeSeries(), m_TimeSeriesData};
              },
              node.value());
        }
    }

    std::string DB::saveToString(FileParse::FileFormat format) const
    {
        Tags tags;
        auto node{Common::createTopNode(tags.timeSeriesData(), format)};

        std::string content;
        std::visit(
          [this, &tags, &content](auto & adapter) {
              adapter << FileParse::Child{tags.version(), m_Version};
              adapter << FileParse::Child{tags.timeSeries(), m_TimeSeriesData};
              content = adapter.getContent();
          },
          node);

        return content;
    }

    int DB::saveToFile(FileParse::FileFormat format) const
    {
        Tags tags;
        auto node{Common::createTopNode(tags.timeSeriesData(), format)};

        int result = 0;
        std::visit(
          [this, &tags, &result](auto & adapter) {
              adapter << FileParse::Child{tags.version(), m_Version};
              adapter << FileParse::Child{tags.timeSeries(), m_TimeSeriesData};
              result = adapter.writeToFile(m_FileName);
          },
          node);

        return result;
    }

    std::vector<TimeSeriesData> DB::loadTimeSeriesDataFromFile(const std::string & xmlFileName)
    {
        Tags tags;
        auto topNode{Common::getLibraryTopNodeFromFile(xmlFileName, tags.timeSeriesData())};

        std::vector<TimeSeriesData> records;
        if(topNode.has_value())
        {
            std::visit(
              [this, &tags, &records](auto & adapter) {
                  adapter >> FileParse::Child{tags.version(), m_Version};
                  adapter >> FileParse::Child{tags.timeSeries(), records};
              },
              topNode.value());
        }

        return records;
    }

    std::optional<TimeSeriesData> DB::getByUUID(std::string_view uuid) const
    {
        return lbnl::find_element(m_TimeSeriesData,
                                  [uuid](const TimeSeriesData & data) { return data.UUID == uuid; });
    }

    std::optional<TimeSeriesData> DB::getByName(std::string_view name) const
    {
        return lbnl::find_element(m_TimeSeriesData,
                                  [name](const TimeSeriesData & data) { return data.Name == name; });
    }

    std::vector<TimeSeriesData> & DB::getTimeSeriesData()
    {
        return m_TimeSeriesData;
    }

    std::vector<std::string> DB::getNames() const
    {
        return lbnl::transform_to_vector(m_TimeSeriesData,
                                         [](const TimeSeriesData & data) { return data.Name; });
    }

    std::string DB::getFileName() const
    {
        return m_FileName;
    }

    void DB::add(const TimeSeriesData & data)
    {
        m_TimeSeriesData.emplace_back(data);
    }

    void DB::update(const TimeSeriesData & data)
    {
        for(auto & existing : m_TimeSeriesData)
        {
            if(existing.UUID == data.UUID)
            {
                existing = data;
            }
        }
    }

    void DB::updateOrAdd(const TimeSeriesData & data)
    {
        const auto existing{getByUUID(data.UUID)};
        existing.has_value() ? update(data) : add(data);
    }

    void DB::deleteWithUUID(std::string_view uuid)
    {
        m_TimeSeriesData.erase(std::ranges::remove_if(m_TimeSeriesData,
                                                       [uuid](const TimeSeriesData & data) {
                                                           return data.UUID == uuid;
                                                       })
                                  .begin(),
                                m_TimeSeriesData.end());
    }

    void DB::deleteRecordsWithProjectName(std::string_view projectName)
    {
        m_TimeSeriesData.erase(std::ranges::remove_if(m_TimeSeriesData,
                                                       [projectName](const TimeSeriesData & data) {
                                                           return data.ProjectName == projectName;
                                                       })
                                  .begin(),
                                m_TimeSeriesData.end());
    }

    void DB::deleteTemporaryRecords()
    {
        LibraryCommon::removeTemporaryRecords(m_TimeSeriesData);
    }

    std::vector<TimeSeriesData> loadDatasetsFromZipFile(const std::string & zipFileName)
    {
        std::vector<TimeSeriesData> datasets;
        if(!std::filesystem::exists(zipFileName))
        {
            return datasets;
        }

        const std::string entryPrefix{ThermZip::TimeSeriesDir + "/"};
        for(const auto & [entryName, content] : ThermZip::unzipFiles(zipFileName))
        {
            if(entryName.starts_with(entryPrefix))
            {
                DB entryDB;
                entryDB.loadFromString(content);
                std::ranges::copy(entryDB.getTimeSeriesData(), std::back_inserter(datasets));
            }
        }

        return datasets;
    }

    int saveDatasetsToZipFile(const std::vector<TimeSeriesData> & datasets,
                              const std::string & zipFileName,
                              FileParse::FileFormat format)
    {
        int written = 0;
        for(const auto & dataset : datasets)
        {
            DB entryDB;
            entryDB.add(dataset);
            const auto entryName{ThermZip::timeSeriesEntryName(dataset.UUID, format)};
            auto obsoleteNames{ThermZip::entryNameCandidates(ThermZip::timeSeriesEntryName(dataset.UUID))};
            std::erase(obsoleteNames, entryName);
            written += ThermZip::addToZipFile(zipFileName, entryName, entryDB.saveToString(format), obsoleteNames);
        }

        return written;
    }
}   // namespace TimeSeriesLibrary
