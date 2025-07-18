#include <fstream>
#include <algorithm>

#include <fileParse/XMLNodeAdapter.hxx>
#include <fileParse/Vector.hxx>

#include "DB.hxx"
#include "Common/DB.hxx"

#include "SteadyState.hxx"
#include "Tags.hxx"
#include "Serializers.hxx"

#include "LibraryUtilities/Common.hxx"
#include "LibraryUtilities/FileManipulation.hxx"

#include "Common/Common.hxx"

namespace BCSteadyStateLibrary
{
    DB::DB(const std::string & xmlFileName) : m_FileName(xmlFileName)
    {
        // Need to check if file exists and create empty one if it does not.
        if(std::ifstream f(xmlFileName.c_str()); !f.good())
        {
            // This is minimal requirement for XML file. Otherwise, component will crash.
            BCSteadyStateLibrary::Tags tags;
            const std::string fileContent{
              Common::generateXmlContent(tags.boundaryConditions(), "BC Steady State.xsd", m_Version)};
            File::createFileFromString(xmlFileName, fileContent);
        }

        try
        {
            m_BoundaryConditions = loadBoundaryConditionsFromFile(xmlFileName);
        }
        catch(...)
        {
            throw;
        }
    }

    void DB::loadFromXMLString(const std::string & xmlString)
    {
        BCSteadyStateLibrary::Tags tags;
        const auto xBCNode{getTopNodeFromString(xmlString, tags.boundaryConditions())};

        if(xBCNode.has_value())
        {
            xBCNode.value() >> FileParse::Child{"Version", m_Version};
            xBCNode.value() >> FileParse::Child{tags.boundaryCondition(), m_BoundaryConditions};
        }
    }

    std::string DB::saveToXMLString() const
    {
        BCSteadyStateLibrary::Tags tags;
        auto node{createTopNode(tags.boundaryConditions())};

        node << FileParse::Child{"Version", m_Version};
        node << FileParse::Child{tags.boundaryCondition(), m_BoundaryConditions};

        return node.getContent();
    }

    void DB::loadFromZipFile(const std::string & zipFileName)
    {
        auto values = Common::loadFromZipFile<std::vector<BoundaryCondition>>(
          zipFileName, ThermZip::SteadyStateBCFileName, "BoundaryConditions");
        if(values)
        {
            m_BoundaryConditions = values.value();
        }
    }

    int DB::saveToZipFile(std::string_view zipFileName) const
    {
        return Common::saveToZIPFile(
          m_BoundaryConditions, ThermZip::SteadyStateBCFileName, zipFileName, "BoundaryConditions");
    }

    std::vector<BoundaryCondition> DB::loadBoundaryConditionsFromFile(const std::string & xmlFileName)
    {
        BCSteadyStateLibrary::Tags tags;
        const auto xBCNode{getTopNodeFromFile(xmlFileName, tags.boundaryConditions())};

        std::vector<BoundaryCondition> boundaryConditions;
        if(xBCNode.has_value())
        {
            xBCNode.value() >> FileParse::Child{"Version", m_Version};
            xBCNode.value() >> FileParse::Child{tags.boundaryCondition(), boundaryConditions};
        }

        return boundaryConditions;
    }

    void DB::update(const BoundaryCondition & condition)
    {
        for(auto & bc : m_BoundaryConditions)
        {
            if(bc.UUID == condition.UUID)
            {
                bc = condition;
            }
        }
    }

    void DB::updateOrAdd(const BoundaryCondition & condition)
    {
        const auto bc{getByUUID(condition.UUID)};
        bc.has_value() ? update(condition) : add(condition);
    }

    void DB::add(const BoundaryCondition & condition)
    {
        m_BoundaryConditions.emplace_back(condition);
    }

    int DB::saveToFile() const
    {
        BCSteadyStateLibrary::Tags tags;

        auto node{createTopNode(tags.boundaryConditions())};

        node << FileParse::Child{"Version", m_Version};
        node << FileParse::Child{tags.boundaryCondition(), m_BoundaryConditions};

        return node.writeToFile(m_FileName);
    }

    std::optional<BoundaryCondition> DB::getByUUID(std::string_view uuid) const
    {
        return getBoundaryCondition(uuid, [uuid](const BoundaryCondition & obj) { return obj.UUID == uuid; });
    }

    std::optional<BoundaryCondition> DB::getByName(std::string_view name) const
    {
        return getBoundaryCondition(name, [name](const BoundaryCondition & obj) { return obj.Name == name; });
    }

    std::vector<BoundaryCondition> & DB::getBoundaryConditions()
    {
        return m_BoundaryConditions;
    }

    std::string DB::getFileName() const
    {
        return m_FileName;
    }

    void DB::deleteWithUUID(std::string_view uuid)
    {
        m_BoundaryConditions.erase(std::remove_if(m_BoundaryConditions.begin(),
                                                  m_BoundaryConditions.end(),
                                                  [uuid](const BoundaryCondition & obj) { return obj.UUID == uuid; }),
                                   m_BoundaryConditions.end());
    }

    std::vector<std::string> DB::getNames() const
    {
        std::vector<std::string> names;
        names.reserve(m_BoundaryConditions.size());
        std::transform(m_BoundaryConditions.begin(),
                       m_BoundaryConditions.end(),
                       std::back_inserter(names),
                       [](const BoundaryCondition & obj) { return obj.Name; });
        return names;
    }

    std::vector<std::string> DB::getDisplayNames() const
    {
        std::vector<std::string> names;
        names.reserve(m_BoundaryConditions.size());
        std::transform(m_BoundaryConditions.begin(),
                       m_BoundaryConditions.end(),
                       std::back_inserter(names),
                       [](const BoundaryCondition & obj) { return LibraryCommon::DisplayName(obj); });
        return names;
    }

    void DB::deleteRecordsWithProjectName(std::string_view projectName)
    {
        m_BoundaryConditions.erase(std::ranges::remove_if(m_BoundaryConditions,
                                                          [projectName](const BoundaryCondition & obj) {
                                                              return obj.ProjectName == projectName;
                                                          })
                                     .begin(),
                                   m_BoundaryConditions.end());
    }

    void DB::deleteTemporaryRecords()
    {
        LibraryCommon::removeTemporaryRecords(m_BoundaryConditions);
    }

    std::optional<BoundaryCondition> DB::getByDisplayName(std::string_view displayName) const
    {
        return getBoundaryCondition(displayName, [displayName](const BoundaryCondition & obj) {
            return LibraryCommon::DisplayName(obj) == displayName;
        });
    }

    [[nodiscard]] std::optional<BoundaryCondition> DB::getDefaultRadiationSurface() const
    {
        for(const auto & bc : m_BoundaryConditions)
        {
            if(std::holds_alternative<BCSteadyStateLibrary::RadiationSurface>(bc.data))
            {
                const auto & rs = std::get<BCSteadyStateLibrary::RadiationSurface>(bc.data);
                if(rs.isDefault)
                {
                    return bc;
                }
            }
        }
        return std::nullopt;
    }

    BoundaryCondition DB::getDefaultRecord() const
    {
        return m_BoundaryConditions[0];
    }

}   // namespace BCSteadyStateLibrary
