#pragma once

#include <string>
#include <vector>

#include <fileParse/FileFormat.hxx>

#include "Geometry.hxx"

namespace GeometryLibrary
{
    struct InputGeometryDataRecord
    {
        InputGeometryDataRecord() = default;

        void loadFromFile(const std::string & fileName);
        [[nodiscard]] int saveToFile(const std::string & fileName,
                                     FileParse::FileFormat format = FileParse::FileFormat::XML) const;

        void loadFromString(const std::string & str);
        [[nodiscard]] std::string saveToString(FileParse::FileFormat format = FileParse::FileFormat::XML) const;

        void add(size_t id, const std::string & name, const std::string & color);
        void addNode(size_t id, double x, double y);
        void addElement(size_t id, size_t node1, size_t node2, size_t node3, size_t node4, size_t materialID);

        void addPolygon(const MaterialPolygon & materialPolygon);

        void addEdgeNode(size_t nodeId);

        std::string version;
        std::vector<Material> materials;
        std::vector<Node> nodes;
        std::vector<Element> elements;
        std::vector<MaterialPolygon> materialPolygons;
        ModelEnclosure modelEnclosure;
    };
}   // namespace GeometryLibrary