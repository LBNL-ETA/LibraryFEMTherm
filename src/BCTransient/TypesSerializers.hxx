#pragma once

#include <fileParse/Optional.hxx>
#include <fileParse/Enum.hxx>

#include "TypesSerializers.hxx"

#include "TypesTags.hxx"
#include "Types.hxx"

namespace BCTypesLibrary
{
    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCTypesLibrary::ConvectionModel & model)
    {
        FileParse::deserializeEnum<NodeAdapter, BCTypesLibrary::ConvectionModel>(
          node, model, BCTypesLibrary::convectionModelFromString);

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCTypesLibrary::ConvectionModel & model)
    {
        FileParse::serializeEnum<NodeAdapter, BCTypesLibrary::ConvectionModel>(
          node, model, BCTypesLibrary::convectionModelToString);

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCTypesLibrary::Direction & direction)
    {
        FileParse::deserializeEnum<NodeAdapter, BCTypesLibrary::Direction>(
          node, direction, BCTypesLibrary::directionFromString);

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCTypesLibrary::Direction & direction)
    {
        FileParse::serializeEnum<NodeAdapter, BCTypesLibrary::Direction>(
          node, direction, BCTypesLibrary::directionToString);

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCTypesLibrary::ConvectionRecord & record)
    {
        node >> FileParse::Child{"Model", record.Model};
        node >> FileParse::Child{"Temperature", record.Temperature};
        node >> FileParse::Child{"FilmCoefficient", record.FilmCoefficient};
        node >> FileParse::Child{"WindSpeed", record.WindSpeed};
        node >> FileParse::Child{"WindDirection", record.WindDirection};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCTypesLibrary::ConvectionRecord & record)
    {
        node << FileParse::Child{"Model", record.Model};
        node << FileParse::Child{"Temperature", record.Temperature};
        node << FileParse::Child{"FilmCoefficient", record.FilmCoefficient};
        node << FileParse::Child{"WindSpeed", record.WindSpeed};
        node << FileParse::Child{"WindDirection", record.WindDirection};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCTypesLibrary::RadiationModel & radiationModel)
    {
        FileParse::deserializeEnum<NodeAdapter, BCTypesLibrary::RadiationModel>(
          node, radiationModel, BCTypesLibrary::radiationModelFromString);

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCTypesLibrary::RadiationModel & radiationModel)
    {
        FileParse::serializeEnum<NodeAdapter, BCTypesLibrary::RadiationModel>(
          node, radiationModel, BCTypesLibrary::radiationModelToString);

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCTypesLibrary::RadiationRecord & record)
    {
        node >> FileParse::Child{"Model", record.Model};
        node >> FileParse::Child{"Temperature", record.Temperature};
        node >> FileParse::Child{"SurfaceEmissivity", record.SurfaceEmissivity};
        node >> FileParse::Child{"ViewFactor", record.ViewFactor};
        node >> FileParse::Child{"hr", record.FilmCoefficient};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCTypesLibrary::RadiationRecord & material)
    {
        node << FileParse::Child{"Model", material.Model};
        node << FileParse::Child{"Temperature", material.Temperature};
        node << FileParse::Child{"SurfaceEmissivity", material.SurfaceEmissivity};
        node << FileParse::Child{"ViewFactor", material.ViewFactor};
        node << FileParse::Child{"hr", material.FilmCoefficient};
        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCTypesLibrary::SolarRecord & record)
    {
        node >> FileParse::Child{"SolarRadiation", record.SolarRadiation};
        node >> FileParse::Child{"Absorptance", record.Absorptance};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCTypesLibrary::SolarRecord & record)
    {
        node << FileParse::Child{"SolarRadiation", record.SolarRadiation};
        node << FileParse::Child{"Absorptance", record.Absorptance};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCTypesLibrary::BCType & type)
    {
        FileParse::deserializeEnum<NodeAdapter, BCTypesLibrary::BCType>(
          node, type, BCTypesLibrary::bcTypeFromString);

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCTypesLibrary::BCType & type)
    {
        FileParse::serializeEnum<NodeAdapter, BCTypesLibrary::BCType>(
          node, type, BCTypesLibrary::bcTypeToString);

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCTypesLibrary::BCModel & model)
    {
        FileParse::deserializeEnum<NodeAdapter, BCTypesLibrary::BCModel>(
          node, model, BCTypesLibrary::bcModelFromString);

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCTypesLibrary::BCModel & model)
    {
        FileParse::serializeEnum<NodeAdapter, BCTypesLibrary::BCModel>(
          node, model, BCTypesLibrary::bcModelToString);

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCTypesLibrary::TypeRecord & record)
    {
        node >> FileParse::Child{"UUID", record.UUID};
        node >> FileParse::Child{"Name", record.Name};
        node >> FileParse::Child{"Protected", record.Protected};

        node >> FileParse::Child{"bcType", record.bcType};
        node >> FileParse::Child{"bcModel", record.bcModel};

        node >> FileParse::Child{"Color", record.Color};

        node >> FileParse::Child{"Convection", record.ConvectionBc};
        node >> FileParse::Child{"Radiation", record.RadiationBc};
        node >> FileParse::Child{"Solar", record.SolarBc};

        node >> FileParse::Child{"UseHeatFlux", record.UseHeatFlux};
        node >> FileParse::Child{"HeatFlux", record.HeatFlux};

        node >> FileParse::Child{"UseTemperature", record.UseTemperature};
        node >> FileParse::Child{"Temperature", record.Temperature};

        node >> FileParse::Child{"UseHumidity", record.UseHumidity};
        node >> FileParse::Child{"Humidity", record.Humidity};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCTypesLibrary::TypeRecord & record)
    {
        node << FileParse::Child{"UUID", record.UUID};
        node << FileParse::Child{"Name", record.Name};
        node << FileParse::Child{"Protected", record.Protected};

        node << FileParse::Child{"bcType", record.bcType};
        node << FileParse::Child{"bcModel", record.bcModel};
        node << FileParse::Child{"Convection", record.ConvectionBc};
        node << FileParse::Child{"Radiation", record.RadiationBc};
        node << FileParse::Child{"Solar", record.SolarBc};
        node << FileParse::Child{"UseHeatFlux", record.UseHeatFlux};
        node << FileParse::Child{"HeatFlux", record.HeatFlux};
        node << FileParse::Child{"UseTemperature", record.UseTemperature};
        node << FileParse::Child{"Temperature", record.Temperature};
        node << FileParse::Child{"UseHumidity", record.UseHumidity};
        node << FileParse::Child{"Humidity", record.Humidity};
        node << FileParse::Child{"Color", record.Color};

        return node;
    }
}   // namespace BCTypesLibrary