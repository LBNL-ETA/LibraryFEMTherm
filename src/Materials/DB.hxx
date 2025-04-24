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
        [[nodiscard]] std::optional<Material> getByName(std::string_view materialName);

        [[nodiscard]] std::optional<Material> getByDisplayName(std::string_view materialName);

        //! \brief Search and returns material by uuid.
        //!
        //! @uuid - Universal Unique Identifier for which search will be performed.
        //! @return - Material record. It will be empty if material with given uuid does not exist.
        [[nodiscard]] std::optional<Material> getByUUID(std::string_view uuid);

        //! \brief Default record is needed to select something at the beginning
        [[nodiscard]] Material getDefaultRecord() const;

        [[nodiscard]] std::vector<std::string> getNames() const;

        [[nodiscard]] std::vector<std::string> getDisplayNames() const;

        //! \brief Adds material into a database.
        //! Note that material is still not saved into the XML file. There is a separate function that does that.
        //! @material - Material that will be stored into the database.
        void add(const Material & material);

        //! \brief Updates current record with new data
        //!
        //! @material Record to update in the library. Updates apply only if the UUID matches.
        void update(const Material & material);

        //! \brief Update current record or in case record do not exist, it adds new to the database.
        void updateOrAdd(const Material & material);

        //! \brief Deletes materials with given uuid
        void deleteWithUUID(std::string_view uuid);

        //! \brief Saves current state of an object to XML file (provided through object constructor)
        [[nodiscard]] int saveToFile() const;

        //! \brief Deletes all materials that belong to given project
        void deleteRecordsWithProjectName(std::string_view projectName);

        [[nodiscard]] std::string getFileName() const;

        //! \brief Sets default record by material name
        void setDefaultRecord(std::string_view materialName);

    private:
        std::optional<Material> getMaterialByPredicate(std::function<bool(const Material&)> predicate) const;

        std::string m_FileName;

        std::vector<Material> m_Materials;

        size_t m_DefaultRecordIndex{0u};

        std::string m_Version{"1"};

        //! \brief Loads materials from the XML file.
        std::vector<Material> loadMaterialsFromXMLFile(const std::string & materialXMLFileName);
    };

    //! \brief Returns all the gas names used by cavities in the library
    [[nodiscard]] std::set<std::string, std::less<>> getNames(const std::vector<Material> & materials);
}
