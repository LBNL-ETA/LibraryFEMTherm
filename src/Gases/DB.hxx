#pragma once

#include <string>
#include <vector>
#include <optional>
#include <functional>

#include <fileParse/FileFormat.hxx>

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

        [[nodiscard]] std::optional<GasesData> getByUUID(std::string_view uuid) const;
        [[nodiscard]] std::optional<GasesData> getByName(std::string_view name) const;
        [[nodiscard]] std::optional<GasesData> getByDisplayName(const std::string & name) const;

        [[nodiscard]] std::vector<Gas> & getGases();
        [[nodiscard]] std::vector<PureGas> & getPureGases();
        [[nodiscard]] std::vector<GasesData> getGasesData() const;

        [[nodiscard]] std::vector<std::string> getNames() const;
        [[nodiscard]] std::vector<std::string> getDisplayNames() const;

        [[nodiscard]] std::vector<std::string> getPureGasNames() const;

        [[nodiscard]] std::string getFileName() const;

        void loadFromString(const std::string & str);

        void addPureGas(const PureGas & pure);
        void updatePureGas(const PureGas & pure);
        void removePureGas(const PureGas & pure);
        void addOrUpdatePureGas(const PureGas & pure);
        void addGas(const Gas & gas);

        void add(const GasesData & gasData);

        [[nodiscard]] int saveToFile(FileParse::FileFormat format = FileParse::FileFormat::XML);
        [[nodiscard]] std::string saveToString(FileParse::FileFormat format = FileParse::FileFormat::XML);

        //! \brief Deletes all gases that have projectName set
        void deleteWithProjectName(const std::string & projectName);
        void deleteTemporaryRecords();
        void deleteWithUUID(std::string_view uuid);

    private:
        template<typename NodeType>
        void savePureGases(NodeType & gasesNode) const;
        template<typename NodeType>
        void saveGases(NodeType & gasesNode) const;

        std::string m_FileName;
        std::string m_Version{"1"};
        std::vector<PureGas> m_PureGases;
        std::vector<Gas> m_Gases;
        void removeTemporaryGasRecords();
        void removeTemporaryPureGasRecords();
    };
}   // namespace GasesLibrary
