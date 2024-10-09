#pragma once

#include <set>
#include <functional>

#include "Materials.hxx"

namespace MaterialsLibrary
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    ///   DB
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    //! \brief Class that handles materials fromXML file.
    class DB
    {
    public:
        DB() = default;
        explicit DB(const std::string & xmlFileName);

        void loadFromXMLString(const std::string & xmlString);
        [[nodiscard]] std::string saveToXMLString() const;

        //! \brief return only solid materials from the XML file.
        //!
        //! \return Reference to all material records is needed in case update to material happens and changes need to
        //! be saved.
        std::vector<Material> & getMaterials();

        //! \brief Search and returns material by name.
        //!
        //! @materialName - Material name for which search will be performed.
        //! @return - Material record. It will be empty if material name does not exist.
        [[nodiscard]] std::optional<Material> getMaterialByName(std::string_view materialName);

        [[nodiscard]] std::optional<Material> getMaterialByDisplayName(std::string_view materialName);

        //! \brief Search and returns material by uuid.
        //!
        //! @uuid - Universal Unique Identifier for which search will be performed.
        //! @return - Material record. It will be empty if material with given uuid does not exist.
        [[nodiscard]] std::optional<Material> getByUUID(const std::string & uuid);

        //! \brief Default record is needed to select something at the beginning
        [[nodiscard]] Material getDefaultRecord() const;

        [[nodiscard]] std::vector<std::string> getMaterialNames() const;

        [[nodiscard]] std::vector<std::string> getMaterialDisplayNames() const;

        //! \brief Adds material into database.
        //! Note that material is still not saved into XML file. There is separate function that does that.
        //! @material - Material that will be stored into database.
        void addMaterial(const Material & material);

        //! \brief Updates current record with new data
        //!
        //! @material - Record that will be updated in the library. Material must match by uuid in order to have other
        //! properties updated.
        void updateMaterial(const Material & material);

        //! \brief Update current record or in case record does not exists, it add new to the database.
        void updateOrAddMaterial(const Material & material);

        //! \brief Deletes materials with given uuid
        void deleteWithUUID(std::string_view uuid);

        //! \brief Saves current state of object to XML file (provided through object constructor)
        [[nodiscard]] int saveToFile() const;

        //! \brief Deletes all materials that belong to given project
        void deleteRecordsWithProjectName(const std::string & projectName);

        [[nodiscard]] std::string getFileName() const;

        //! \brief Sets default record by material name
        void setDefaultRecord(std::string_view materialName);

    private:
        std::optional<Material> getMaterialByPredicate(std::function<bool(const Material&)> predicate) const;

        std::string m_FileName;

        std::vector<Material> m_Materials;

        size_t m_DefaultRecordIndex{0u};

        std::string m_Version{"1"};

        //! \brief Loads materials from XML file.
        std::vector<Material> loadMaterialsFromXMLFile(const std::string & materialXMLFileName);
    };

    //! \brief Returns all the gas names used by cavities in the library
    [[nodiscard]] std::set<std::string, std::less<>> getGasNames(const std::vector<Material> & materials);
}
