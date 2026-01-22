#include <algorithm>
#include <fstream>

#include <fileParse/FileDataHandler.hxx>
#include <fileParse/Vector.hxx>

#include "Types.hxx"
#include "TypesSerializers.hxx"

#include "LibraryUtilities/FileManipulation.hxx"

namespace BCTypesLibrary
{
    //////////////////////////////////////////////////////////////////////////////////////////////
    /// BCTypeRecord
    //////////////////////////////////////////////////////////////////////////////////////////////

    // Need at least some defaults so that BC is not created empty. In case user only presses new button it will
    // still work. (Simon)
    TypeRecord::TypeRecord(std::string uuid) : UUID(std::move(uuid)), ConvectionBc(ConvectionRecord())
    {
        ConvectionBc->Model = ConvectionModel::Fixed_Convection_Coefficient;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////
    /// DB
    //////////////////////////////////////////////////////////////////////////////////////////////

    DB::DB(const std::string & xmlName) : m_FileName(xmlName)
    {
        // Need to check if file exists and create empty one if it does not.
        if(const std::ifstream f(m_FileName.c_str()); !f.good())
        {
            // This is minimal requirement for XML file. Otherwise component will crash.
            const std::string fileContent{"<BoundaryConditionsType>\n</BoundaryConditionsType>"};
            File::createFileFromString(m_FileName, fileContent);
        }

        try
        {
            m_BoundaryConditions = loadBoundaryConditionsFromFile(xmlName);
        }
        catch(...)
        {
            throw;
        }
    }

    std::vector<TypeRecord> & DB::getBoundaryConditions()
    {
        return m_BoundaryConditions;
    }


    std::optional<TypeRecord> DB::getByUUID(std::string_view uuid)
    {
        return getBoundaryCondition([&uuid](const TypeRecord & bc) { return bc.UUID == uuid; });
    }


    std::optional<TypeRecord> DB::getByName(std::string_view name)
    {
        return getBoundaryCondition([&name](const TypeRecord & bc) { return bc.Name == name; });
    }

    void DB::update(const TypeRecord & bcType)
    {
        auto it = std::find_if(m_BoundaryConditions.begin(),
                               m_BoundaryConditions.end(),
                               [&bcType](const TypeRecord & bc) { return bc.UUID == bcType.UUID; });

        if(it != m_BoundaryConditions.end())
        {
            *it = bcType;   // Update the first matching element
        }
    }

    void DB::add(const TypeRecord & bcType)
    {
        m_BoundaryConditions.emplace_back(bcType);
    }

    void DB::updateOrAdd(const TypeRecord & bcType)
    {
        const auto bc{getByUUID(bcType.UUID)};
        if(bc.has_value())
        {
            update(bcType);
        }
        else
        {
            add(bcType);
        }
    }

    void DB::deleteWithUUID(std::string_view uuid)
    {
        m_BoundaryConditions.erase(std::remove_if(std::begin(m_BoundaryConditions),
                                                  std::end(m_BoundaryConditions),
                                                  [&](TypeRecord const & u) { return u.UUID == uuid; }),
                                   std::end(m_BoundaryConditions));
    }

    TypeRecord DB::getDefaultRecord() const
    {
        return m_BoundaryConditions[0];
    }

    int DB::saveToFile(FileParse::FileFormat format) const
    {
        auto node{Common::createTopNode("BoundaryConditionsType", format)};

        int result = 0;
        std::visit(
          [this, &result](auto & adapter) {
              adapter << FileParse::Child{"BoundaryConditionType", m_BoundaryConditions};
              result = adapter.writeToFile(m_FileName);
          },
          node);

        return result;
    }

    void DB::loadFromString(const std::string & str)
    {
        auto node{Common::getTopNodeFromString(str, "BoundaryConditionsType")};

        if(node.has_value())
        {
            std::visit(
              [this](auto & adapter) {
                  adapter >> FileParse::Child{"BoundaryConditionType", m_BoundaryConditions};
              },
              node.value());
        }
    }

    std::string DB::saveToString(FileParse::FileFormat format) const
    {
        auto node{Common::createTopNode("BoundaryConditionsType", format)};

        std::string content;
        std::visit(
          [this, &content](auto & adapter) {
              adapter << FileParse::Child{"BoundaryConditionType", m_BoundaryConditions};
              content = adapter.getContent();
          },
          node);

        return content;
    }

    std::vector<TypeRecord> DB::loadBoundaryConditionsFromFile(std::string_view inputFileName)
    {
        const auto xBCTypeNodes{getXMLTopNodeFromFile(inputFileName.data(), "BoundaryConditionsType")};

        std::vector<TypeRecord> BCTypes;

        if(xBCTypeNodes.has_value())
        {
            xBCTypeNodes.value() >> FileParse::Child{"BoundaryConditionType", BCTypes};
        }

        return BCTypes;
    }
}   // namespace BCTypesLibrary
