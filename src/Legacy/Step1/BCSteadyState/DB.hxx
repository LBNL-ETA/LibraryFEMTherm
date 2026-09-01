#pragma once

#include <algorithm>
#include <optional>
#include <string_view>
#include <string>
#include <vector>

#include <fileParse/FileFormat.hxx>

#include "SteadyState.hxx"

namespace BCSteadyStateLibrary
{
    class DB
    {
    public:
        DB() = default;
        explicit DB(const std::string & xmlFileName);

        void loadFromString(const std::string & str);
        [[nodiscard]] std::string saveToString(FileParse::FileFormat format = FileParse::FileFormat::XML) const;

        void loadFromZipFile(const std::string & zipFileName);
        int saveToZipFile(std::string_view zipFileName) const;

        [[nodiscard]] std::optional<BoundaryCondition>
          getByUUID(std::string_view uuid) const;
        [[nodiscard]] std::optional<BoundaryCondition>
          getByName(std::string_view name) const;
        [[nodiscard]] std::optional<BoundaryCondition>
          getByDisplayName(std::string_view displayName) const;

        [[nodiscard]] std::vector<BoundaryCondition> & getBoundaryConditions();

        [[nodiscard]] std::string getFileName() const;

        void deleteWithUUID(std::string_view uuid);

        //! \brief Updates current record with new data
        //!
        //! @material - Record that will be updated in the library. Material must match by uuid in order to have other
        //! properties updated.
        void update(const BoundaryCondition & condition);

        //! \brief Update current record or in case record does not exists, it add new to the database.
        void updateOrAdd(const BoundaryCondition & condition);

        void add(const BoundaryCondition & condition);

        [[nodiscard]] std::vector<std::string> getNames() const;
        [[nodiscard]] std::vector<std::string> getDisplayNames() const;

        void deleteRecordsWithProjectName(std::string_view projectName);

        //! \brief Deletes all temporary records (one with set ProjectName)
        void deleteTemporaryRecords();

        //! \brief Moves records from one project mark to another (document Save As /
        //! rename). IGU-surface records are skipped: glazing surfaces cannot be reused
        //! and are regenerated for the renamed document instead.
        void renameRecordsWithProjectName(std::string_view oldName, std::string_view newName);

        [[nodiscard]] std::optional<BoundaryCondition> getDefaultRadiationSurface() const;
        [[nodiscard]] std::optional<BoundaryCondition> getDefaultRecord() const;

        [[nodiscard]] int saveToFile(FileParse::FileFormat format = FileParse::FileFormat::XML) const;

        //! True once a mutator has effectively changed the records since load or the last
        //! successful saveIfDirty. Mutations that change nothing (updating a record with an
        //! identical copy, deleting by a key that matches no record) do not set it.
        [[nodiscard]] bool isDirty() const;

        //! saveToFile gated on isDirty: returns 0 without touching the file when clean;
        //! otherwise saves and clears the flag only when the save reports success (0).
        int saveIfDirty(FileParse::FileFormat format = FileParse::FileFormat::XML);

    private:
        std::string m_FileName;
        std::vector<BoundaryCondition> m_BoundaryConditions;
        std::string m_Version{"1"};
        //! Effective-mutation flag behind isDirty/saveIfDirty. Known bypass: the non-const
        //! getBoundaryConditions() hands out a mutable reference the flag cannot see; keep
        //! new call sites read-only or route them through a tracked mutator.
        bool m_Dirty{false};

        std::vector<BoundaryCondition> loadBoundaryConditionsFromFile(const std::string & xmlFileName);

        template<typename Pred>
        [[nodiscard]] std::optional<BoundaryCondition> getBoundaryCondition(std::string_view value,
                                                                            Pred pred) const
        {
            std::optional<BoundaryCondition> result;

            if(value.empty())
            {
                // Return first record as default when search value is empty
                return m_BoundaryConditions.empty() ? std::nullopt
                                                    : std::optional<BoundaryCondition>{m_BoundaryConditions[0]};
            }

            if(auto iter = std::find_if(m_BoundaryConditions.begin(), m_BoundaryConditions.end(), pred);
               iter != m_BoundaryConditions.end())
            {
                result = *iter;
            }

            return result;
        }
    };
}   // namespace BCSteadyState