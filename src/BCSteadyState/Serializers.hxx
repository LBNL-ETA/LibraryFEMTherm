#pragma once

#include <vector>

#include <fileParse/Variant.hxx>

#include "Tags.hxx"
#include "SteadyState.hxx"


namespace BCSteadyStateLibrary
{
    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCSteadyStateLibrary::Convection & convection)
    {
        BCSteadyStateLibrary::Tags tag;

        node >> FileParse::Child{tag.temperature(), convection.temperature};
        node >> FileParse::Child{tag.filmCoefficient(), convection.filmCoefficient};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCSteadyStateLibrary::Convection & convection)
    {
        BCSteadyStateLibrary::Tags tag;
        node << FileParse::Child{tag.temperature(), convection.temperature};
        node << FileParse::Child{tag.filmCoefficient(), convection.filmCoefficient};
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCSteadyStateLibrary::ConstantFlux & constantFlux)
    {
        BCSteadyStateLibrary::Tags tag;
        node >> FileParse::Child{tag.flux(), constantFlux.flux};
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCSteadyStateLibrary::ConstantFlux & constantFlux)
    {
        BCSteadyStateLibrary::Tags tag;
        node << FileParse::Child{tag.flux(), constantFlux.flux};
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCSteadyStateLibrary::AutomaticEnclosure & automaticEnclosure)
    {
        BCSteadyStateLibrary::Tags tag;
        node >> FileParse::Child{tag.temperature(), automaticEnclosure.temperature};
        node >> FileParse::Child{tag.emissivity(), automaticEnclosure.emissivity};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCSteadyStateLibrary::AutomaticEnclosure & automaticEnclosure)
    {
        BCSteadyStateLibrary::Tags tag;
        node << FileParse::Child{tag.temperature(), automaticEnclosure.temperature};
        node << FileParse::Child{tag.emissivity(), automaticEnclosure.emissivity};
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCSteadyStateLibrary::ManualEnclosure &)
    {
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCSteadyStateLibrary::ManualEnclosure &)
    {
        BCSteadyStateLibrary::Tags tag;
        // TODO: Charlie wants emissivity here. emtpyNode used to avoid compilation warning [[nodicard]]
        const auto emptyNode{node.addChild(tag.manualEnclosure())};
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCSteadyStateLibrary::BlackBodyRadiation & blackBodyRadiation)
    {
        BCSteadyStateLibrary::Tags tag;
        node >> FileParse::Child{tag.temperature(), blackBodyRadiation.temperature};
        node >> FileParse::Child{tag.emissivity(), blackBodyRadiation.emissivity};
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCSteadyStateLibrary::BlackBodyRadiation & blackBodyRadiation)
    {
        BCSteadyStateLibrary::Tags tag;
        node << FileParse::Child{tag.temperature(), blackBodyRadiation.temperature};
        node << FileParse::Child{tag.emissivity(), blackBodyRadiation.emissivity};
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCSteadyStateLibrary::LinearizedRadiation & linearizedRadiation)
    {
        BCSteadyStateLibrary::Tags tag;
        node >> FileParse::Child{tag.temperature(), linearizedRadiation.temperature};
        node >> FileParse::Child{tag.filmCoefficient(), linearizedRadiation.filmCoefficient};
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCSteadyStateLibrary::LinearizedRadiation & linearizedRadiation)
    {
        BCSteadyStateLibrary::Tags tag;
        node << FileParse::Child{tag.temperature(), linearizedRadiation.temperature};
        node << FileParse::Child{tag.filmCoefficient(), linearizedRadiation.filmCoefficient};
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCSteadyStateLibrary::Radiation & radiation)
    {
        FileParse::deserializeOptionalVariant(
          node,
          {"AutomaticEnclosure", "ManualEnclosure", "BlackBodyRadiation", "LinearizedRadiation"},
          radiation.radiation);

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCSteadyStateLibrary::Radiation & radiation)
    {
        FileParse::serializeOptionalVariant(
          node,
          {"AutomaticEnclosure", "ManualEnclosure", "BlackBodyRadiation", "LinearizedRadiation"},
          radiation.radiation);

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCSteadyStateLibrary::Comprehensive & comprehensive)
    {
        BCSteadyStateLibrary::Tags tag;
        node >> FileParse::Child{tag.relativeHumidity(), comprehensive.relativeHumidity};
        node >> FileParse::Child{tag.convection(), comprehensive.convection};
        node >> FileParse::Child{tag.constantFlux(), comprehensive.constantFlux};
        node >> FileParse::Child{tag.radiation(), comprehensive.radiation};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCSteadyStateLibrary::Comprehensive & comprehensive)
    {
        BCSteadyStateLibrary::Tags tag;
        node << FileParse::Child{tag.relativeHumidity(), comprehensive.relativeHumidity};
        node << FileParse::Child{tag.convection(), comprehensive.convection};
        node << FileParse::Child{tag.constantFlux(), comprehensive.constantFlux};
        node << FileParse::Child{tag.radiation(), comprehensive.radiation};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCSteadyStateLibrary::Simplified & simplified)
    {
        BCSteadyStateLibrary::Tags tag;
        node >> FileParse::Child{tag.temperature(), simplified.temperature};
        node >> FileParse::Child{tag.filmCoefficient(), simplified.filmCoefficient};
        node >> FileParse::Child{tag.relativeHumidity(), simplified.relativeHumidity};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCSteadyStateLibrary::Simplified & simplified)
    {
        BCSteadyStateLibrary::Tags tag;
        node << FileParse::Child{tag.temperature(), simplified.temperature};
        node << FileParse::Child{tag.filmCoefficient(), simplified.filmCoefficient};
        node << FileParse::Child{tag.relativeHumidity(), simplified.relativeHumidity};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCSteadyStateLibrary::RadiationSurface & radiationSurface)
    {
        BCSteadyStateLibrary::Tags tag;
        node >> FileParse::Child{tag.temperature(), radiationSurface.temperature};
        node >> FileParse::Child{tag.emissivity(), radiationSurface.emissivity};
        node >> FileParse::Child{tag.isDefault(), radiationSurface.isDefault};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCSteadyStateLibrary::RadiationSurface & radiationSurface)
    {
        BCSteadyStateLibrary::Tags tag;
        node << FileParse::Child{tag.isDefault(), radiationSurface.isDefault};
        node << FileParse::Child{tag.temperature(), radiationSurface.temperature};
        node << FileParse::Child{tag.emissivity(), radiationSurface.emissivity};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCSteadyStateLibrary::BoundaryCondition & boundaryCondition)
    {
        BCSteadyStateLibrary::Tags tag;
        node >> FileParse::Child{tag.uuid(), boundaryCondition.UUID};
        node >> FileParse::Child{tag.name(), boundaryCondition.Name};
        node >> FileParse::Child{tag.protectedTag(), boundaryCondition.Protected};
        node >> FileParse::Child{tag.color(), boundaryCondition.Color};
        node >> FileParse::Child{tag.isIGUSurface(), boundaryCondition.isIGUSurface};
        FileParse::deserializeVariant(node, {"Comprehensive", "Simplified", "RadiationSurface"}, boundaryCondition.data);

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCSteadyStateLibrary::BoundaryCondition & boundaryCondition)
    {
        BCSteadyStateLibrary::Tags tag;
        node << FileParse::Child{tag.uuid(), boundaryCondition.UUID};
        node << FileParse::Child{tag.name(), boundaryCondition.Name};
        node << FileParse::Child{tag.protectedTag(), boundaryCondition.Protected};
        node << FileParse::Child{tag.color(), boundaryCondition.Color};
        node << FileParse::Child{tag.isIGUSurface(), boundaryCondition.isIGUSurface};
        FileParse::serializeVariant(node, {"Comprehensive", "Simplified", "RadiationSurface"}, boundaryCondition.data);

        return node;
    }
}