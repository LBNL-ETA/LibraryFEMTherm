#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <fileParse/FileFormat.hxx>

#include "TimeSeriesData.hxx"

namespace TimeSeriesLibrary
{
    class DB
    {
    public:
        DB() = default;
        explicit DB(const std::string & xmlFileName);

        void loadFromString(const std::string & str);
        [[nodiscard]] std::string saveToString(FileParse::FileFormat format = FileParse::FileFormat::XML) const;
        [[nodiscard]] int saveToFile(FileParse::FileFormat format = FileParse::FileFormat::XML) const;

        [[nodiscard]] std::optional<TimeSeriesData> getByUUID(std::string_view uuid) const;
        [[nodiscard]] std::optional<TimeSeriesData> getByName(std::string_view name) const;

        [[nodiscard]] std::vector<TimeSeriesData> & getTimeSeriesData();
        [[nodiscard]] std::vector<std::string> getNames() const;
        [[nodiscard]] std::string getFileName() const;

        void add(const TimeSeriesData & data);
        void update(const TimeSeriesData & data);
        void updateOrAdd(const TimeSeriesData & data);
        void deleteWithUUID(std::string_view uuid);

        void deleteRecordsWithProjectName(std::string_view projectName);
        void deleteTemporaryRecords();

    private:
        std::string m_FileName;
        std::vector<TimeSeriesData> m_TimeSeriesData;
        std::string m_Version{"1"};

        [[nodiscard]] std::vector<TimeSeriesData> loadTimeSeriesDataFromFile(const std::string & xmlFileName);
    };

    //! THMZ storage: each dataset is its own archive entry "time series/<uuid>.xml",
    //! shaped as a one-record library file so the library schema validates it. An archive
    //! without such entries yields an empty vector (pre-consolidation file).
    [[nodiscard]] std::vector<TimeSeriesData> loadDatasetsFromZipFile(const std::string & zipFileName);

    //! Writes every dataset as its own entry; returns the number of entries written.
    int saveDatasetsToZipFile(const std::vector<TimeSeriesData> & datasets,
                              const std::string & zipFileName,
                              FileParse::FileFormat format = FileParse::FileFormat::XML);
}   // namespace TimeSeriesLibrary