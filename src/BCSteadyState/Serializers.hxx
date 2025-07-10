#pragma once

#include <vector>

#include <fileParse/Variant.hxx>

#include "Tags.hxx"
#include "SteadyState.hxx"


namespace BCSteadyStateLibrary
{
    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, Convection & convection)
    {
        Tags tag;

        node >> FileParse::Child{tag.temperature(), convection.temperature};
        node >> FileParse::Child{tag.filmCoefficient(), convection.filmCoefficient};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const Convection & convection)
    {
        BCSteadyStateLibrary::Tags tag;
        node << FileParse::Child{tag.temperature(), convection.temperature};
        node << FileParse::Child{tag.filmCoefficient(), convection.filmCoefficient};
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ConstantFlux & constantFlux)
    {
        Tags tag;
        node >> FileParse::Child{tag.flux(), constantFlux.flux};
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ConstantFlux & constantFlux)
    {
        Tags tag;
        node << FileParse::Child{tag.flux(), constantFlux.flux};
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, AutomaticEnclosure & automaticEnclosure)
    {
        Tags tag;
        node >> FileParse::Child{tag.temperature(), automaticEnclosure.temperature};
        node >> FileParse::Child{tag.emissivity(), automaticEnclosure.emissivity};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const AutomaticEnclosure & automaticEnclosure)
    {
        Tags tag;
        node << FileParse::Child{tag.temperature(), automaticEnclosure.temperature};
        node << FileParse::Child{tag.emissivity(), automaticEnclosure.emissivity};
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ManualEnclosure &)
    {
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ManualEnclosure &)
    {
        Tags tag;
        // TODO: Charlie wants emissivity here. emtpyNode used to avoid compilation warning [[nodicard]]
        const auto emptyNode{node.addChild(tag.manualEnclosure())};
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BlackBodyRadiation & blackBodyRadiation)
    {
        Tags tag;
        node >> FileParse::Child{tag.temperature(), blackBodyRadiation.temperature};
        node >> FileParse::Child{tag.emissivity(), blackBodyRadiation.emissivity};
        node >> FileParse::Child{tag.viewFactor(), blackBodyRadiation.viewFactor};
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BlackBodyRadiation & blackBodyRadiation)
    {
        Tags tag;
        node << FileParse::Child{tag.temperature(), blackBodyRadiation.temperature};
        node << FileParse::Child{tag.emissivity(), blackBodyRadiation.emissivity};
        node << FileParse::Child{tag.viewFactor(), blackBodyRadiation.viewFactor};
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, LinearizedRadiation & linearizedRadiation)
    {
        Tags tag;
        node >> FileParse::Child{tag.temperature(), linearizedRadiation.temperature};
        node >> FileParse::Child{tag.filmCoefficient(), linearizedRadiation.filmCoefficient};
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const LinearizedRadiation & linearizedRadiation)
    {
        Tags tag;
        node << FileParse::Child{tag.temperature(), linearizedRadiation.temperature};
        node << FileParse::Child{tag.filmCoefficient(), linearizedRadiation.filmCoefficient};
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, Radiation & radiation)
    {
        FileParse::deserializeOptionalVariant(
          node,
          {"AutomaticEnclosure", "ManualEnclosure", "BlackBodyRadiation", "LinearizedRadiation"},
          radiation.radiation);

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const Radiation & radiation)
    {
        FileParse::serializeOptionalVariant(
          node,
          {"AutomaticEnclosure", "ManualEnclosure", "BlackBodyRadiation", "LinearizedRadiation"},
          radiation.radiation);

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, Comprehensive & comprehensive)
    {
        Tags tag;
        node >> FileParse::Child{tag.relativeHumidity(), comprehensive.relativeHumidity};
        node >> FileParse::Child{tag.convection(), comprehensive.convection};
        node >> FileParse::Child{tag.constantFlux(), comprehensive.constantFlux};
        node >> FileParse::Child{tag.radiation(), comprehensive.radiation};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const Comprehensive & comprehensive)
    {
        Tags tag;
        node << FileParse::Child{tag.relativeHumidity(), comprehensive.relativeHumidity};
        node << FileParse::Child{tag.convection(), comprehensive.convection};
        node << FileParse::Child{tag.constantFlux(), comprehensive.constantFlux};
        node << FileParse::Child{tag.radiation(), comprehensive.radiation};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, Simplified & simplified)
    {
        Tags tag;
        node >> FileParse::Child{tag.temperature(), simplified.temperature};
        node >> FileParse::Child{tag.filmCoefficient(), simplified.filmCoefficient};
        node >> FileParse::Child{tag.relativeHumidity(), simplified.relativeHumidity};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const Simplified & simplified)
    {
        Tags tag;
        node << FileParse::Child{tag.temperature(), simplified.temperature};
        node << FileParse::Child{tag.filmCoefficient(), simplified.filmCoefficient};
        node << FileParse::Child{tag.relativeHumidity(), simplified.relativeHumidity};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, RadiationSurface & radiationSurface)
    {
        Tags tag;
        node >> FileParse::Child{tag.temperature(), radiationSurface.temperature};
        node >> FileParse::Child{tag.emissivity(), radiationSurface.emissivity};
        node >> FileParse::Child{tag.isDefault(), radiationSurface.isDefault};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const RadiationSurface & radiationSurface)
    {
        Tags tag;
        node << FileParse::Child{tag.isDefault(), radiationSurface.isDefault};
        node << FileParse::Child{tag.temperature(), radiationSurface.temperature};
        node << FileParse::Child{tag.emissivity(), radiationSurface.emissivity};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BoundaryCondition & boundaryCondition)
    {
        Tags tag;
        node >> FileParse::Child{tag.uuid(), boundaryCondition.UUID};
        node >> FileParse::Child{tag.name(), boundaryCondition.Name};
        node >> FileParse::Child{tag.projectName(), boundaryCondition.ProjectName};
        node >> FileParse::Child{tag.protectedTag(), boundaryCondition.Protected};
        node >> FileParse::Child{tag.color(), boundaryCondition.Color};
        node >> FileParse::Child{tag.isIGUSurface(), boundaryCondition.isIGUSurface};
        FileParse::deserializeVariant(
          node, {"Comprehensive", "Simplified", "RadiationSurface"}, boundaryCondition.data);

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BoundaryCondition & boundaryCondition)
    {
        Tags tag;
        node << FileParse::Child{tag.uuid(), boundaryCondition.UUID};
        node << FileParse::Child{tag.name(), boundaryCondition.Name};
        node << FileParse::Child{tag.projectName(), boundaryCondition.ProjectName};
        node << FileParse::Child{tag.protectedTag(), boundaryCondition.Protected};
        node << FileParse::Child{tag.color(), boundaryCondition.Color};
        node << FileParse::Child{tag.isIGUSurface(), boundaryCondition.isIGUSurface};
        FileParse::serializeVariant(node, {"Comprehensive", "Simplified", "RadiationSurface"}, boundaryCondition.data);

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, std::vector<BoundaryCondition> & boundaryCondition)
    {
        Tags tag;
        node >> FileParse::Child{tag.boundaryCondition(), boundaryCondition};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const std::vector<BoundaryCondition> & boundaryCondition)
    {
        Tags tag;
        node << FileParse::Child{tag.boundaryCondition(), boundaryCondition};

        return node;
    }
}   // namespace BCSteadyStateLibrary