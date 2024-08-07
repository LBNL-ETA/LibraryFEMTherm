#pragma once

#include <vector>
#include <optional>

#include "THMZ/Common/Enumerators.hxx"
#include "THMZ/CMA/Enumerators.hxx"

namespace ThermFile
{
    struct NodeResults
    {
        size_t index{0u};
        double temperature{0.0};
        double xFlux{0.0};
        double yFlux{0.0};
    };

    struct EdgeResults
    {
        SegmentType segmentType{SegmentType::Unknown};
        size_t index{0u};
        double area{0.0};
        double flux{0.0};
        double heatRate{0.0};
    };

    //! Structure to hold the edge results for a given surface tag.
    //! It is used in calculations for the condensation resistance.
    struct TagNodes
    {
        std::string tag;
        std::vector<size_t> nodes;
    };

    struct CaseMeshResults
    {
        std::optional<RunType> resultsType;
        std::optional<CMACase> glazingCase;
        std::optional<CMACase> spacerCase;
        std::vector<NodeResults> nodes;
        std::vector<EdgeResults> elementEdges;
        std::vector<EdgeResults> polygonEdges;
        std::vector<size_t> temperatureCountourNodeIDs;
    };

    struct MeshResults
    {
        std::string version{"1"};
        std::vector<CaseMeshResults> cases;
        std::vector<TagNodes> tagNodes;
    };
}   // namespace ThermFile