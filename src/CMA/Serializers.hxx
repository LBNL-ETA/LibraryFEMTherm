#pragma once

#include <map>

#include <fileParse/Map.hxx>
#include <fileParse/Enum.hxx>

#include "CMA.hxx"
#include "Tags.hxx"

namespace CMALibrary
{
    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, CMALibrary::Environment & environment)
    {
        CMALibrary::Tags tag;
        node >> FileParse::Child(tag.indoorTemperatureTag(), environment.Tin);
        node >> FileParse::Child(tag.outdoorTemperatureTag(), environment.Tout);
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const CMALibrary::Environment & environment)
    {
        CMALibrary::Tags tag;
        node << FileParse::Child(tag.indoorTemperatureTag(), environment.Tin);
        node << FileParse::Child(tag.outdoorTemperatureTag(), environment.Tout);
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, std::map<CMALibrary::Option, double> & options)
    {
        node >> FileParse::Child(CMALibrary::optionToString(CMALibrary::Option::Low), options[CMALibrary::Option::Low]);
        node >> FileParse::Child(CMALibrary::optionToString(CMALibrary::Option::High), options[CMALibrary::Option::High]);

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const std::map<CMALibrary::Option, double> & options)
    {
        node << FileParse::Child{CMALibrary::optionToString(CMALibrary::Option::Low), options.at(CMALibrary::Option::Low)};
        node << FileParse::Child{CMALibrary::optionToString(CMALibrary::Option::High), options.at(CMALibrary::Option::High)};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, CMALibrary::LayerValues & layerValues)
    {
        CMALibrary::Tags tag;

        node >> FileParse::Child(tag.hcInTag(), layerValues.hcin);
        node >> FileParse::Child(tag.hcOutTag(), layerValues.hcout);
        node >> FileParse::Child(tag.emissivityInTag(), layerValues.emissivityIn);
        node >> FileParse::Child(tag.emissivityOutTag(), layerValues.emissivityOut);

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const CMALibrary::LayerValues & layerValues)
    {
        CMALibrary::Tags tag;

        node << FileParse::Child(tag.hcInTag(), layerValues.hcin);
        node << FileParse::Child(tag.hcOutTag(), layerValues.hcout);
        node << FileParse::Child(tag.emissivityInTag(), layerValues.emissivityIn);
        node << FileParse::Child(tag.emissivityOutTag(), layerValues.emissivityOut);

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, CMALibrary::Option & option)
    {
        FileParse::deserializeEnum<NodeAdapter, Option>(
          node, option, CMALibrary::optionFromString);

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, CMALibrary::Option & option)
    {
        FileParse::serializeEnum<NodeAdapter, Option>(
          node, option, CMALibrary::optionToString);

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, CMALibrary::LayerOptionsKey & layerOptionsKey)
    {
        CMALibrary::Tags tag;

        node >> FileParse::Child{tag.glazingOptionTag(), layerOptionsKey.glazing};
        node >> FileParse::Child{tag.spacerOptionTag(), layerOptionsKey.spacer};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(const NodeAdapter & node, const CMALibrary::LayerOptionsKey & layerOptionsKey)
    {
        CMALibrary::Tags tag;

        node << FileParse::Child{tag.glazingOptionTag(), layerOptionsKey.glazing};
        node << FileParse::Child{tag.spacerOptionTag(), layerOptionsKey.spacer};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, CMALibrary::SingleLayer & singleLayer)
    {
        CMALibrary::Tags tag;

        node >> FileParse::Child(tag.thicknessTag(), singleLayer.thickness);
        node >> FileParse::Child(tag.conductivityTag(), singleLayer.conductivity);
        FileParse::deserializeMapAsChilds(node, "Options", singleLayer.layerOptions);

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const CMALibrary::SingleLayer & singleLayer)
    {
        CMALibrary::Tags tag;

        node << FileParse::Child(tag.thicknessTag(), singleLayer.thickness);
        node << FileParse::Child(tag.conductivityTag(), singleLayer.conductivity);
        FileParse::serializeMapAsChilds(node, "Options", singleLayer.layerOptions);

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, CMALibrary::DoubleLayer & doubleLayer)
    {
        CMALibrary::Tags tag;

        node >> FileParse::Child(tag.thickness1Tag(), doubleLayer.thickness1);
        node >> FileParse::Child(tag.thickness2Tag(), doubleLayer.thickness2);
        node >> FileParse::Child(tag.conductivity1Tag(), doubleLayer.conductivity1);
        node >> FileParse::Child(tag.conductivity2Tag(), doubleLayer.conductivity2);
        FileParse::deserializeMapAsChilds(node, "Options", doubleLayer.layerOptions);

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(const NodeAdapter & node, const CMALibrary::DoubleLayer & doubleLayer)
    {
        CMALibrary::Tags tag;

        node << FileParse::Child(tag.thickness1Tag(), doubleLayer.thickness1);
        node << FileParse::Child(tag.thickness2Tag(), doubleLayer.thickness2);
        node << FileParse::Child(tag.conductivity1Tag(), doubleLayer.conductivity1);
        node << FileParse::Child(tag.conductivity2Tag(), doubleLayer.conductivity2);
        FileParse::serializeMapAsChilds(node, "Options", doubleLayer.layerOptions);

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, CMALibrary::CMAData & cmaData)
    {
        CMALibrary::Tags tag;
        node >> FileParse::Child{"Version", cmaData.version};
        node >> FileParse::Child{tag.environmentTag(), cmaData.environment};
        node >> FileParse::Child{tag.glazingConductanceTag(), cmaData.glazingConductance};
        node >> FileParse::Child{tag.spacerConductivityTag(), cmaData.spacerConductivity};
        node >> FileParse::Child{tag.singleLayerTag(), cmaData.singleLayer};
        node >> FileParse::Child{tag.doubleLayerTag(), cmaData.doubleLayer};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const CMALibrary::CMAData & cmaData)
    {
        CMALibrary::Tags tag;
        node << FileParse::Child{"Version", cmaData.version};
        node << FileParse::Child{tag.environmentTag(), cmaData.environment};
        node << FileParse::Child{tag.glazingConductanceTag(), cmaData.glazingConductance};
        node << FileParse::Child{tag.spacerConductivityTag(), cmaData.spacerConductivity};
        node << FileParse::Child{tag.singleLayerTag(), cmaData.singleLayer};
        node << FileParse::Child{tag.doubleLayerTag(), cmaData.doubleLayer};

        return node;
    }
}   // namespace CMALibrary