#pragma once

#include "Results.hxx"

#include <fileParse/Vector.hxx>
#include <fileParse/Optional.hxx>

#include "THMZ/Common/EnumSerializers.hxx"
#include "THMZ/CMA/EnumSerializers.hxx"

namespace ThermFile
{
    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, NodeResults & nodeResults)
    {
        node >> FileParse::Child{"Index", nodeResults.index};
        node >> FileParse::Child{"Temperature", nodeResults.temperature};
        node >> FileParse::Child{"X-flux", nodeResults.xFlux};
        node >> FileParse::Child{"Y-flux", nodeResults.yFlux};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const NodeResults & nodeResults)
    {
        node << FileParse::Child{"Index", nodeResults.index};
        node << FileParse::Child{"Temperature", nodeResults.temperature};
        node << FileParse::Child{"X-flux", nodeResults.xFlux};
        node << FileParse::Child{"Y-flux", nodeResults.yFlux};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, EdgeResults & edgeResults)
    {
        node >> FileParse::Child{"Type", edgeResults.segmentType};
        node >> FileParse::Child{"Index", edgeResults.index};
        node >> FileParse::Child{"Area", edgeResults.area};
        node >> FileParse::Child{"Flux", edgeResults.flux};
        node >> FileParse::Child{"HeatRate", edgeResults.heatRate};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const EdgeResults & edgeResults)
    {
        node << FileParse::Child{"Type", edgeResults.segmentType};
        node << FileParse::Child{"Index", edgeResults.index};
        node << FileParse::Child{"Area", edgeResults.area};
        node << FileParse::Child{"Flux", edgeResults.flux};
        node << FileParse::Child{"HeatRate", edgeResults.heatRate};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, TagNodes & caseTagResults)
    {
        node >> FileParse::Child{"Tag", caseTagResults.tag};
        node >> FileParse::Child{{"Nodes", "Node"}, caseTagResults.nodes};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const TagNodes & caseTagResults)
    {
        node << FileParse::Child{"Tag", caseTagResults.tag};
        node << FileParse::Child{{"Nodes", "Node"}, caseTagResults.nodes};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, CaseMeshResults & caseMeshResults)
    {
        node >> FileParse::Child{"ResultsType", caseMeshResults.resultsType};
        node >> FileParse::Child{"GlazingCase", caseMeshResults.glazingCase};
        node >> FileParse::Child{"SpacerCase", caseMeshResults.spacerCase};
        node >> FileParse::Child{{"Nodes", "Node"}, caseMeshResults.nodes};
        node >> FileParse::Child{{"ElementEdges", "Edge"}, caseMeshResults.elementEdges};
        node >> FileParse::Child{{"PolygonEdges", "Edge"}, caseMeshResults.polygonEdges};
        node >> FileParse::Child{{"TemperatureContourData", "NodeID"}, caseMeshResults.temperatureCountourNodeIDs};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const CaseMeshResults & caseMeshResults)
    {
        node << FileParse::Child{"ResultsType", caseMeshResults.resultsType};
        node << FileParse::Child{"GlazingCase", caseMeshResults.glazingCase};
        node << FileParse::Child{"SpacerCase", caseMeshResults.spacerCase};
        node << FileParse::Child{{"Nodes", "Node"}, caseMeshResults.nodes};
        node << FileParse::Child{{"ElementEdges", "Edge"}, caseMeshResults.elementEdges};
        node << FileParse::Child{{"PolygonEdges", "Edge"}, caseMeshResults.polygonEdges};
        node << FileParse::Child{{"TemperatureContourData", "NodeID"}, caseMeshResults.temperatureCountourNodeIDs};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, MeshResults & meshResults)
    {
        node >> FileParse::Child{"Version", meshResults.version};
        node >> FileParse::Child{"Case", meshResults.cases};
        node >> FileParse::Child{"TagNodes", meshResults.tagNodes};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const MeshResults & meshResults)
    {
        node << FileParse::Child{"Version", meshResults.version};
        node << FileParse::Child{"Case", meshResults.cases};
        node << FileParse::Child{"TagNodes", meshResults.tagNodes};

        return node;
    }
}   // namespace ThermFile