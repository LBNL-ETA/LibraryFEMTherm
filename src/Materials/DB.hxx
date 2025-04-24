#pragma once

#include <set>
#include <functional>

#include "Materials.hxx"

namespace MaterialsLibrary
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    ///   DB
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    //! \brief Class that handles materials from an XML file.
    class DB
    {
    public:
        DB() = default;
        explicit DB(const std::string & xmlFileName);

        void loadFromXMLString(const std::string & xmlString);
        [[nodiscard]] std::string saveToXMLString() const;

        //! \brief Returns only solid materials from the XML file.
        //!
        //! \return Reference to all material records is needed in case an update to material happens and changes need
        //! to be saved.
        std::vector<Material> & getMaterials();

        //! \brief Searches and returns material by name.
        //!
        //! @materialName - The material name for which the search will be performed.
        //! @return - The material record. It will be empty if the material name does not exist.
        [[nodiscard]] std::optional<Material> getByName(std::string_view materialName);

        [[nodiscard]] std::optional<Material> getByDisplayName(std::string_view materialName);

        //! \brief Searches and returns material by UUID.
        //!
        //! @uuid - The Universal Unique Identifier for which the search will be performed.
        //! @return - The material record. It will be empty if a material with the given UUID does not exist.
        [[nodiscard]] std::optional<Material> getByUUID(std::string_view uuid) const;

        //! \brief Default record is needed to select something at the beginning.
        [[nodiscard]] Material getDefaultRecord() const;

        [[nodiscard]] std::vector<std::string> getNames() const;

        [[nodiscard]] std::vector<std::string> getDisplayNames() const;

        //! \brief Adds material into a database.
        //! Note that the material is still not saved to the XML file. There is a separate function that does that.
        //! @material - The material that will be stored in the database.
        void add(const Material & material);

        //! \brief Updates the current record with new data.
        //!
        //! @material - The record to update in the library. Updates apply only if the UUID matches.
        void update(const Material & material);

        //! \brief Updates the current record or, if the record does not exist, adds a new one to the database.
        void updateOrAdd(const Material & material);

        //! \brief Deletes materials with the given UUID.
        void deleteWithUUID(std::string_view uuid);

        //! \brief Saves the current state of an object to the XML file (provided through the object constructor).
        [[nodiscard]] int saveToFile() const;

        //! \brief Deletes all materials that belong to the given project.
        void deleteRecordsWithProjectName(std::string_view projectName);

        [[nodiscard]] std::string getFileName() const;

        //! \brief Sets the default record by material name.
        void setDefaultRecord(std::string_view materialName);

    private:
        std::optional<Material> getMaterialByPredicate(std::function<bool(const Material &)> predicate) const;

        std::string m_FileName;

        std::vector<Material> m_Materials;

        size_t m_DefaultRecordIndex{0u};

        std::string m_Version{"1"};

        //! \brief Loads materials from the XML file.
        std::vector<Material> loadMaterialsFromXMLFile(const std::string & materialXMLFileName);
    };

    //! \brief Returns all the gas names used by cavities in the library.
    [[nodiscard]] std::set<std::string, std::less<>> getNames(const std::vector<Material> & materials);
}   // namespace MaterialsLibrary