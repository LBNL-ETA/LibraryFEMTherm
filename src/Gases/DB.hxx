#pragma once

#include <string>
#include <vector>
#include <optional>

#include <fileParse/XMLNodeAdapter.hxx>

#include "Gases.hxx"

namespace GasesLibrary
{
    [[nodiscard]] std::string loadVersionFromXMLFile(std::string_view fileName);

    [[nodiscard]] std::vector<Gas> loadGasesFromXMLFile(std::string_view fileName);
    [[nodiscard]] std::vector<Gas> loadGasesDataFromXMLString(std::string_view xmlString);

    [[nodiscard]] std::vector<PureGas> loadPureGasesFromXMLFile(std::string_view fileName);
    [[nodiscard]] std::vector<PureGas> loadPureGasesFromXMLString(std::string_view xmlString);

    class DB
    {
    public:
        DB() = default;
        explicit DB(const std::string & fileName);

        [[nodiscard]] std::optional<PureGas> getPureGasByUUID(std::string_view uuid) const;
        [[nodiscard]] std::optional<PureGas> getPureGasByName(std::string_view name) const;
        [[nodiscard]] std::optional<PureGas> getPureGasByDisplayName(std::string_view name) const;

        [[nodiscard]] std::optional<Gas> getGasByUUID(std::string_view uuid) const;
        [[nodiscard]] std::optional<Gas> getGasByName(std::string_view name) const;
        [[nodiscard]] std::optional<Gas> getGasByDisplayName(std::string_view name) const;

        [[nodiscard]] std::optional<GasesData> getGasDataByUUID(std::string_view uuid) const;
        [[nodiscard]] std::optional<GasesData> getGasDataByName(std::string_view name) const;
        [[nodiscard]] std::optional<GasesData> getGasDataByDisplayName(const std::string & name) const;

        [[nodiscard]] std::vector<Gas> & getGases();
        [[nodiscard]] std::vector<PureGas> & getPureGases();
        [[nodiscard]] std::vector<GasesData> getGasesData() const;

        [[nodiscard]] std::vector<std::string> getGasNames() const;
        [[nodiscard]] std::vector<std::string> getGasDisplayNames() const;

        [[nodiscard]] std::vector<std::string> getPureGasNames() const;

        [[nodiscard]] std::string getFileName() const;

        void addPureGas(const PureGas & pure);
        void updatePureGas(const PureGas & pure);
        void removePureGas(const PureGas & pure);
        void addOrUpdatePureGas(const PureGas & pure);
        void addGas(const Gas & gas);

        void addGasesData(const GasesData & gasData);

        [[nodiscard]] int saveToFile();
        [[nodiscard]] std::string saveToXMLString();

        //! \brief Deletes all materials that belong to given project
        void deleteGasesWithProjectName(const std::string & projectName);
        void deleteGasesWithUUID(const std::string & uuid);

    private:
        void savePureGases(XMLNodeAdapter & gasesNode) const;
        void saveGases(XMLNodeAdapter & gasesNode) const;

        std::string m_FileName;
        std::string m_Version{"1"};
        std::vector<PureGas> m_PureGases;
        std::vector<Gas> m_Gases;
        void removeTemporaryRecords();
    };
}   // namespace GasesLibrary
