#pragma once

#include <vector>

#include <fileParse/Enum.hxx>
#include <fileParse/Optional.hxx>
#include <fileParse/Variant.hxx>
#include <fileParse/Vector.hxx>

#include "TimeSeriesData/Serializers.hxx"

#include "BoundaryConditions.hxx"
#include "Tags.hxx"

namespace BCLibrary
{
    template<typename NodeAdapter>
        requires TimeSeriesLibrary::SerializerNode<NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ConvectionModel & model)
    {
        FileParse::deserializeEnum<NodeAdapter, ConvectionModel>(node, model, convectionModelFromString);
        return node;
    }

    template<typename NodeAdapter>
        requires TimeSeriesLibrary::SerializerNode<NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ConvectionModel & model)
    {
        FileParse::serializeEnum<NodeAdapter, ConvectionModel>(node, model, convectionModelToString);
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, Constant & constant)
    {
        FileParse::operator>>(node, constant.value);
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const Constant & constant)
    {
        FileParse::operator<<(node, constant.value);
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, FromTimeSeries & fromTimeSeries)
    {
        node >> fromTimeSeries.role;
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const FromTimeSeries & fromTimeSeries)
    {
        node << fromTimeSeries.role;
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, Source & source)
    {
        FileParse::deserializeVariant(node, {"Constant", "FromTimeSeries"}, source);
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const Source & source)
    {
        FileParse::serializeVariant(node, {"Constant", "FromTimeSeries"}, source);
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, Convection & convection)
    {
        Tags tag;
        node >> FileParse::Child{tag.model(), convection.model};
        node >> FileParse::Child{tag.airTemperature(), convection.airTemperature};
        node >> FileParse::Child{tag.filmCoefficient(), convection.filmCoefficient};
        node >> FileParse::Child{tag.windSpeed(), convection.windSpeed};
        node >> FileParse::Child{tag.windDirection(), convection.windDirection};
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const Convection & convection)
    {
        Tags tag;
        node << FileParse::Child{tag.model(), convection.model};
        node << FileParse::Child{tag.airTemperature(), convection.airTemperature};
        node << FileParse::Child{tag.filmCoefficient(), convection.filmCoefficient};
        node << FileParse::Child{tag.windSpeed(), convection.windSpeed};
        node << FileParse::Child{tag.windDirection(), convection.windDirection};
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, AutomaticEnclosure & enclosure)
    {
        Tags tag;
        node >> FileParse::Child{tag.emissivity(), enclosure.emissivity};
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const AutomaticEnclosure & enclosure)
    {
        Tags tag;
        node << FileParse::Child{tag.emissivity(), enclosure.emissivity};
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
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BlackBodyRadiation & blackBody)
    {
        Tags tag;
        node >> FileParse::Child{tag.temperature(), blackBody.temperature};
        node >> FileParse::Child{tag.emissivity(), blackBody.emissivity};
        node >> FileParse::Child{tag.viewFactor(), blackBody.viewFactor};
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BlackBodyRadiation & blackBody)
    {
        Tags tag;
        node << FileParse::Child{tag.temperature(), blackBody.temperature};
        node << FileParse::Child{tag.emissivity(), blackBody.emissivity};
        node << FileParse::Child{tag.viewFactor(), blackBody.viewFactor};
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, FixedCoefficientRadiation & fixed)
    {
        Tags tag;
        node >> FileParse::Child{tag.temperature(), fixed.temperature};
        node >> FileParse::Child{tag.coefficient(), fixed.coefficient};
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const FixedCoefficientRadiation & fixed)
    {
        Tags tag;
        node << FileParse::Child{tag.temperature(), fixed.temperature};
        node << FileParse::Child{tag.coefficient(), fixed.coefficient};
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, Solar & solar)
    {
        Tags tag;
        node >> FileParse::Child{tag.irradiance(), solar.irradiance};
        node >> FileParse::Child{tag.absorptance(), solar.absorptance};
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const Solar & solar)
    {
        Tags tag;
        node << FileParse::Child{tag.irradiance(), solar.irradiance};
        node << FileParse::Child{tag.absorptance(), solar.absorptance};
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, SurfaceExchange & exchange)
    {
        Tags tag;
        node >> FileParse::Child{tag.relativeHumidity(), exchange.relativeHumidity};
        node >> FileParse::Child{tag.convection(), exchange.convection};
        FileParse::deserializeOptionalVariant(
          node, {"AutomaticEnclosure", "ManualEnclosure", "BlackBody", "FixedCoefficient"}, exchange.radiation);
        node >> FileParse::Child{tag.solar(), exchange.solar};
        node >> FileParse::Child{tag.flux(), exchange.flux};
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const SurfaceExchange & exchange)
    {
        Tags tag;
        node << FileParse::Child{tag.relativeHumidity(), exchange.relativeHumidity};
        node << FileParse::Child{tag.convection(), exchange.convection};
        FileParse::serializeOptionalVariant(
          node, {"AutomaticEnclosure", "ManualEnclosure", "BlackBody", "FixedCoefficient"}, exchange.radiation);
        node << FileParse::Child{tag.solar(), exchange.solar};
        node << FileParse::Child{tag.flux(), exchange.flux};
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, PrescribedState & prescribed)
    {
        Tags tag;
        node >> FileParse::Child{tag.temperature(), prescribed.temperature};
        node >> FileParse::Child{tag.relativeHumidity(), prescribed.relativeHumidity};
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const PrescribedState & prescribed)
    {
        Tags tag;
        node << FileParse::Child{tag.temperature(), prescribed.temperature};
        node << FileParse::Child{tag.relativeHumidity(), prescribed.relativeHumidity};
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, RadiationSurface & radiationSurface)
    {
        Tags tag;
        node >> FileParse::Child{tag.isDefault(), radiationSurface.isDefault};
        node >> FileParse::Child{tag.temperature(), radiationSurface.temperature};
        node >> FileParse::Child{tag.emissivity(), radiationSurface.emissivity};
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
    const NodeAdapter & operator>>(const NodeAdapter & node, NoExchange &)
    {
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const NoExchange &)
    {
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
          node, {"SurfaceExchange", "PrescribedState", "RadiationSurface", "NoExchange"}, boundaryCondition.data);
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
        FileParse::serializeVariant(
          node, {"SurfaceExchange", "PrescribedState", "RadiationSurface", "NoExchange"}, boundaryCondition.data);
        return node;
    }
}   // namespace BCLibrary
