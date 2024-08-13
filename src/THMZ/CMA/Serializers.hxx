#pragma once

#include <fileParse/Base.hxx>
#include <fileParse/Map.hxx>
#include <fileParse/Vector.hxx>
#include <fileParse/Variant.hxx>

#include "THMZ/CMA/CMA.hxx"
#include "THMZ/CMA/EnumSerializers.hxx"

namespace ThermFile
{
    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, CMABCContainer & cmaBCContainer)
    {
        const auto childNodes{node.getChildNodesByName("Position")};
        for(const auto & childNode : childNodes)
        {
            std::string value;
            CMABCPosition key;

            childNode >> FileParse::Child{"Value", key};
            childNode >> FileParse::Child{"UUID", value};

            //! Only first inserted key is stored. If the key already exists, the value is not updated.
            cmaBCContainer.emplace(key, value);
        }

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const CMABCContainer & cmaBCContainer)
    {
        if(cmaBCContainer.empty())
            return node;

        for(const auto & [enumKey, mapValue] : cmaBCContainer)
        {
            auto childNode{node.addChild("Position")};
            childNode << FileParse::Child{"Value", enumKey};
            childNode << FileParse::Child{"UUID", mapValue};
        }

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, CMAGapSpacer & bestWorstValues)
    {
        node >> FileParse::Child{"GlazingOption", bestWorstValues.glazingCase};
        node >> FileParse::Child{"SpacerOption", bestWorstValues.spacerCase};
        node >> FileParse::Child{"GapConductance", bestWorstValues.gapConductance};
        node >> FileParse::Child{"SpacerConductivity", bestWorstValues.spacerConductivity};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const CMAGapSpacer & bestWorstValues)
    {
        node << FileParse::Child{"GlazingOption", bestWorstValues.glazingCase};
        node << FileParse::Child{"SpacerOption", bestWorstValues.spacerCase};
        node << FileParse::Child{"GapConductance", bestWorstValues.gapConductance};
        node << FileParse::Child{"SpacerConductivity", bestWorstValues.spacerConductivity};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, CMALayerInput & cmaLayerInput)
    {
        node >> FileParse::Child{"Glazing", cmaLayerInput.glazingCase};
        node >> FileParse::Child{"Spacer", cmaLayerInput.spacerCase};
        node >> FileParse::Child{"hcin", cmaLayerInput.hcin};
        node >> FileParse::Child{"hcout", cmaLayerInput.hcout};
        node >> FileParse::Child{"EmissivityIn", cmaLayerInput.emissivityIn};
        node >> FileParse::Child{"EmissivityOut", cmaLayerInput.emissivityOut};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const CMALayerInput & cmaLayerInput)
    {
        node << FileParse::Child{"Glazing", cmaLayerInput.glazingCase};
        node << FileParse::Child{"Spacer", cmaLayerInput.spacerCase};
        node << FileParse::Child{"hcin", cmaLayerInput.hcin};
        node << FileParse::Child{"hcout", cmaLayerInput.hcout};
        node << FileParse::Child{"EmissivityIn", cmaLayerInput.emissivityIn};
        node << FileParse::Child{"EmissivityOut", cmaLayerInput.emissivityOut};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, CMASingleLayer & cmaSingleLayer)
    {
        node >> FileParse::Child{"Thickness", cmaSingleLayer.thickness};
        node >> FileParse::Child{"Conductivity", cmaSingleLayer.conductivity};
        node >> FileParse::Child{"Option", cmaSingleLayer.option};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const CMADoubleLayer & cmaDoubleLayer)
    {
        node << FileParse::Child{"ThicknessIn", cmaDoubleLayer.thicknessIn};
        node << FileParse::Child{"ThicknessOut", cmaDoubleLayer.thicknessOut};
        node << FileParse::Child{"ConductivityIn", cmaDoubleLayer.conductivityIn};
        node << FileParse::Child{"ConductivityOut", cmaDoubleLayer.conductivityOut};
        node << FileParse::Child{"Option", cmaDoubleLayer.option};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, CMADoubleLayer & cmaDoubleLayer)
    {
        node >> FileParse::Child{"ThicknessIn", cmaDoubleLayer.thicknessIn};
        node >> FileParse::Child{"ThicknessOut", cmaDoubleLayer.thicknessOut};
        node >> FileParse::Child{"ConductivityIn", cmaDoubleLayer.conductivityIn};
        node >> FileParse::Child{"ConductivityOut", cmaDoubleLayer.conductivityOut};
        node >> FileParse::Child{"Option", cmaDoubleLayer.option};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const CMASingleLayer & cmaSingleLayer)
    {
        node << FileParse::Child{"Thickness", cmaSingleLayer.thickness};
        node << FileParse::Child{"Conductivity", cmaSingleLayer.conductivity};
        node << FileParse::Child{"Option", cmaSingleLayer.option};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::CMAData & cmaData)
    {
        node >> FileParse::Child{"IGUType", cmaData.iguType};
        node >> FileParse::Child{"InteriorTemperature", cmaData.interiorTemperature};
        node >> FileParse::Child{"ExteriorTemperature", cmaData.exteriorTemperature};
        node >> FileParse::Child{"Case", cmaData.gapSpacerInput};
        FileParse::deserializeVariant(node, {"SingleLayer", "DoubleLayer"}, cmaData.layerInput);
        node >> FileParse::Child{"CMABCPositions", cmaData.cmaBCPositions};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::CMAData & cmaData)
    {
        node << FileParse::Child{"IGUType", cmaData.iguType};
        node << FileParse::Child{"InteriorTemperature", cmaData.interiorTemperature};
        node << FileParse::Child{"ExteriorTemperature", cmaData.exteriorTemperature};
        node << FileParse::Child{"Case", cmaData.gapSpacerInput};
        FileParse::serializeVariant(node, {"SingleLayer", "DoubleLayer"}, cmaData.layerInput);
        node << FileParse::Child{"CMABCPositions", cmaData.cmaBCPositions};

        return node;
    }
}   // namespace ThermFile