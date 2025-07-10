#pragma once

#include <vector>
#include <optional>

#include <fileParse/Optional.hxx>
#include <fileParse/Vector.hxx>
#include <fileParse/Variant.hxx>
#include <fileParse/Enum.hxx>

#include "Materials.hxx"
#include "Tags.hxx"

namespace MaterialsLibrary
{
    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const MaterialsLibrary::point & point)
    {
        node << FileParse::Child{"x", point.x} << FileParse::Child{"y", point.y};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, MaterialsLibrary::point & point)
    {
        node >> FileParse::Child{"x", point.x} >> FileParse::Child{"y", point.y};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, MaterialsLibrary::MaterialRoughness & roughness)
    {
        FileParse::deserializeEnum<NodeAdapter, MaterialsLibrary::MaterialRoughness>(
          node, roughness, MaterialsLibrary::materialRoughnessFromString);

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const MaterialsLibrary::MaterialRoughness & roughness)
    {
        FileParse::serializeEnum<NodeAdapter, MaterialsLibrary::MaterialRoughness>(
          node, roughness, MaterialsLibrary::materialRoughnessToString);

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, MaterialsLibrary::HygroThermal & material)
    {
        node >> FileParse::Child{"DefaultThickness", material.DefaultThickness};
        node >> FileParse::Child{"MaterialInformation", material.MaterialInformation};
        node >> FileParse::Child{"BulkDensity", material.BulkDensity};
        node >> FileParse::Child{"Porosity", material.Porosity};
        node >> FileParse::Child{"SpecificHeatCapacityDry", material.SpecificHeatCapacityDry};
        node >> FileParse::Child{"ThermalConductivityDry", material.ThermalConductivityDry};

        node >> FileParse::Child{"Roughness", material.Roughness};

        node >> FileParse::Child{"WaterVaporDiffusionResistanceFactor", material.WaterVaporDiffusionResistanceFactor};
        node >> FileParse::Child{{"MoistureStorageFunction", "TableValue"}, material.MoistureStorageFunction};
        node >> FileParse::Child{{"LiquidTransportationCoefficientSuction", "TableValue"},
                             material.LiquidTransportationCoefficientSuction};
        node >> FileParse::Child{{"LiquidTransportationCoefficientRedistribution", "TableValue"},
                             material.LiquidTransportationCoefficientRedistribution};
        node >> FileParse::Child{{"ThermalConductivityMoistureDependent", "TableValue"},
                             material.ThermalConductivityMoistureDependent};
        node >> FileParse::Child{{"ThermalConductivityTemperatureDependent", "TableValue"},
                             material.ThermalConductivityTemperatureDependent};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const MaterialsLibrary::HygroThermal & hygroThermal)
    {
        node << FileParse::Child{"DefaultThickness", hygroThermal.DefaultThickness};
        node << FileParse::Child{"MaterialInformation", hygroThermal.MaterialInformation};
        node << FileParse::Child{"BulkDensity", hygroThermal.BulkDensity};
        node << FileParse::Child{"Porosity", hygroThermal.Porosity};
        node << FileParse::Child{"SpecificHeatCapacityDry", hygroThermal.SpecificHeatCapacityDry};
        node << FileParse::Child{"ThermalConductivityDry", hygroThermal.ThermalConductivityDry};

        node << FileParse::Child{"Roughness", hygroThermal.Roughness};

        node << FileParse::Child{"WaterVaporDiffusionResistanceFactor",
                             hygroThermal.WaterVaporDiffusionResistanceFactor};

        node << FileParse::Child{{"MoistureStorageFunction", "TableValue"}, hygroThermal.MoistureStorageFunction};
        node << FileParse::Child{{"LiquidTransportationCoefficientSuction", "TableValue"},
                             hygroThermal.LiquidTransportationCoefficientSuction};
        node << FileParse::Child{{"LiquidTransportationCoefficientRedistribution", "TableValue"},
                             hygroThermal.LiquidTransportationCoefficientRedistribution};
        node << FileParse::Child{{"ThermalConductivityMoistureDependent", "TableValue"},
                             hygroThermal.ThermalConductivityMoistureDependent};
        node << FileParse::Child{{"ThermalConductivityTemperatureDependent", "TableValue"},
                             hygroThermal.ThermalConductivityTemperatureDependent};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, MaterialsLibrary::OpticalProperties & opticalProperties)
    {
        node >> FileParse::Child{"Transmittance", opticalProperties.Transmittance};
        node >> FileParse::Child{"Reflectance", opticalProperties.Reflectance};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const MaterialsLibrary::OpticalProperties & opticalProperties)
    {
        node << FileParse::Child{"Transmittance", opticalProperties.Transmittance};
        node << FileParse::Child{"Reflectance", opticalProperties.Reflectance};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, MaterialsLibrary::OpticalSide & opticalSide)
    {
        node >> FileParse::Child{MaterialsLibrary::frontSideString(), opticalSide.Front};
        node >> FileParse::Child{MaterialsLibrary::backSideString(), opticalSide.Back};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const MaterialsLibrary::OpticalSide & opticalSide)
    {
        node << FileParse::Child{MaterialsLibrary::frontSideString(), opticalSide.Front};
        node << FileParse::Child{MaterialsLibrary::backSideString(), opticalSide.Back};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, MaterialsLibrary::OpticalType & opticalRange)
    {
        node >> FileParse::Child{MaterialsLibrary::directString(), opticalRange.Direct};
        node >> FileParse::Child{MaterialsLibrary::diffuseString(), opticalRange.Diffuse};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const MaterialsLibrary::OpticalType & opticalRange)
    {
        node << FileParse::Child{MaterialsLibrary::directString(), opticalRange.Direct};
        node << FileParse::Child{MaterialsLibrary::diffuseString(), opticalRange.Diffuse};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, MaterialsLibrary::Infrared & infrared)
    {
        node >> FileParse::Child{"Transmittance", infrared.Transmittance};
        node >> FileParse::Child{"Emissivity-Front", infrared.EmissivityFront};
        node >> FileParse::Child{"Emissivity-Back", infrared.EmissivityBack};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const MaterialsLibrary::Infrared & infrared)
    {
        node << FileParse::Child{"Transmittance", infrared.Transmittance};
        node << FileParse::Child{"Emissivity-Front", infrared.EmissivityFront};
        node << FileParse::Child{"Emissivity-Back", infrared.EmissivityBack};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, MaterialsLibrary::Integrated & integrated)
    {
        node >> FileParse::Child{"Infrared", integrated.IR};
        node >> FileParse::Child{"Solar", integrated.Solar};
        node >> FileParse::Child{"Visible", integrated.Visible};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const MaterialsLibrary::Integrated & integrated)
    {
        node << FileParse::Child{"Infrared", integrated.IR};
        node << FileParse::Child{"Solar", integrated.Solar};
        node << FileParse::Child{"Visible", integrated.Visible};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, MaterialsLibrary::Optical & optical)
    {
        node >> FileParse::Child{"Integrated", optical.integrated};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const MaterialsLibrary::Optical & optical)
    {
        node << FileParse::Child{"Integrated", optical.integrated};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, MaterialsLibrary::Solid & solid)
    {
        node >> FileParse::Child{"HygroThermal", solid.hygroThermal};
        node >> FileParse::Child{"Optical", solid.optical};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const MaterialsLibrary::Solid & solid)
    {
        node << FileParse::Child{"HygroThermal", solid.hygroThermal};
        node << FileParse::Child{"Optical", solid.optical};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, MaterialsLibrary::CavityStandard & standard)
    {
        FileParse::deserializeEnum<NodeAdapter, MaterialsLibrary::CavityStandard>(
          node, standard, MaterialsLibrary::cavityStandardFromString);

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const MaterialsLibrary::CavityStandard & standard)
    {
        FileParse::serializeEnum<NodeAdapter, MaterialsLibrary::CavityStandard>(
          node, standard, MaterialsLibrary::cavityStandardToString);

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, MaterialsLibrary::Cavity & cavity)
    {
        node >> FileParse::Child{"CavityStandard", cavity.cavityStandard};
        node >> FileParse::Child{"Gas", cavity.GasName};
        node >> FileParse::Child{"EmissivitySide1", cavity.EmissivitySide1};
        node >> FileParse::Child{"EmissivitySide2", cavity.EmissivitySide2};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const MaterialsLibrary::Cavity & cavity)
    {
        node << FileParse::Child{"CavityStandard", cavity.cavityStandard};
        node << FileParse::Child{"Gas", cavity.GasName};
        node << FileParse::Child{"EmissivitySide1", cavity.EmissivitySide1};
        node << FileParse::Child{"EmissivitySide2", cavity.EmissivitySide2};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, MaterialsLibrary::RadiationEnclosure & enclosure)
    {
        node >> FileParse::Child{"EmissivityDefault", enclosure.emissivityDefault};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const MaterialsLibrary::RadiationEnclosure & enclosure)
    {
        node << FileParse::Child{"EmissivityDefault", enclosure.emissivityDefault};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, MaterialsLibrary::Material & material)
    {
        using FileParse::operator>>;

        node >> FileParse::Child{"UUID", material.UUID};
        node >> FileParse::Child{"Name", material.Name};
        node >> FileParse::Child{"ProjectName", material.ProjectName};
        node >> FileParse::Child{"Protected", material.Protected};
        node >> FileParse::Child{"Color", material.Color};

        FileParse::deserializeVariant(node, {"Solid", "Cavity", "RadiationEnclosure"}, material.data);

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const MaterialsLibrary::Material & material)
    {
        using FileParse::operator<<;

        node << FileParse::Child{"UUID", material.UUID};
        node << FileParse::Child{"Name", material.Name};
        node << FileParse::Child{"ProjectName", material.ProjectName};
        node << FileParse::Child{"Protected", material.Protected};
        node << FileParse::Child{"Color", material.Color};

        FileParse::serializeVariant(node, {"Solid", "Cavity", "RadiationEnclosure"}, material.data);

        return node;
    }
}   // namespace MaterialsLibrary