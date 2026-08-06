#pragma once

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <fileParse/FileFormat.hxx>

#include "BoundaryConditions.hxx"

namespace BCLibrary
{
    class DB
    {
    public:
        DB() = default;
        explicit DB(const std::string & xmlFileName);

        void loadFromString(const std::string & str);
        [[nodiscard]] std::string saveToString(FileParse::FileFormat format = FileParse::FileFormat::XML) const;
        [[nodiscard]] int saveToFile(FileParse::FileFormat format = FileParse::FileFormat::XML) const;

        //! THMZ storage: the embedded unified library lives in the archive as the
        //! BoundaryConditions.xml entry. Loading from an archive without that entry
        //! leaves the DB unchanged (pre-consolidation file; migration handles it).
        void loadFromZipFile(const std::string & zipFileName);
        //! Same, but from already-extracted archive entries (no further extraction).
        void loadFromEntries(const std::map<std::string, std::string> & entries);
        [[nodiscard]] int saveToZipFile(std::string_view zipFileName,
                                        FileParse::FileFormat format = FileParse::FileFormat::XML) const;

        [[nodiscard]] std::optional<BoundaryCondition> getByUUID(std::string_view uuid) const;
        [[nodiscard]] std::optional<BoundaryCondition> getByName(std::string_view name) const;
        [[nodiscard]] std::optional<BoundaryCondition> getByDisplayName(std::string_view displayName) const;

        [[nodiscard]] std::vector<BoundaryCondition> & getBoundaryConditions();
        [[nodiscard]] std::vector<std::string> getNames() const;
        [[nodiscard]] std::vector<std::string> getDisplayNames() const;
        [[nodiscard]] std::string getFileName() const;

        void add(const BoundaryCondition & condition);
        void update(const BoundaryCondition & condition);
        void updateOrAdd(const BoundaryCondition & condition);
        void deleteWithUUID(std::string_view uuid);

        void deleteRecordsWithProjectName(std::string_view projectName);
        void deleteTemporaryRecords();

        [[nodiscard]] std::optional<BoundaryCondition> getDefaultRadiationSurface() const;
        [[nodiscard]] std::optional<BoundaryCondition> getDefaultRecord() const;

    private:
        std::string m_FileName;
        std::vector<BoundaryCondition> m_BoundaryConditions;
        std::string m_Version{"1"};

        [[nodiscard]] std::vector<BoundaryCondition> loadBoundaryConditionsFromFile(const std::string & xmlFileName);
    };
}   // namespace BCLibrary
