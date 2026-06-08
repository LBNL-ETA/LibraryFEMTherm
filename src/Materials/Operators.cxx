#include "Operators.hxx"

#include "Materials.hxx"
#include "Tags.hxx"
#include "LibraryUtilities/Common.hxx"
#include "LibraryUtilities/PropertyDifference.hxx"

namespace MaterialsLibrary
{
    using LibraryCommon::PropertyDifference;
    using LibraryCommon::PropertyValue;

    // Forward declarations so ADL (the collectors) and mutually-recursive visitFields resolve
    // regardless of definition order.
    PropertyValue toValue(const std::optional<MaterialRoughness> & value);
    PropertyValue toValue(const std::optional<std::vector<point>> & value);

    template<typename Visitor> void visitFields(const HygroThermal & lhs, const HygroThermal & rhs, Visitor & vis);
    template<typename Visitor> void visitFields(const Infrared & lhs, const Infrared & rhs, Visitor & vis);
    template<typename Visitor> void visitFields(const OpticalProperties & lhs, const OpticalProperties & rhs, Visitor & vis);
    template<typename Visitor> void visitFields(const OpticalSide & lhs, const OpticalSide & rhs, Visitor & vis);
    template<typename Visitor> void visitFields(const OpticalType & lhs, const OpticalType & rhs, Visitor & vis);
    template<typename Visitor> void visitFields(const Integrated & lhs, const Integrated & rhs, Visitor & vis);
    template<typename Visitor> void visitFields(const Optical & lhs, const Optical & rhs, Visitor & vis);
    template<typename Visitor> void visitFields(const Solid & lhs, const Solid & rhs, Visitor & vis);

    // ---- Module-specific toValue overloads (raw labels for non-numeric leaves) ----
    PropertyValue toValue(const std::optional<MaterialRoughness> & value)
    {
        if(value.has_value())
        {
            return materialRoughnessToString(value.value());
        }
        return std::monostate{};
    }

    PropertyValue toValue(const std::optional<std::vector<point>> & value)
    {
        if(value.has_value())
        {
            return std::string{"table (" + std::to_string(value->size()) + " pts)"};
        }
        return std::monostate{};
    }

    // ---- The single physical-field list per struct ----
    template<typename Visitor>
    void visitFields(const HygroThermal & lhs, const HygroThermal & rhs, Visitor & vis)
    {
        vis("Default thickness", lhs.DefaultThickness, rhs.DefaultThickness);
        vis("Material information", lhs.MaterialInformation, rhs.MaterialInformation);
        vis("Bulk density", lhs.BulkDensity, rhs.BulkDensity);
        vis("Porosity", lhs.Porosity, rhs.Porosity);
        vis("Specific heat (dry)", lhs.SpecificHeatCapacityDry, rhs.SpecificHeatCapacityDry);
        vis("Thermal conductivity (dry)", lhs.ThermalConductivityDry, rhs.ThermalConductivityDry);
        vis("Roughness", lhs.Roughness, rhs.Roughness);
        vis("Vapor resistance factor", lhs.WaterVaporDiffusionResistanceFactor, rhs.WaterVaporDiffusionResistanceFactor);
        vis("Moisture storage function", lhs.MoistureStorageFunction, rhs.MoistureStorageFunction);
        vis("Liquid transport (suction)", lhs.LiquidTransportationCoefficientSuction, rhs.LiquidTransportationCoefficientSuction);
        vis("Liquid transport (redistribution)",
            lhs.LiquidTransportationCoefficientRedistribution,
            rhs.LiquidTransportationCoefficientRedistribution);
        vis("Conductivity (moisture-dependent)", lhs.ThermalConductivityMoistureDependent, rhs.ThermalConductivityMoistureDependent);
        vis("Conductivity (temperature-dependent)",
            lhs.ThermalConductivityTemperatureDependent,
            rhs.ThermalConductivityTemperatureDependent);
    }

