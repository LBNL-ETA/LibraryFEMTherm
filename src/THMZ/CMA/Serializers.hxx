#pragma once

#include <fileParse/Base.hxx>
#include <fileParse/Map.hxx>

#include "THMZ/CMA/CMA.hxx"
#include "THMZ/CMA/EnumSerializers.hxx"

namespace ThermFile
{
    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, CMABCContainer & cmaBCContainer)
    {
        const auto childNodes{node.getChildNodesByName("Position")};
        for(const auto & childNode : childNodes)
        {
            std::string value;
            CMABCPosition key;

            childNode >> FileParse::Child{"Value", key};
            childNode >> FileParse::Child{"UUID", value};

            //! Only first inserted key is stored. If the key already exists, the value is not updated.
            cmaBCContainer.emplace(key, value);
        }

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const CMABCContainer & cmaBCContainer)
    {
        if(cmaBCContainer.empty())
            return node;

        for(const auto & [enumKey, mapValue] : cmaBCContainer)
        {
            auto childNode{node.addChild("Position")};
            childNode << FileParse::Child{"Value", enumKey};
            childNode << FileParse::Child{"UUID", mapValue};
        }

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::CMAData & cmaData)
    {
        node >> FileParse::Child{"IGUType", cmaData.iguType};
        node >> FileParse::Child{"CMABCPositions", cmaData.cmaBCPositions};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::CMAData & cmaData)
    {
        node << FileParse::Child{"IGUType", cmaData.iguType};
        node << FileParse::Child{"CMABCPositions", cmaData.cmaBCPositions};

        return node;
    }
}