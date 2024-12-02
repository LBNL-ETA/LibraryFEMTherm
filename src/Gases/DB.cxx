#include "DB.hxx"

#include <algorithm>
#include <fstream>

#include <fileParse/XMLNodeAdapter.hxx>
#include <fileParse/Vector.hxx>

#include "Serializers.hxx"
#include "Operators.hxx"

#include "Common/Common.hxx"
#include "LibraryUtilities/Common.hxx"
#include "LibraryUtilities/FileManipulation.hxx"

namespace GasesLibrary
{

    template<typename T>
    T processGasesNodeFromFile(const std::string_view & fileName, const std::string & tag)
    {
        Tags fileTags;
        const auto gasesNode = getTopNodeFromFile(fileName.data(), fileTags.gases());

        T result{};

        if(gasesNode.has_value())
        {
            gasesNode.value() >> FileParse::Child{tag, result};
        }

        return result;
    }

    template<typename T>
    T processGasesNodeFromString(const std::string_view & xmlString, const std::string & tag)
    {
        Tags fileTags;
        const auto gasesNode = getTopNodeFromString(xmlString.data(), fileTags.gases());

        T result{};

        if(gasesNode.has_value())
        {
            gasesNode.value() >> FileParse::Child{tag, result};
        }

        return result;
    }

    std::string loadVersionFromXMLFile(std::string_view fileName)
    {
        Tags tag;
        return processGasesNodeFromFile<std::string>(fileName, "Version");
    }

    std::vector<Gas> loadGasesFromXMLFile(std::string_view fileName)
    {
        Tags tag;
        return processGasesNodeFromFile<std::vector<Gas>>(fileName, tag.gas());
    }

    std::vector<Gas> loadGasesDataFromXMLString(std::string_view xmlString)
    {
        Tags tag;
        return processGasesNodeFromString<std::vector<Gas>>(xmlString, tag.gas());
    }

    std::vector<PureGas> loadPureGasesFromXMLFile(std::string_view fileName)
    {
        Tags tag;
        return processGasesNodeFromFile<std::vector<PureGas>>(fileName, tag.pureGas());
    }

    std::vector<PureGas> loadPureGasesFromXMLString(std::string_view xmlString)
    {
        Tags tag;
        return processGasesNodeFromString<std::vector<PureGas>>(xmlString, tag.pureGas());
    }

    DB::DB(const std::string & fileName) : m_FileName(fileName)
    {
        // Need to check if file exists and create empty one if it does not.
        if(std::ifstream f(fileName.c_str()); !f.good())
        {
            Tags tag;
            // This is minimal requirement for XML file. Otherwise, component will crash.
            const std::string fileContent{Common::generateXmlContent(tag.gases(), "Gases.xsd", m_Version)};
            File::createFileFromString(fileName, fileContent);
        }

        try
        {
            m_Version = loadVersionFromXMLFile(fileName);
            m_PureGases = loadPureGasesFromXMLFile(fileName);
            m_Gases = loadGasesFromXMLFile(fileName);
        }
        catch(...)
        {
            throw;
        }
    }

    std::optional<PureGas> DB::getPureGasByUUID(std::string_view uuid) const
    {
        for(const auto & gas : m_PureGases)
        {
            if(gas.UUID == uuid)
            {
                return gas;
            }
        }
        return std::nullopt;
    }

    std::vector<PureGas> & DB::getPureGases()
    {
        return m_PureGases;
    }

    void DB::addPureGas(const PureGas & pure)
    {
        m_PureGases.emplace_back(pure);
    }

    int DB::saveToFile()
    {
        removeTemporaryRecords();

        Tags tag;
        auto gasesNode{createTopNode(tag.gases())};

        gasesNode << FileParse::Child{"Version", m_Version};
        savePureGases(gasesNode);
        saveGases(gasesNode);

        return gasesNode.writeToFile(m_FileName);
    }

    std::string DB::saveToXMLString()
    {
        removeTemporaryRecords();

        Tags tag;
        auto gasesNode{createTopNode(tag.gases())};

        gasesNode << FileParse::Child{"Version", m_Version};
        savePureGases(gasesNode);
        saveGases(gasesNode);

        return gasesNode.getContent();
    }

    void DB::savePureGases(XMLNodeAdapter & gasesNode) const
    {
        for(const auto & gas : m_PureGases)
        {
            if(!LibraryCommon::isRecordTemporary(gas))
            {
                Tags tag;
                XMLNodeAdapter pureGasNode = gasesNode.addChild(tag.pureGas());
                pureGasNode << gas;
            }
        }
    }

    std::optional<PureGas> DB::getPureGasByName(std::string_view name) const
    {
        for(const auto & gas : m_PureGases)
        {
            if(gas.Name == name)
            {
                return gas;
            }
        }

        return std::nullopt;
    }

    std::optional<PureGas> DB::getPureGasByDisplayName(std::string_view name) const
    {
        for(const auto & gas : m_PureGases)
        {
            if(LibraryCommon::DisplayName(gas) == name)
            {
                return gas;
            }
        }

        return std::nullopt;
    }

    std::optional<Gas> DB::getGasByUUID(std::string_view uuid) const
    {
        for(const auto & gas : m_Gases)
        {
            if(gas.UUID == uuid)
            {
                return gas;
            }
        }

        return std::nullopt;
    }

    std::optional<Gas> DB::getGasByName(std::string_view name) const
    {
        for(const auto & gas : m_Gases)
        {
            if(gas.Name == name)
            {
                return gas;
            }
        }

        return std::nullopt;
    }

    std::vector<Gas> & DB::getGases()
    {
        return m_Gases;
    }

