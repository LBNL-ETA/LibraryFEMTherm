#pragma once

#include <string>
#include <vector>

namespace GeometryLibrary
{
    struct Material
    {
        Material() = default;
        Material(size_t id, const std::string & name, const std::string & color);

        size_t ID{0u};
        std::string Name;
        std::string Color{"0x000000"};
    };

    struct Node
    {
        Node() = default;
        Node(size_t id, double x, double y);

        size_t ID{0u};
        double x{0.0};
        double y{0.0};
    };

    struct Element
    {
        Element() = default;
        Element(size_t id, size_t node_id1, size_t node_id2, size_t node_id3, size_t node_id4, size_t material_id);

        size_t id{0u};
        size_t nodeID1{0u};
        size_t nodeID2{0u};
        size_t nodeID3{0u};
        size_t nodeID4{0u};
        size_t materialID{0u};
    };

    struct MaterialPolygon
    {
        MaterialPolygon() = default;
        MaterialPolygon(size_t id, size_t materialID);
        void addNode(size_t id);

        size_t id{0u};
        size_t materialID{0u};
        std::vector<size_t> m_Nodes;
    };

    struct ModelEnclosure
    {
        std::vector<size_t> node;
    };
}   // namespace ModelGeometryLibrary
