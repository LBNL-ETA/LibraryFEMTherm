#pragma once

#include <map>
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

        //! True once a mutator has effectively changed the records since load or the last
        //! successful saveIfDirty. Mutations that change nothing (updating a record with an
        //! identical copy, deleting by a key that matches no record) do not set it.
        [[nodiscard]] bool isDirty() const;

        //! saveToFile gated on isDirty: returns 0 without touching the file when clean;
        //! otherwise saves and clears the flag only when the save reports success (0).
        int saveIfDirty(FileParse::FileFormat format = FileParse::FileFormat::XML);

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
        //! Effective-mutation flag behind isDirty/saveIfDirty. Known bypass: the non-const
        //! getTimeSeriesData() hands out a mutable reference the flag cannot see; its call
        //! sites are read-only today, and the signature stays because the templated list
        //! dialog base relies on the shared DB shape.
        bool m_Dirty{false};

        [[nodiscard]] std::vector<TimeSeriesData> loadTimeSeriesDataFromFile(const std::string & xmlFileName);
    };

    //! THMZ storage: each dataset is its own archive entry "time series/<uuid>.xml",
    //! shaped as a one-record library file so the library schema validates it. An archive
    //! without such entries yields an empty vector (pre-consolidation file).
    [[nodiscard]] std::vector<TimeSeriesData> loadDatasetsFromZipFile(const std::string & zipFileName);

    //! Same, but from already-extracted archive entries (no further extraction).
    [[nodiscard]] std::vector<TimeSeriesData> loadDatasetsFromEntries(const std::map<std::string, std::string> & entries);

    //! Writes every dataset as its own entry; returns the number of entries written.
    int saveDatasetsToZipFile(const std::vector<TimeSeriesData> & datasets,
                              const std::string & zipFileName,
                              FileParse::FileFormat format = FileParse::FileFormat::XML);
}   // namespace TimeSeriesLibrary