#include <iostream>
#include <sstream>

#include "Geometry.hxx"

namespace GeometryLibrary
{
    Material::Material(size_t id, const std::string & name, const std::string & color) :
        ID(id), Name(name), Color(color)
    {}

    Node::Node(size_t id, double x, double y) : ID(id), x(x), y(y)
    {}

    Element::Element(
      size_t elementID, size_t node_id1, size_t node_id2, size_t node_id3, size_t node_id4, size_t material_id) :
        id(elementID),
        nodeID1(node_id1),
        nodeID2(node_id2),
        nodeID3(node_id3),
        nodeID4(node_id4),
        materialID(material_id)
    {}

    MaterialPolygon::MaterialPolygon(size_t id, size_t materialID) : id(id), materialID(materialID)
    {}

    void MaterialPolygon::addNode(size_t nodeID)
    {
        m_Nodes.emplace_back(nodeID);
    }

}   // namespace GeometryLibrary
