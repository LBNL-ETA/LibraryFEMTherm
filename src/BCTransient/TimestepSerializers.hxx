#pragma once

#include <fileParse/Vector.hxx>

#include "TimestepSerializers.hxx"
#include "Timestep.hxx"

namespace BCInputFileLibrary
{
    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCInputFileLibrary::TARPTimestep & record)
    {
        node >> FileParse::Child{"Index", record.id};
        node >> FileParse::Child{"Temperature", record.temperature};
        node >> FileParse::Child{"Humidity", record.humidity};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCInputFileLibrary::TARPTimestep & record)
    {
        node << FileParse::Child{"Index", record.id};
        node << FileParse::Child{"Temperature", record.temperature};
        node << FileParse::Child{"Humidity", record.humidity};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCInputFileLibrary::ASHRAEInside & record)
    {
        node >> FileParse::Child{"Index", record.id};
        node >> FileParse::Child{"Temperature", record.temperature};
        node >> FileParse::Child{"Humidity", record.humidity};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCInputFileLibrary::ASHRAEInside & record)
    {
        node << FileParse::Child{"Index", record.id};
        node << FileParse::Child{"Temperature", record.temperature};
        node << FileParse::Child{"Humidity", record.humidity};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCInputFileLibrary::ASHRAEOutside & record)
    {
        node >> FileParse::Child{"Index", record.id};
        node >> FileParse::Child{"Temperature", record.temperature};
        node >> FileParse::Child{"Humidity", record.humidity};
        node >> FileParse::Child{"WindSpeed", record.windSpeed};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCInputFileLibrary::ASHRAEOutside & record)
    {
        node << FileParse::Child{"Index", record.id};
        node << FileParse::Child{"Temperature", record.temperature};
        node << FileParse::Child{"Humidity", record.humidity};
        node << FileParse::Child{"WindSpeed", record.windSpeed};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCInputFileLibrary::FixedFilmCoefficient & record)
    {
        node >> FileParse::Child{"Index", record.id};
        node >> FileParse::Child{"FilmCoefficient", record.fixedFilmCoefficient};
        node >> FileParse::Child{"Temperature", record.temperature};
        node >> FileParse::Child{"Humidity", record.humidity};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCInputFileLibrary::FixedFilmCoefficient & record)
    {
        node << FileParse::Child{"Index", record.id};
        node << FileParse::Child{"FilmCoefficient", record.fixedFilmCoefficient};
        node << FileParse::Child{"Temperature", record.temperature};
        node << FileParse::Child{"Humidity", record.humidity};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCInputFileLibrary::YazdanianKlems & record)
    {
        node >> FileParse::Child{"Index", record.id};
        node >> FileParse::Child{"Temperature", record.temperature};
        node >> FileParse::Child{"Humidity", record.humidity};
        node >> FileParse::Child{"WindSpeed", record.windSpeed};
        node >> FileParse::Child{"WindDirection", record.windDirection};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCInputFileLibrary::YazdanianKlems & record)
    {
        node << FileParse::Child{"Index", record.id};
        node << FileParse::Child{"Temperature", record.temperature};
        node << FileParse::Child{"Humidity", record.humidity};
        node << FileParse::Child{"WindSpeed", record.windSpeed};
        node << FileParse::Child{"WindDirection", record.windDirection};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCInputFileLibrary::Kimura & record)
    {
        node >> FileParse::Child{"Index", record.id};
        node >> FileParse::Child{"Temperature", record.temperature};
        node >> FileParse::Child{"Humidity", record.humidity};
        node >> FileParse::Child{"WindSpeed", record.windSpeed};
        node >> FileParse::Child{"WindDirection", record.windDirection};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCInputFileLibrary::Kimura & record)
    {
        node << FileParse::Child{"Index", record.id};
        node << FileParse::Child{"Temperature", record.temperature};
        node << FileParse::Child{"Humidity", record.humidity};
        node << FileParse::Child{"WindSpeed", record.windSpeed};
        node << FileParse::Child{"WindDirection", record.windDirection};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCInputFileLibrary::Montazeri & record)
    {
        node >> FileParse::Child{"Index", record.id};
        node >> FileParse::Child{"Temperature", record.temperature};
        node >> FileParse::Child{"Humidity", record.humidity};
        node >> FileParse::Child{"WindSpeed", record.windSpeed};
        node >> FileParse::Child{"WindDirection", record.windDirection};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCInputFileLibrary::Montazeri & record)
    {
        node << FileParse::Child{"Index", record.id};
        node << FileParse::Child{"Temperature", record.temperature};
        node << FileParse::Child{"Humidity", record.humidity};
        node << FileParse::Child{"WindSpeed", record.windSpeed};
        node << FileParse::Child{"WindDirection", record.windDirection};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCInputFileLibrary::ConvectionTimesteps & record)
    {
        node >> FileParse::Child{{"ConvectionTimesteps", "TARPTimestep"}, record.tarp};
        node >> FileParse::Child{{"ConvectionTimesteps", "ASHRAETimestepInside"}, record.ashraeInside};
        node >> FileParse::Child{{"ConvectionTimesteps", "ASHRAETimestepOutside"}, record.ashraeOutside};
        node >> FileParse::Child{{"ConvectionTimesteps", "FixedConvectionFilmTimestep"}, record.fixedFilmCoefficient};
        node >> FileParse::Child{{"ConvectionTimesteps", "YazdanianKlemsTimestep"}, record.yazdanianKlems};
        node >> FileParse::Child{{"ConvectionTimesteps", "KimuraTimestep"}, record.kimura};
        node >> FileParse::Child{{"ConvectionTimesteps", "MontazeriTimestep"}, record.montazeri};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCInputFileLibrary::ConvectionTimesteps & record)
    {
        if(!record.tarp.empty() || !record.ashraeInside.empty() || !record.ashraeOutside.empty()
           || !record.fixedFilmCoefficient.empty() || !record.yazdanianKlems.empty() || !record.kimura.empty()
           || !record.montazeri.empty())
        {
            auto childNode{node.addChild("ConvectionTimesteps")};
            childNode << FileParse::Child{"TARPTimestep", record.tarp};
            childNode << FileParse::Child{"ASHRAETimestepInside", record.ashraeInside};
            childNode << FileParse::Child{"ASHRAETimestepOutside", record.ashraeOutside};
            childNode << FileParse::Child{"FixedConvectionFilmTimestep", record.fixedFilmCoefficient};
            childNode << FileParse::Child{"YazdanianKlemsTimestep", record.yazdanianKlems};
            childNode << FileParse::Child{"KimuraTimestep", record.kimura};
            childNode << FileParse::Child{"MontazeriTimestep", record.montazeri};
        }

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCInputFileLibrary::FixedRadiation & record)
    {
        node >> FileParse::Child{"Index", record.id};
        node >> FileParse::Child{"Temperature", record.temperature};
        node >> FileParse::Child{"hr", record.hr};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCInputFileLibrary::FixedRadiation & record)
    {
        node << FileParse::Child{"Index", record.id};
        node << FileParse::Child{"Temperature", record.temperature};
        node << FileParse::Child{"hr", record.hr};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCInputFileLibrary::BlackBodyRadiation & record)
    {
        node >> FileParse::Child{"Index", record.id};
        node >> FileParse::Child{"Temperature", record.temperature};
        node >> FileParse::Child{"Emissivity", record.emissivity};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCInputFileLibrary::BlackBodyRadiation & record)
    {
        node << FileParse::Child{"Index", record.id};
        node << FileParse::Child{"Temperature", record.temperature};
        node << FileParse::Child{"Emissivity", record.emissivity};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCInputFileLibrary::RadiationTimesteps & record)
    {
        node >> FileParse::Child{{"RadiationTimesteps", "FixedRadiationCoefficientTimestep"}, record.fixedRadiation};
        node >> FileParse::Child{{"RadiationTimesteps", "BlackBodyRadiationTimestep"}, record.blackBodyRadiation};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCInputFileLibrary::RadiationTimesteps & record)
    {
        if(!record.fixedRadiation.empty() || !record.blackBodyRadiation.empty())
        {
            auto childNode{node.addChild("RadiationTimesteps")};
            childNode << FileParse::Child{"FixedRadiationTimestep", record.fixedRadiation};
            childNode << FileParse::Child{"BlackBodyRadiationTimestep", record.blackBodyRadiation};
        }

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCInputFileLibrary::HeatFlux & record)
    {
        node >> FileParse::Child{"Index", record.id};
        node >> FileParse::Child{"HeatFlux", record.heatFlux};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCInputFileLibrary::HeatFlux & record)
    {
        node << FileParse::Child{"Index", record.id};
        node << FileParse::Child{"HeatFlux", record.heatFlux};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCInputFileLibrary::SolarRadiation & record)
    {
        node >> FileParse::Child{"Index", record.id};
        node >> FileParse::Child{"SolarRadiation", record.solarRadiation};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCInputFileLibrary::SolarRadiation & record)
    {
        node << FileParse::Child{"Index", record.id};
        node << FileParse::Child{"SolarRadiation", record.solarRadiation};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCInputFileLibrary::FixedTemperature & record)
    {
        node >> FileParse::Child{"Index", record.id};
        node >> FileParse::Child{"Temperature", record.temperature};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCInputFileLibrary::FixedTemperature & record)
    {
        node << FileParse::Child{"Index", record.id};
        node << FileParse::Child{"Temperature", record.temperature};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, BCInputFileLibrary::FixedHumidity & record)
    {
        node >> FileParse::Child{"Index", record.id};
        node >> FileParse::Child{"Humidity", record.humidity};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const BCInputFileLibrary::FixedHumidity & record)
    {
        node << FileParse::Child{"Index", record.id};
        node << FileParse::Child{"Humidity", record.humidity};

        return node;
    }
}   // namespace BCInputFileLibrary