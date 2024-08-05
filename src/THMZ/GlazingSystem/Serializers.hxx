#pragma once

#include "GlazingSystem.hxx"

#include <fileParse/Map.hxx>
#include <fileParse/Vector.hxx>

#include "EnumSerializers.hxx"

namespace ThermFile
{
    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node,
                                   ThermFile::EnvironmentCoefficients & environmentCoefficients)
    {
        node >> FileParse::Child{"H", environmentCoefficients.h};
        node >> FileParse::Child{"Hc", environmentCoefficients.hc};
        node >> FileParse::Child{"Hr", environmentCoefficients.hr};
        node >> FileParse::Child{"Temperature", environmentCoefficients.temperature};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::EnvironmentCoefficients & environmentCoefficients)
    {
        node << FileParse::Child{"H", environmentCoefficients.h};
        node << FileParse::Child{"Hc", environmentCoefficients.hc};
        node << FileParse::Child{"Hr", environmentCoefficients.hr};
        node << FileParse::Child{"Temperature", environmentCoefficients.temperature};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node,
                                   std::map<ThermFile::Position, ThermFile::EnvironmentCoefficients> & coefficients)
    {
        node >> FileParse::Child{ThermFile::positionToString(ThermFile::Position::Exterior),
                                              coefficients[ThermFile::Position::Exterior]};
        node >> FileParse::Child{ThermFile::positionToString(ThermFile::Position::Interior),
                                              coefficients[ThermFile::Position::Interior]};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node,
                             const std::map<ThermFile::Position, ThermFile::EnvironmentCoefficients> & coefficients)
    {
        node << FileParse::Child{ThermFile::positionToString(ThermFile::Position::Exterior),
                                              coefficients.at(ThermFile::Position::Exterior)};
        node << FileParse::Child{ThermFile::positionToString(ThermFile::Position::Interior),
                                              coefficients.at(ThermFile::Position::Interior)};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node,
                                   ThermFile::GlazingSystemProperties & glazingSystemProperties)
    {
        node
          >> FileParse::Child{"EnvironmentalCondition", glazingSystemProperties.environmentalCondition};
        node
          >> FileParse::Child{"BoundaryConditionStyle", glazingSystemProperties.boundaryConditionStyle};
        node >> FileParse::Child{"U-value", glazingSystemProperties.uValue};
        node >> FileParse::Child{"Coefficients", glazingSystemProperties.coefficients};
        node >> FileParse::Child{"Solar", glazingSystemProperties.solar};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::GlazingSystemProperties & glazingSystemProperties)
    {
        node << FileParse::Child{"EnvironmentalCondition",
                                              glazingSystemProperties.environmentalCondition};
        node << FileParse::Child{"BoundaryConditionStyle",
                                              glazingSystemProperties.boundaryConditionStyle};
        node << FileParse::Child{"U-value", glazingSystemProperties.uValue};
        node << FileParse::Child{"Coefficients", glazingSystemProperties.coefficients};
        node << FileParse::Child{"Solar", glazingSystemProperties.solar};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::IGUSurface & iguSurface)
    {
        node >> FileParse::Child{"Temperature", iguSurface.temperature};
        node >> FileParse::Child{"Emissivity", iguSurface.emissivity};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::IGUSurface & iguSurface)
    {
        node << FileParse::Child{"Temperature", iguSurface.temperature};
        node << FileParse::Child{"Emissivity", iguSurface.emissivity};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::IGULayer & iguLayer)
    {
        node >> FileParse::Child{"ID", iguLayer.ID};
        node >> FileParse::Child{"Name", iguLayer.name};
        node >> FileParse::Child{"Thickness", iguLayer.thickness};
        node >> FileParse::Child{"Conductivity", iguLayer.conductivity};
        node >> FileParse::Child{"Surface1", iguLayer.surface1};
        node >> FileParse::Child{"Surface2", iguLayer.surface2};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::IGULayer & iguLayer)
    {
        node << FileParse::Child{"ID", iguLayer.ID};
        node << FileParse::Child{"Name", iguLayer.name};
        node << FileParse::Child{"Thickness", iguLayer.thickness};
        node << FileParse::Child{"Conductivity", iguLayer.conductivity};
        node << FileParse::Child{"Surface1", iguLayer.surface1};
        node << FileParse::Child{"Surface2", iguLayer.surface2};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::IGUGasCoefficients & iguGasCoefficients)
    {
        node >> FileParse::Child{"A", iguGasCoefficients.A};
        node >> FileParse::Child{"B", iguGasCoefficients.B};
        node >> FileParse::Child{"C", iguGasCoefficients.C};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::IGUGasCoefficients & iguGasCoefficients)
    {
        node << FileParse::Child{"A", iguGasCoefficients.A};
        node << FileParse::Child{"B", iguGasCoefficients.B};
        node << FileParse::Child{"C", iguGasCoefficients.C};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::IGUGasProperties & iguGasProperties)
    {
        node >> FileParse::Child{"Name", iguGasProperties.name};
        node >> FileParse::Child{"MolecularWeight", iguGasProperties.molecularWeight};
        node >> FileParse::Child{"Conductivity", iguGasProperties.conductivity};
        node >> FileParse::Child{"Viscosity", iguGasProperties.viscosity};
        node >> FileParse::Child{"SpecificHeat", iguGasProperties.specificHeat};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::IGUGasProperties & iguGasProperties)
    {
        node << FileParse::Child{"Name", iguGasProperties.name};
        node << FileParse::Child{"MolecularWeight", iguGasProperties.molecularWeight};
        node << FileParse::Child{"Conductivity", iguGasProperties.conductivity};
        node << FileParse::Child{"Viscosity", iguGasProperties.viscosity};
        node << FileParse::Child{"SpecificHeat", iguGasProperties.specificHeat};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::IGUPureGas & iguPureGas)
    {
        node >> FileParse::Child{"Fraction", iguPureGas.fraction};
        node >> FileParse::Child{"Properties", iguPureGas.properties};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::IGUPureGas & iguPureGas)
    {
        node << FileParse::Child{"Fraction", iguPureGas.fraction};
        node << FileParse::Child{"Properties", iguPureGas.properties};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::IGUGas & iguGas)
    {
        node >> FileParse::Child{"ID", iguGas.ID};
        node >> FileParse::Child{"Name", iguGas.name};
        node >> FileParse::Child{"Thickness", iguGas.thickness};
        node >> FileParse::Child{"Conductivity", iguGas.conductivity};
        node >> FileParse::Child{{"Components", "Component"}, iguGas.components};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::IGUGas & iguGas)
    {
        node << FileParse::Child{"ID", iguGas.ID};
        node << FileParse::Child{"Name", iguGas.name};
        node << FileParse::Child{"Thickness", iguGas.thickness};
        node << FileParse::Child{"Conductivity", iguGas.conductivity};
        node << FileParse::Child{{"Components", "Component"}, iguGas.components};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::ShadeProperties & shadeProperties)
    {
        node >> FileParse::Child{"DBottom", shadeProperties.dBottom};
        node >> FileParse::Child{"DTop", shadeProperties.dTop};
        node >> FileParse::Child{"DLeft", shadeProperties.dLeft};
        node >> FileParse::Child{"DRight", shadeProperties.dRight};
        node >> FileParse::Child{"GapKeff", shadeProperties.gapKeff};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::ShadeProperties & shadeProperties)
    {
        node << FileParse::Child{"DBottom", shadeProperties.dBottom};
        node << FileParse::Child{"DTop", shadeProperties.dTop};
        node << FileParse::Child{"DLeft", shadeProperties.dLeft};
        node << FileParse::Child{"DRight", shadeProperties.dRight};
        node << FileParse::Child{"GapKeff", shadeProperties.gapKeff};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::IGUShadeLayer & iguShadeLayer)
    {
        node >> FileParse::Child{"Position", iguShadeLayer.position};
        node >> FileParse::Child{"EmissivityRatio", iguShadeLayer.emissivityRatio};
        node >> FileParse::Child{"HcRatio", iguShadeLayer.hcRatio};
        node >> FileParse::Child{"Thickness", iguShadeLayer.thickness};
        node >> FileParse::Child{"GapThickness", iguShadeLayer.gapThickness};
        node >> FileParse::Child{"ModeledHeight", iguShadeLayer.modeledHeight};
        node >> FileParse::Child{"Properties", iguShadeLayer.properties};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::IGUShadeLayer & iguShadeLayer)
    {
        node << FileParse::Child{"Position", iguShadeLayer.position};
        node << FileParse::Child{"EmissivityRatio", iguShadeLayer.emissivityRatio};
        node << FileParse::Child{"HcRatio", iguShadeLayer.hcRatio};
        node << FileParse::Child{"Thickness", iguShadeLayer.thickness};
        node << FileParse::Child{"GapThickness", iguShadeLayer.gapThickness};
        node << FileParse::Child{"ModeledHeight", iguShadeLayer.modeledHeight};
        node << FileParse::Child{"Properties", iguShadeLayer.properties};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node,
                                   std::map<ThermFile::EnvironmentalCondition, GlazingSystemProperties> & properties)
    {
        node >> FileParse::Child{
          ThermFile::environmentalConditionsToString(ThermFile::EnvironmentalCondition::Winter),
          properties[ThermFile::EnvironmentalCondition::Winter]};
        node >> FileParse::Child{
          ThermFile::environmentalConditionsToString(ThermFile::EnvironmentalCondition::Summer),
          properties[ThermFile::EnvironmentalCondition::Summer]};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node,
                             const std::map<ThermFile::EnvironmentalCondition, GlazingSystemProperties> & properties)
    {
        node << FileParse::Child{
          ThermFile::environmentalConditionsToString(ThermFile::EnvironmentalCondition::Winter),
          properties.at(ThermFile::EnvironmentalCondition::Winter)};
        node << FileParse::Child{
          ThermFile::environmentalConditionsToString(ThermFile::EnvironmentalCondition::Summer),
          properties.at(ThermFile::EnvironmentalCondition::Summer)};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::GlazingSystem & glazingSystem)
    {
        node >> FileParse::Child{"ID", glazingSystem.ID};
        node >> FileParse::Child{"Index", glazingSystem.index};
        node >> FileParse::Child{"Name", glazingSystem.name};
        node >> FileParse::Child{{"GlazingSystemProperties", "Property"}, glazingSystem.properties};
        node >> FileParse::Child{{"Layers", "Layer"}, glazingSystem.layers};
        node >> FileParse::Child{{"Gases", "Gas"}, glazingSystem.gases};
        node >> FileParse::Child{"ShadeLayer", glazingSystem.shadeLayer};
        node >> FileParse::Child{"ActualHeight", glazingSystem.actualHeight};
        node >> FileParse::Child{"Orientation", glazingSystem.orientation};
        node >> FileParse::Child{"IsNominalThickness", glazingSystem.isNominalThickness};
        node >> FileParse::Child{"InteriorBC", glazingSystem.interiorBC};
        node >> FileParse::Child{"ExteriorBC", glazingSystem.exteriorBC};
        node >> FileParse::Child{"OptionFlag", glazingSystem.optionFlag};
        node >> FileParse::Child{"Source", glazingSystem.source};
        node >> FileParse::Child{"SiteLineHeight", glazingSystem.siteLineHeight};
        node >> FileParse::Child{"EOGHeight", glazingSystem.eogHeight};
        node >> FileParse::Child{"SightLineToShadeEdge", glazingSystem.sightLineToShadeEdge};
        node >> FileParse::Child{"IsDeflected", glazingSystem.isDeflected};
        node >> FileParse::Child{"EdgeGlassTag", glazingSystem.edgeGlassTag};
        node >> FileParse::Child{{"GlazingOptions", "ID"}, glazingSystem.glazingOptionIDs};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::GlazingSystem & glazingSystem)
    {
        node << FileParse::Child{"ID", glazingSystem.ID};
        node << FileParse::Child{"Index", glazingSystem.index};
        node << FileParse::Child{"Name", glazingSystem.name};
        node << FileParse::Child{{"GlazingSystemProperties", "Property"}, glazingSystem.properties};
        node << FileParse::Child{{"Layers", "Layer"}, glazingSystem.layers};
        node << FileParse::Child{{"Gases", "Gas"}, glazingSystem.gases};
        node << FileParse::Child{"ShadeLayer", glazingSystem.shadeLayer};
        node << FileParse::Child{"ActualHeight", glazingSystem.actualHeight};
        node << FileParse::Child{"Orientation", glazingSystem.orientation};
        node << FileParse::Child{"IsNominalThickness", glazingSystem.isNominalThickness};
        node << FileParse::Child{"InteriorBC", glazingSystem.interiorBC};
        node << FileParse::Child{"ExteriorBC", glazingSystem.exteriorBC};
        node << FileParse::Child{"OptionFlag", glazingSystem.optionFlag};
        node << FileParse::Child{"Source", glazingSystem.source};
        node << FileParse::Child{"SiteLineHeight", glazingSystem.siteLineHeight};
        node << FileParse::Child{"EOGHeight", glazingSystem.eogHeight};
        node << FileParse::Child{"SightLineToShadeEdge", glazingSystem.sightLineToShadeEdge};
        node << FileParse::Child{"IsDeflected", glazingSystem.isDeflected};
        node << FileParse::Child{"EdgeGlassTag", glazingSystem.edgeGlassTag};
        node << FileParse::Child{{"GlazingOptions", "ID"}, glazingSystem.glazingOptionIDs};

        return node;
    }

}   // namespace ThermFile