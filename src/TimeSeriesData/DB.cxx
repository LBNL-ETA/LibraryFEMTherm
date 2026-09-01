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
        m_Dirty = true;
    }

    void DB::update(const TimeSeriesData & data)
    {
        for(auto & existing : m_TimeSeriesData)
        {
            if(existing.UUID == data.UUID && existing != data)
            {
                existing = data;
                m_Dirty = true;
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
        const size_t sizeBefore{m_TimeSeriesData.size()};
        m_TimeSeriesData.erase(std::ranges::remove_if(m_TimeSeriesData,
                                                       [uuid](const TimeSeriesData & data) {
                                                           return data.UUID == uuid;
                                                       })
                                  .begin(),
                                m_TimeSeriesData.end());
        m_Dirty = m_Dirty || m_TimeSeriesData.size() != sizeBefore;
    }

    void DB::deleteRecordsWithProjectName(std::string_view projectName)
    {
        const size_t sizeBefore{m_TimeSeriesData.size()};
        m_TimeSeriesData.erase(std::ranges::remove_if(m_TimeSeriesData,
                                                       [projectName](const TimeSeriesData & data) {
                                                           return data.ProjectName == projectName;
                                                       })
                                  .begin(),
                                m_TimeSeriesData.end());
        m_Dirty = m_Dirty || m_TimeSeriesData.size() != sizeBefore;
    }

    void DB::deleteTemporaryRecords()
    {
        const size_t sizeBefore{m_TimeSeriesData.size()};
        LibraryCommon::removeTemporaryRecords(m_TimeSeriesData);
        m_Dirty = m_Dirty || m_TimeSeriesData.size() != sizeBefore;
    }

    std::vector<TimeSeriesData> loadDatasetsFromZipFile(const std::string & zipFileName)
    {
        if(!std::filesystem::exists(zipFileName))
        {
            return {};
        }

        return loadDatasetsFromEntries(ThermZip::unzipFiles(zipFileName));
    }

    std::vector<TimeSeriesData> loadDatasetsFromEntries(const std::map<std::string, std::string> & entries)
    {
        std::vector<TimeSeriesData> datasets;

        const std::string entryPrefix{ThermZip::TimeSeriesDir + "/"};
        for(const auto & [entryName, content] : entries)
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
        // One extract and one compressed rewrite for the whole batch; the per-dataset
        // addToZipFile alternative costs a full archive rewrite per dataset.
        auto entries{std::filesystem::exists(zipFileName) ? ThermZip::unzipFiles(zipFileName)
                                                          : std::map<std::string, std::string>{}};

        for(const auto & dataset : datasets)
        {
            const auto baseName{ThermZip::TimeSeriesDir + "/" + dataset.UUID};
            for(const auto & staleName : ThermZip::entryNameCandidates(baseName))
            {
                entries.erase(staleName);
            }

            DB entryDB;
            entryDB.add(dataset);
            entries[ThermZip::timeSeriesEntryName(dataset.UUID, format)] = entryDB.saveToString(format);
        }

        ThermZip::zipFiles(entries, zipFileName);

        return static_cast<int>(datasets.size());
    }
}   // namespace TimeSeriesLibrary
