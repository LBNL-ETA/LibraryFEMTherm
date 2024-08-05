#pragma once

#include <fileParse/Optional.hxx>
#include <fileParse/Vector.hxx>

#include "Mesh.hxx"

#include "THMZ/Common/EnumSerializers.hxx"
#include "THMZ/CMA/EnumSerializers.hxx"

namespace ThermFile::Mesh
{
    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, Node & point)
    {
        node >> FileParse::Child{"index", point.index};
        node >> FileParse::Child{"x", point.x};
        node >> FileParse::Child{"y", point.y};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const Node & point)
    {
        node << FileParse::Child{"index", point.index};
        node << FileParse::Child{"x", point.x};
        node << FileParse::Child{"y", point.y};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, Element & element)
    {
        node >> FileParse::Child{"index", element.index};
        node >> FileParse::Child{"node1", element.node1};
        node >> FileParse::Child{"node2", element.node2};
        node >> FileParse::Child{"node3", element.node3};
        node >> FileParse::Child{"node4", element.node4};
        node >> FileParse::Child{"materialID", element.materialID};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const Element & element)
    {
        node << FileParse::Child{"index", element.index};
        node << FileParse::Child{"node1", element.node1};
        node << FileParse::Child{"node2", element.node2};
        node << FileParse::Child{"node3", element.node3};
        node << FileParse::Child{"node4", element.node4};
        node << FileParse::Child{"materialID", element.materialID};

        return node;
    }

    namespace Segment
    {
        template<typename NodeAdapter>
        inline const NodeAdapter & operator>>(const NodeAdapter & node, Segment::Flux & flux)
        {
            node >> FileParse::Child{"index", flux.index};
            node >> FileParse::Child{"node1", flux.node1};
            node >> FileParse::Child{"node2", flux.node2};
            node >> FileParse::Child{"flux1", flux.flux1};
            node >> FileParse::Child{"flux2", flux.flux2};
            node >> FileParse::Child{"label", flux.label};

            return node;
        }

        template<typename NodeAdapter>
        inline NodeAdapter & operator<<(NodeAdapter & node, const Segment::Flux & flux)
        {
            node << FileParse::Child{"index", flux.index};
            node << FileParse::Child{"node1", flux.node1};
            node << FileParse::Child{"node2", flux.node2};
            node << FileParse::Child{"flux1", flux.flux1};
            node << FileParse::Child{"flux2", flux.flux2};
            node << FileParse::Child{"label", flux.label};

            return node;
        }

        template<typename NodeAdapter>
        inline const NodeAdapter & operator>>(const NodeAdapter & node, Segment::Temperature & temperature)
        {
            node >> FileParse::Child{"node", temperature.node};
            node >> FileParse::Child{"temperature", temperature.temperature};
            node >> FileParse::Child{"label", temperature.label};

            return node;
        }

        template<typename NodeAdapter>
        inline NodeAdapter & operator<<(NodeAdapter & node, const Segment::Temperature & temperature)
        {
            node << FileParse::Child{"node", temperature.node};
            node << FileParse::Child{"temperature", temperature.temperature};
            node << FileParse::Child{"label", temperature.label};

            return node;
        }

        template<typename NodeAdapter>
        inline const NodeAdapter & operator>>(const NodeAdapter & node, Segment::Convection & convection)
        {
            node >> FileParse::Child{"index", convection.index};
            node >> FileParse::Child{"node1", convection.node1};
            node >> FileParse::Child{"node2", convection.node2};
            node >> FileParse::Child{"filmCoefficient", convection.filmCoefficient};
            node >> FileParse::Child{"temperature1", convection.temperature1};
            node >> FileParse::Child{"temperature2", convection.temperature2};
            node >> FileParse::Child{"label", convection.label};

            return node;
        }

        template<typename NodeAdapter>
        inline NodeAdapter & operator<<(NodeAdapter & node, const Segment::Convection & convection)
        {
            node << FileParse::Child{"index", convection.index};
            node << FileParse::Child{"node1", convection.node1};
            node << FileParse::Child{"node2", convection.node2};
            node << FileParse::Child{"filmCoefficient", convection.filmCoefficient};
            node << FileParse::Child{"temperature1", convection.temperature1};
            node << FileParse::Child{"temperature2", convection.temperature2};
            node << FileParse::Child{"label", convection.label};

            return node;
        }

        template<typename NodeAdapter>
        inline const NodeAdapter & operator>>(const NodeAdapter & node, Segment::Radiation & radiation)
        {
            node >> FileParse::Child{"index", radiation.index};
            node >> FileParse::Child{"node1", radiation.node1};
            node >> FileParse::Child{"node2", radiation.node2};
            node >> FileParse::Child{"filmCoefficient", radiation.filmCoefficient};
            node >> FileParse::Child{"temperature1", radiation.temperature1};
            node >> FileParse::Child{"temperature2", radiation.temperature2};

            return node;
        }

        template<typename NodeAdapter>
        inline NodeAdapter & operator<<(NodeAdapter & node, const Segment::Radiation & radiation)
        {
            node << FileParse::Child{"index", radiation.index};
            node << FileParse::Child{"node1", radiation.node1};
            node << FileParse::Child{"node2", radiation.node2};
            node << FileParse::Child{"filmCoefficient", radiation.filmCoefficient};
            node << FileParse::Child{"temperature1", radiation.temperature1};
            node << FileParse::Child{"temperature2", radiation.temperature2};

            return node;
        }

        template<typename NodeAdapter>
        inline const NodeAdapter & operator>>(const NodeAdapter & node,
                                              Segment::RadiationEnclosure & enclosureRadiation)
        {
            node >> FileParse::Child{"index", enclosureRadiation.index};
            node >> FileParse::Child{"node1", enclosureRadiation.node1};
            node >> FileParse::Child{"node2", enclosureRadiation.node2};
            node >> FileParse::Child{"condCode", enclosureRadiation.condCode};
            node >> FileParse::Child{"emissivity", enclosureRadiation.emissivity};
            node >> FileParse::Child{"temperature", enclosureRadiation.temperature};

            return node;
        }

        template<typename NodeAdapter>
        inline NodeAdapter & operator<<(NodeAdapter & node, const Segment::RadiationEnclosure & enclosureRadiation)
        {
            node << FileParse::Child{"index", enclosureRadiation.index};
            node << FileParse::Child{"node1", enclosureRadiation.node1};
            node << FileParse::Child{"node2", enclosureRadiation.node2};
            node << FileParse::Child{"condCode", enclosureRadiation.condCode};
            node << FileParse::Child{"emissivity", enclosureRadiation.emissivity};
            node << FileParse::Child{"temperature", enclosureRadiation.temperature};

            return node;
        }
    }   // namespace Segment

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, BCSegments & bcSegments)
    {
        node >> FileParse::Child{{"Fluxes", "Flux"}, bcSegments.fluxes};
        node >> FileParse::Child{{"Temperatures", "Temperature"}, bcSegments.temperatures};
        node >> FileParse::Child{{"Convections", "Convection"}, bcSegments.convections};
        node >> FileParse::Child{{"Radiations", "Radiation"}, bcSegments.radiations};
        node >> FileParse::Child{{"RadiationEnclosures", "RadiationEnclosure"}, bcSegments.radiationEnclosures};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const BCSegments & bcSegments)
    {
        node << FileParse::Child{{"Fluxes", "Flux"}, bcSegments.fluxes};
        node << FileParse::Child{{"Temperatures", "Temperature"}, bcSegments.temperatures};
        node << FileParse::Child{{"Convections", "Convection"}, bcSegments.convections};
        node << FileParse::Child{{"Radiations", "Radiation"}, bcSegments.radiations};
        node << FileParse::Child{{"RadiationEnclosures", "RadiationEnclosure"}, bcSegments.radiationEnclosures};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, Case & meshCase)
    {
        node >> FileParse::Child{"ModelType", meshCase.modelType};
        node >> FileParse::Child{"GlazingCase", meshCase.glazingCase};
        node >> FileParse::Child{"SpacerCase", meshCase.spacerCase};
        node >> FileParse::Child{{"Nodes", "Node"}, meshCase.nodes};
        node >> FileParse::Child{{"Elements", "Element"}, meshCase.elements};
        node >> FileParse::Child{"BCSegments", meshCase.bcSegments};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const Case & meshCase)
    {
        node << FileParse::Child{"ModelType", meshCase.modelType};
        node << FileParse::Child{"GlazingCase", meshCase.glazingCase};
        node << FileParse::Child{"SpacerCase", meshCase.spacerCase};
        node << FileParse::Child{{"Nodes", "Node"}, meshCase.nodes};
        node << FileParse::Child{{"Elements", "Element"}, meshCase.elements};
        node << FileParse::Child{"BCSegments", meshCase.bcSegments};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, Mesh & mesh)
    {
        // Intentionally put in root level of the Mesh
        node >> FileParse::Child{"Version", mesh.version};
        node >> FileParse::Child{"Case", mesh.cases};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const Mesh & mesh)
    {
        // Intentionally put in root level of the Mesh
        node << FileParse::Child{"Version", mesh.version};
        node << FileParse::Child{"Case", mesh.cases};

        return node;
    }
}   // namespace ThermFile::Mesh