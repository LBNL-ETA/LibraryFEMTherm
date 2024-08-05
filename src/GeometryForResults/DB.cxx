#include "DB.hxx"

#include <fileParse/XMLNodeAdapter.hxx>

#include "Serializers.hxx"

#include "LibraryUtilities/Common.hxx"
#include "LibraryUtilities/StringRoutines.hxx"

namespace GeometryLibrary
{
    void InputGeometryDataRecord::addMaterialPolygon(const MaterialPolygon & materialPolygon)
    {
        materialPolygons.emplace_back(materialPolygon);
    }

    void InputGeometryDataRecord::addEdgeNode(const size_t nodeId)
    {
        modelEnclosure.node.emplace_back(nodeId);
    }

    void InputGeometryDataRecord::loadFromXMLFile(const std::string & fileName)
    {
        const auto xInputDataNode{getTopNodeFromFile(fileName, "InputGeometryData")};

        if(xInputDataNode.has_value())
        {
            const auto & node{xInputDataNode.value()};
            node >> FileParse::Child{"Version", version};
            node >> FileParse::Child{{"Materials", "Material"}, materials};
            node >> FileParse::Child{{"Nodes", "Node"}, nodes};
            node >> FileParse::Child{{"Elements", "Element"}, elements};
            node >> FileParse::Child{{"MaterialPolygons", "MaterialPolygon"}, materialPolygons};
            node >> modelEnclosure;
        }
    }

    void InputGeometryDataRecord::addMaterial(const size_t id,
                                              const std::string & name,
                                              const std::string & color)
    {
        materials.emplace_back(id, name, color);
    }

    int InputGeometryDataRecord::saveToXML(const std::string & fileName) const
    {
        XMLNodeAdapter node{createTopNode("InputGeometryData")};

        node << FileParse::Child{"Version", version};
        node << FileParse::Child{{"Materials", "Material"}, materials};
        node << FileParse::Child{{"Nodes", "Node"}, nodes};
        node << FileParse::Child{{"Elements", "Element"}, elements};
        node << FileParse::Child{{"MaterialPolygons", "MaterialPolygon"}, materialPolygons};
        node << modelEnclosure;

        return node.writeToFile(fileName);
    }

    void InputGeometryDataRecord::addNode(const size_t id, const double x, const double y)
    {
        nodes.emplace_back(id, x, y);
    }

    void InputGeometryDataRecord::addElement(
      size_t id, size_t node1, size_t node2, size_t node3, size_t node4, size_t materialID)
    {
        elements.emplace_back(id, node1, node2, node3, node4, materialID);
    }
}   // namespace GeometryLibrary