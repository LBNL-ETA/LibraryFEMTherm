#pragma once

#include <vector>

#include <fileParse/Vector.hxx>

#include "Serializers.hxx"
#include "Geometry.hxx"

namespace GeometryLibrary
{
    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, GeometryLibrary::Material & material)
    {
        node >> FileParse::Child{"ID", material.ID};
        node >> FileParse::Child{"Name", material.Name};
        node >> FileParse::Child{"Color", material.Color};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const GeometryLibrary::Material & material)
    {
        node << FileParse::Child{"ID", material.ID};
        node << FileParse::Child{"Name", material.Name};
        node << FileParse::Child{"Color", material.Color};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, std::vector<GeometryLibrary::Material> & materials)
    {
        node >> FileParse::Child{{"Materials", "Material"}, materials};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const std::vector<GeometryLibrary::Material> & materials)
    {
        node << FileParse::Child{{"Materials", "Material"}, materials};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, GeometryLibrary::Node & point)
    {
        node >> FileParse::Child{"ID", point.ID};
        node >> FileParse::Child{"x", point.x};
        node >> FileParse::Child{"y", point.y};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const GeometryLibrary::Node & point)
    {
        node << FileParse::Child{"ID", point.ID};
        node << FileParse::Child{"x", point.x};
        node << FileParse::Child{"y", point.y};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, std::vector<GeometryLibrary::Node> & nodes)
    {
        node >> FileParse::Child{{"Nodes", "Node"}, nodes};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const std::vector<GeometryLibrary::Node> & nodes)
    {
        node << FileParse::Child{{"Nodes", "Node"}, nodes};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, GeometryLibrary::Element & element)
    {
        node >> FileParse::Child{"ID", element.id};
        node >> FileParse::Child{"NodeID", element.nodeID1, 0u};
        node >> FileParse::Child{"NodeID", element.nodeID2, 1u};
        node >> FileParse::Child{"NodeID", element.nodeID3, 2u};
        node >> FileParse::Child{"NodeID", element.nodeID4, 3u};
        node >> FileParse::Child{"MaterialID", element.materialID};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const GeometryLibrary::Element & element)
    {
        node << FileParse::Child{"ID", element.id};
        node << FileParse::Child{"NodeID", element.nodeID1, 0u};
        node << FileParse::Child{"NodeID", element.nodeID2, 1u};
        node << FileParse::Child{"NodeID", element.nodeID3, 2u};
        node << FileParse::Child{"NodeID", element.nodeID4, 3u};
        node << FileParse::Child{"MaterialID", element.materialID};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, std::vector<GeometryLibrary::Element> & elements)
    {
        node >> FileParse::Child{{"Elements", "Element"}, elements};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const std::vector<GeometryLibrary::Element> & elements)
    {
        node << FileParse::Child{{"Elements", "Element"}, elements};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, GeometryLibrary::MaterialPolygon & materialPolygon)
    {
        node >> FileParse::Child{"ID", materialPolygon.id};
        node >> FileParse::Child{"MaterialID", materialPolygon.materialID};
        node >> FileParse::Child{"NodeID", materialPolygon.m_Nodes};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const GeometryLibrary::MaterialPolygon & materialPolygon)
    {
        node << FileParse::Child{"ID", materialPolygon.id};
        node << FileParse::Child{"MaterialID", materialPolygon.materialID};
        node << FileParse::Child{"NodeID", materialPolygon.m_Nodes};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, std::vector<GeometryLibrary::MaterialPolygon> & materialPolygons)
    {
        node >> FileParse::Child{{"MaterialPolygons", "MaterialPolygon"}, materialPolygons};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const std::vector<GeometryLibrary::MaterialPolygon> & materialPolygons)
    {
        node << FileParse::Child{{"MaterialPolygons", "MaterialPolygon"}, materialPolygons};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, GeometryLibrary::ModelEnclosure & modelEnclosure)
    {
        node >> FileParse::Child{{"ModelEnclosure", "NodeID"}, modelEnclosure};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const GeometryLibrary::ModelEnclosure & modelEnclosure)
    {
        node << FileParse::Child{{"ModelEnclosure", "NodeID"}, modelEnclosure};

        return node;
    }
}   // namespace GeometryLibrary