    template<typename Visitor>
    void visitFields(const Infrared & lhs, const Infrared & rhs, Visitor & vis)
    {
        vis("transmittance", lhs.Transmittance, rhs.Transmittance);
        vis("emissivity (front)", lhs.EmissivityFront, rhs.EmissivityFront);
        vis("emissivity (back)", lhs.EmissivityBack, rhs.EmissivityBack);
    }

    // Optical leaves keep exact comparison (tolerance 0), matching the previous optical ==.
    template<typename Visitor>
    void visitFields(const OpticalProperties & lhs, const OpticalProperties & rhs, Visitor & vis)
    {
        vis("Transmittance", lhs.Transmittance, rhs.Transmittance, 0.0);
        vis("Reflectance", lhs.Reflectance, rhs.Reflectance, 0.0);
    }

    template<typename Visitor>
    void visitFields(const OpticalSide & lhs, const OpticalSide & rhs, Visitor & vis)
    {
        vis.nested("Front", lhs.Front, rhs.Front);
        vis.nested("Back", lhs.Back, rhs.Back);
    }

    template<typename Visitor>
    void visitFields(const OpticalType & lhs, const OpticalType & rhs, Visitor & vis)
    {
        vis.nested("Direct", lhs.Direct, rhs.Direct);
        vis.nested("Diffuse", lhs.Diffuse, rhs.Diffuse);
    }

    template<typename Visitor>
    void visitFields(const Integrated & lhs, const Integrated & rhs, Visitor & vis)
    {
        vis.nested("IR", lhs.IR, rhs.IR);
        vis.optional("Solar", lhs.Solar, rhs.Solar);
        vis.optional("Visible", lhs.Visible, rhs.Visible);
    }

    template<typename Visitor>
    void visitFields(const Optical & lhs, const Optical & rhs, Visitor & vis)
    {
        vis.optional("", lhs.integrated, rhs.integrated);
    }

    template<typename Visitor>
    void visitFields(const Solid & lhs, const Solid & rhs, Visitor & vis)
    {
        vis.optional("", lhs.hygroThermal, rhs.hygroThermal);
        vis.optional("", lhs.optical, rhs.optical);
    }

    template<typename RecordType>
    bool equalByFields(const RecordType & lhs, const RecordType & rhs)
    {
        LibraryCommon::EqualityCollector collector;
        visitFields(lhs, rhs, collector);
        return collector.equal;
    }

    bool operator==(const point & lhs, const point & rhs)
    {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }

    bool operator==(const HygroThermal & lhs, const HygroThermal & rhs)
    {
        return equalByFields(lhs, rhs);
    }

    bool operator==(const OpticalProperties & lhs, const OpticalProperties & rhs)
    {
        return equalByFields(lhs, rhs);
    }

    bool operator==(const OpticalSide & lhs, const OpticalSide & rhs)
    {
        return equalByFields(lhs, rhs);
    }

    bool operator==(const OpticalType & lhs, const OpticalType & rhs)
    {
        return equalByFields(lhs, rhs);
    }

    bool operator==(const Integrated & lhs, const Integrated & rhs)
    {
        return equalByFields(lhs, rhs);
    }

    bool operator==(const Infrared & lhs, const Infrared & rhs)
    {
        return equalByFields(lhs, rhs);
    }

    bool operator==(const Optical & lhs, const Optical & rhs)
    {
        return equalByFields(lhs, rhs);
    }

    bool operator==(const Solid & lhs, const Solid & rhs)
    {
        return equalByFields(lhs, rhs);
    }

    bool operator==(const Material & lhs, const Material & rhs)
    {
        return lhs.UUID == rhs.UUID && lhs.Name == rhs.Name && lhs.Protected == rhs.Protected && lhs.Color == rhs.Color
               && lhs.data == rhs.data;
    }

    bool arePhysicalPropertiesEqual(const Material & lhs, const Material & rhs)
    {
        return lhs.data == rhs.data;
    }

    std::vector<PropertyDifference> physicalDifferences(const Material & lhs, const Material & rhs)
    {
        LibraryCommon::DifferenceCollector collector;
        visitFields(lhs.data, rhs.data, collector);
        return std::move(collector.diffs);
    }

}   // namespace MaterialsLibrary
