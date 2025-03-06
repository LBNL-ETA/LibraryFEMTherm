#pragma once

#include <string>
#include <vector>

namespace GeometryLibrary
{
    struct Material
    {
        size_t ID{0u};
        std::string Name{""};
        std::string Color{"0x000000"};
    };

    struct Node
    {
        size_t ID{0u};
        double x{0.0};
        double y{0.0};
    };

    struct Element
    {
        size_t id{0u};
        size_t nodeID1{0u};
        size_t nodeID2{0u};
        size_t nodeID3{0u};
        size_t nodeID4{0u};
        size_t materialID{0u};
    };

    struct MaterialPolygon
    {
        size_t id{0u};
        size_t materialID{0u};
        std::vector<size_t> m_Nodes{};
    };

    using ModelEnclosure = std::vector<size_t>;

}   // namespace ModelGeometryLibrary
