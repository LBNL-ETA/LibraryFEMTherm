#include "DB.hxx"

#include <fileParse/FileDataHandler.hxx>

#include "Serializers.hxx"

#include "LibraryUtilities/Common.hxx"
#include "LibraryUtilities/StringRoutines.hxx"

namespace GeometryLibrary
{
    void InputGeometryDataRecord::addPolygon(const MaterialPolygon & materialPolygon)
    {
        materialPolygons.emplace_back(materialPolygon);
    }

    void InputGeometryDataRecord::addEdgeNode(const size_t nodeId)
    {
        modelEnclosure.emplace_back(nodeId);
    }

    namespace Helper
    {
        template<typename NodeType>
        void loadFromNode(const NodeType & node,
                          std::string & version,
                          std::vector<Material> & materials,
                          std::vector<Node> & nodes,
                          std::vector<Element> & elements,
                          std::vector<MaterialPolygon> & materialPolygons,
                          ModelEnclosure & modelEnclosure)
        {
            node >> FileParse::Child{"Version", version};
            node >> FileParse::Child{{"Materials", "Material"}, materials};
            node >> FileParse::Child{{"Nodes", "Node"}, nodes};
            node >> FileParse::Child{{"Elements", "Element"}, elements};
            node >> FileParse::Child{{"MaterialPolygons", "MaterialPolygon"}, materialPolygons};
            node >> modelEnclosure;
        }

        template<typename NodeType>
        void saveToNode(NodeType & node,
                        const std::string & version,
                        const std::vector<Material> & materials,
                        const std::vector<Node> & nodes,
                        const std::vector<Element> & elements,
                        const std::vector<MaterialPolygon> & materialPolygons,
                        const ModelEnclosure & modelEnclosure)
        {
            node << FileParse::Child{"Version", version};
            node << FileParse::Child{{"Materials", "Material"}, materials};
            node << FileParse::Child{{"Nodes", "Node"}, nodes};
            node << FileParse::Child{{"Elements", "Element"}, elements};
            node << FileParse::Child{{"MaterialPolygons", "MaterialPolygon"}, materialPolygons};
            node << modelEnclosure;
        }
    }   // namespace Helper

    void InputGeometryDataRecord::loadFromFile(const std::string & fileName)
    {
        auto nodeOpt{Common::getTopNodeFromFile(fileName, "InputGeometryData")};

        if(nodeOpt.has_value())
        {
            std::visit(
              [this](auto & adapter) {
                  Helper::loadFromNode(adapter, version, materials, nodes, elements, materialPolygons, modelEnclosure);
              },
              nodeOpt.value());
        }
    }

    int InputGeometryDataRecord::saveToFile(const std::string & fileName, FileParse::FileFormat format) const
    {
        auto node{Common::createTopNode("InputGeometryData", format)};

        int result = 0;
        std::visit(
          [this, &fileName, &result](auto & adapter) {
              Helper::saveToNode(adapter, version, materials, nodes, elements, materialPolygons, modelEnclosure);
              result = adapter.writeToFile(fileName);
          },
          node);

        return result;
    }

    void InputGeometryDataRecord::loadFromString(const std::string & str)
    {
        auto nodeOpt{Common::getTopNodeFromString(str, "InputGeometryData")};

        if(nodeOpt.has_value())
        {
            std::visit(
              [this](auto & adapter) {
                  Helper::loadFromNode(adapter, version, materials, nodes, elements, materialPolygons, modelEnclosure);
              },
              nodeOpt.value());
        }
    }

    std::string InputGeometryDataRecord::saveToString(FileParse::FileFormat format) const
    {
        auto node{Common::createTopNode("InputGeometryData", format)};

        std::string content;
        std::visit(
          [this, &content](auto & adapter) {
              Helper::saveToNode(adapter, version, materials, nodes, elements, materialPolygons, modelEnclosure);
              content = adapter.getContent();
          },
          node);

        return content;
    }

    void InputGeometryDataRecord::add(const size_t id, const std::string & name, const std::string & color)
    {
        materials.push_back(Material{id, name, color});
    }

    void InputGeometryDataRecord::addNode(const size_t id, const double x, const double y)
    {
        nodes.push_back(Node{id, x, y});
    }

    void InputGeometryDataRecord::addElement(
      size_t id, size_t node1, size_t node2, size_t node3, size_t node4, size_t materialID)
    {
        elements.push_back(Element{id, node1, node2, node3, node4, materialID});
    }
}   // namespace GeometryLibrary