    void DB::saveGases(XMLNodeAdapter & gasesNode) const
    {
        if(!m_Gases.empty())
        {
            GasesLibrary::Tags tag;
            gasesNode << FileParse::Child{tag.gas(), m_Gases};
        }
    }

    void DB::removeTemporaryRecords()
    {
        auto newEnd = std::remove_if(
          m_Gases.begin(), m_Gases.end(), [](const Gas & gas) { return LibraryCommon::isRecordTemporary(gas); });
        m_Gases.erase(newEnd, m_Gases.end());
    }

    void DB::addGas(const Gas & gas)
    {
        m_Gases.emplace_back(gas);
    }

    std::optional<Gas> DB::getGasByDisplayName(std::string_view name) const
    {
        for(const auto & gas : m_Gases)
        {
            if(LibraryCommon::DisplayName(gas) == name)
            {
                return gas;
            }
        }

        return std::nullopt;
    }

    std::optional<GasesData> DB::getByUUID(std::string_view uuid) const
    {
        std::optional<GasesData> result;

        if(auto gas{getGasByUUID(uuid)}; gas.has_value())
        {
            std::vector<std::optional<PureGas>> pureGases;
            for(const auto & component : gas.value().Components)
            {
                pureGases.emplace_back(getPureGasByName(component.PureGasName));
            }
            result = GasesData{gas.value(), pureGases};
        }

        return result;
    }

    std::optional<GasesData> DB::getByName(std::string_view name) const
    {
        std::optional<GasesData> result;

        if(auto gas{getGasByName(name)}; gas.has_value())
        {
            std::vector<std::optional<PureGas>> pureGases;
            for(const auto & component : gas.value().Components)
            {
                pureGases.emplace_back(getPureGasByName(component.PureGasName));
            }
            result = GasesData{gas.value(), pureGases};
        }

        return result;
    }

    std::optional<GasesData> DB::getByDisplayName(const std::string & name) const
    {
        std::optional<GasesData> result;

        if(const auto gas{getGasByDisplayName(name)}; gas.has_value())
        {
            std::vector<std::optional<PureGas>> pureGases;
            for(const auto & component : gas.value().Components)
            {
                pureGases.emplace_back(getPureGasByDisplayName(component.PureGasName));
            }
            result = GasesData{gas.value(), pureGases};
        }

        return result;
    }

    void DB::deleteWithProjectName(const std::string & projectName)
    {
        m_Gases.erase(std::remove_if(m_Gases.begin(),
                                     m_Gases.end(),
                                     [&projectName](const Gas & gas) { return gas.ProjectName == projectName; }),
                      m_Gases.end());
    }

    std::vector<std::string> DB::getNames() const
    {
        std::vector<std::string> names;
        names.reserve(m_Gases.size());

        for(const auto & gas : m_Gases)
        {
            names.push_back(gas.Name);
        }

        return names;
    }

    std::vector<std::string> DB::getDisplayNames() const
    {
        std::vector<std::string> names;
        names.reserve(m_Gases.size());

        for(const auto & gas : m_Gases)
        {
            names.push_back(LibraryCommon::DisplayName(gas));
        }

        return names;
    }

    std::vector<std::string> DB::getPureGasNames() const
    {
        std::vector<std::string> names;
        names.reserve(m_PureGases.size());

        for(const auto & pureGas : m_PureGases)
        {
            names.push_back(pureGas.Name);
        }

        return names;
    }

    std::string DB::getFileName() const
    {
        return m_FileName;
    }

    void DB::updatePureGas(const PureGas & pure)
    {
        for(auto & pureGas : m_PureGases)
        {
            if(pureGas.Name == pure.Name)
            {
                pureGas = pure;
                break;
            }
        }
    }

    void DB::removePureGas(const PureGas & pure)
    {
        for(auto it = m_PureGases.begin(); it != m_PureGases.end(); ++it)
        {
            if(it->Name == pure.Name)
            {
                m_PureGases.erase(it);
                break;
            }
        }
    }

    std::vector<GasesData> DB::getGasesData() const
    {
        std::vector<GasesData> result;
        result.reserve(m_Gases.size());

        for(const auto & gas : m_Gases)
        {
            std::vector<std::optional<PureGas>> pureGases;
            pureGases.reserve(gas.Components.size());

            for(const auto & component : gas.Components)
            {
                pureGases.emplace_back(getPureGasByDisplayName(component.PureGasName));
            }
            result.emplace_back(gas, pureGases);
        }

        return result;
    }

    void DB::deleteWithUUID(std::string_view uuid)
    {
        m_Gases.erase(
          std::remove_if(m_Gases.begin(), m_Gases.end(), [&uuid](const Gas & gas) { return gas.UUID == uuid; }),
          m_Gases.end());
    }

    void DB::add(const GasesData & gasData)
    {
        // add gas only if gasData.gas.uuid is not in the library
        if(!getGasByUUID(gasData.gas.UUID).has_value())
        {
            addGas(gasData.gas);
        }
        for(const auto & pureGas : gasData.components)
        {
            auto pureGasFromLibrary{getPureGasByName(pureGas->Name)};
            if(!pureGasFromLibrary.has_value()
               || (pureGas.has_value() && !arePhysicalPropertiesEqual(pureGas.value(), pureGasFromLibrary.value())))
            {
                addPureGas(pureGas.value());
            }
        }
    }

    void DB::addOrUpdatePureGas(const PureGas & pure)
    {
        if(auto pureGas{getPureGasByName(pure.Name)}; pureGas.has_value())
        {
            updatePureGas(pure);
        }
        else
        {
            addPureGas(pure);
        }
    }
}   // namespace GasesLibrary
