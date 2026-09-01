#pragma once

#include <string>
#include <optional>
#include <vector>
#include <algorithm>

#include <fileParse/FileFormat.hxx>

#include "Definitions.hxx"

namespace BCTypesLibrary
{
    struct ConvectionRecord
    {
        ConvectionRecord() = default;

        ConvectionModel Model{ConvectionModel::ASHRAE_NFRC_Inside};
        std::optional<double> Temperature;
        std::optional<double> FilmCoefficient;
        std::optional<double> WindSpeed;
        std::optional<Direction> WindDirection;
    };

    struct RadiationRecord
    {
        RadiationModel Model{RadiationModel::Automatic_Enclosure};
        std::optional<double> Temperature;
        std::optional<double> SurfaceEmissivity;
        std::optional<double> ViewFactor;
        std::optional<double> FilmCoefficient;
    };

    struct SolarRecord
    {
        std::optional<double> SolarRadiation;
        std::optional<double> Absorptance;
    };


    //////////////////////////////////////////////////////////////////////////////////////////////
    // TypeRecord
    //////////////////////////////////////////////////////////////////////////////////////////////

    struct TypeRecord
    {
        TypeRecord() = default;
        explicit TypeRecord(std::string uuid);

        std::string UUID;
        std::string Name{"Default Name"};
        bool Protected{false};
        BCType bcType{BCType::Transient};
        BCModel bcModel{BCModel::Neumann};

        std::optional<ConvectionRecord> ConvectionBc;
        std::optional<RadiationRecord> RadiationBc;
        std::optional<SolarRecord> SolarBc;

        bool UseHeatFlux{false};
        std::optional<double> HeatFlux;
        bool UseTemperature{false};
        std::optional<double> Temperature;
        bool UseHumidity{false};
        std::optional<double> Humidity;
        std::string Color{"FFFFFF"};
        std::optional<std::string> ProjectName;
    };

    //////////////////////////////////////////////////////////////////////////////////////////////
    // BoundaryConditionTypesDB
    //////////////////////////////////////////////////////////////////////////////////////////////

    class DB
    {
    public:
        DB() = default;
        explicit DB(const std::string & xmlName);

        std::vector<TypeRecord> & getBoundaryConditions();
        std::optional<TypeRecord> getByUUID(std::string_view uuid) const;
        std::optional<TypeRecord> getByName(std::string_view name) const;

        //! \brief Finds a record by its display name ("Project:Name" for project-temporary
        //! records, plain Name otherwise) -- the name THERM boundary polygons carry.
        std::optional<TypeRecord> getByDisplayName(std::string_view displayName) const;

        //! \brief Updates current record with new data
        //!
        //! @bcType - Record that will be updated in the library
        void update(const TypeRecord & bcType);

        //! \brief Adds boundary condition type into the database.
        //! Note that bc type is still not saved into XML file. There is separate function that does that.
        //! @bcType - Boundary Condition Type that will be stored into database.
        void add(const TypeRecord & bcType);

        //! \brief Update current record or in case record does not exists, it add new to the database.
        void updateOrAdd(const TypeRecord & material);

        //! \brief Removes record with given uuid
        void deleteWithUUID(std::string_view uuid);

        //! \brief Saves current state of object to file (provided through object constructor)
        [[nodiscard]] int saveToFile(FileParse::FileFormat format = FileParse::FileFormat::XML) const;

        //! True once a mutator has effectively changed the records since load or the last
        //! successful saveIfDirty. Mutations that change nothing (updating a record with an
        //! identical copy, deleting by a key that matches no record) do not set it.
        [[nodiscard]] bool isDirty() const;

        //! saveToFile gated on isDirty: returns 0 without touching the file when clean;
        //! otherwise saves and clears the flag only when the save reports success (0).
        int saveIfDirty(FileParse::FileFormat format = FileParse::FileFormat::XML);

        void loadFromString(const std::string & str);
        [[nodiscard]] std::string saveToString(FileParse::FileFormat format = FileParse::FileFormat::XML) const;

        [[nodiscard]] std::optional<TypeRecord> getDefaultRecord() const;

    private:
        std::string m_FileName;
        std::vector<TypeRecord> m_BoundaryConditions;
        std::string m_Version{"1"};
        //! Effective-mutation flag behind isDirty/saveIfDirty. Known bypass: the non-const
        //! getBoundaryConditions() hands out a mutable reference the flag cannot see; keep
        //! new call sites read-only or route them through a tracked mutator.
        bool m_Dirty{false};

        [[nodiscard]] std::vector<TypeRecord>
          loadBoundaryConditionsFromFile(std::string_view inputFileName);

        // Generalized search function using a predicate
        template<typename Predicate>
        std::optional<TypeRecord> getBoundaryCondition(Predicate pred) const
        {
            auto it = std::find_if(begin(m_BoundaryConditions), end(m_BoundaryConditions), pred);

            if(it != end(m_BoundaryConditions))
            {
                return *it;
            }

            return std::nullopt;   // No match found
        }
    };
}   // namespace BCTypesLibrary
