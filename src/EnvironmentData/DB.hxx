#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <fileParse/FileFormat.hxx>

#include "EnvironmentData.hxx"

namespace EnvironmentDataLibrary
{
    class DB
    {
    public:
        DB() = default;
        explicit DB(const std::string & xmlFileName);

        void loadFromString(const std::string & str);
        [[nodiscard]] std::string saveToString(FileParse::FileFormat format = FileParse::FileFormat::XML) const;
        [[nodiscard]] int saveToFile(FileParse::FileFormat format = FileParse::FileFormat::XML) const;

        [[nodiscard]] std::optional<EnvironmentData> getByUUID(std::string_view uuid) const;
        [[nodiscard]] std::optional<EnvironmentData> getByName(std::string_view name) const;

        [[nodiscard]] std::vector<EnvironmentData> & getEnvironmentData();
        [[nodiscard]] std::vector<std::string> getNames() const;
        [[nodiscard]] std::string getFileName() const;

        void add(const EnvironmentData & data);
        void update(const EnvironmentData & data);
        void updateOrAdd(const EnvironmentData & data);
        void deleteWithUUID(std::string_view uuid);

        void deleteRecordsWithProjectName(std::string_view projectName);
        void deleteTemporaryRecords();

    private:
        std::string m_FileName;
        std::vector<EnvironmentData> m_EnvironmentData;
        std::string m_Version{"1"};

        [[nodiscard]] std::vector<EnvironmentData> loadEnvironmentDataFromFile(const std::string & xmlFileName);
    };

    //! THMZ storage: each dataset is its own archive entry "environment data/<uuid>.xml",
    //! shaped as a one-record library file so the library schema validates it. An archive
    //! without such entries yields an empty vector (pre-consolidation file).
    [[nodiscard]] std::vector<EnvironmentData> loadDatasetsFromZipFile(const std::string & zipFileName);

    //! Writes every dataset as its own entry; returns the number of entries written.
    int saveDatasetsToZipFile(const std::vector<EnvironmentData> & datasets, const std::string & zipFileName);
}   // namespace EnvironmentDataLibrary