#pragma once

#include <string>
#include <vector>

#include "SteadyState.hxx"

namespace BCSteadyStateLibrary
{
    class DB
    {
    public:
        DB() = default;
        explicit DB(const std::string & xmlFileName);

        void loadFromXMLString(const std::string & xmlString);
        [[nodiscard]] std::string saveToXMLString() const;

        [[nodiscard]] std::optional<BoundaryCondition>
          getByUUID(std::string_view uuid) const;
        [[nodiscard]] std::optional<BoundaryCondition>
          getBoundaryConditionByName(std::string_view name) const;
        [[nodiscard]] std::optional<BoundaryCondition>
          getBoundaryConditionByDisplayName(std::string_view displayName) const;

        [[nodiscard]] std::vector<BoundaryCondition> & getBoundaryConditions();

        [[nodiscard]] std::string getFileName() const;

        void deleteRecordWithUUID(std::string_view uuid);

        void addBoundaryCondition(const BoundaryCondition & condition);

        [[nodiscard]] std::vector<std::string> getBoundaryConditionNames() const;
        [[nodiscard]] std::vector<std::string> getBoundaryConditionDisplayNames() const;

        void deleteRecordsWithProjectName(std::string_view projectName);

        [[nodiscard]] std::optional<BoundaryCondition> getDefaultRadiationSurface() const;
        [[nodiscard]] BoundaryCondition getDefaultRecord() const;

        [[nodiscard]] int saveToFile() const;

    private:
        std::string m_FileName;
        std::vector<BoundaryCondition> m_BoundaryConditions;
        std::string m_Version{"1"};

        std::vector<BoundaryCondition> loadBoundaryConditionsFromFile(const std::string & xmlFileName);

        template<typename Pred>
        [[nodiscard]] std::optional<BoundaryCondition> getBoundaryCondition(std::string_view value,
                                                                            Pred pred) const
        {
            std::optional<BoundaryCondition> result;

            if(auto it = std::find_if(m_BoundaryConditions.begin(), m_BoundaryConditions.end(), pred);
               it != m_BoundaryConditions.end())
            {
                result = *it;
            }

            if(value.empty())
            {
                result = m_BoundaryConditions[0];
            }

            return result;
        }
    };
}   // namespace BCSteadyState