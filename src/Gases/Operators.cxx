#include "Operators.hxx"

#include <algorithm>
#include <string>

#include "Gases.hxx"
#include "LibraryUtilities/Common.hxx"
#include "LibraryUtilities/PropertyDifference.hxx"

namespace GasesLibrary
{
    using LibraryCommon::PropertyDifference;

    template<typename Visitor> void visitFields(const Coefficients & lhs, const Coefficients & rhs, Visitor & vis);
    template<typename Visitor> void visitFields(const GasProperties & lhs, const GasProperties & rhs, Visitor & vis);

    // ---- The single physical-field list per struct ----
    template<typename Visitor>
    void visitFields(const Coefficients & lhs, const Coefficients & rhs, Visitor & vis)
    {
        vis("A", lhs.A, rhs.A, 1e-6);
        vis("B", lhs.B, rhs.B, 1e-9);
        vis("C", lhs.C, rhs.C, 1e-6);
    }

    template<typename Visitor>
    void visitFields(const GasProperties & lhs, const GasProperties & rhs, Visitor & vis)
    {
        vis("Molecular weight", lhs.MolecularWeight, rhs.MolecularWeight);
        vis("Specific heat ratio", lhs.SpecificHeatRatio, rhs.SpecificHeatRatio);
        vis.nested("Conductivity", lhs.Conductivity, rhs.Conductivity);
        vis.nested("Viscosity", lhs.Viscosity, rhs.Viscosity);
        vis.nested("Specific heat", lhs.SpecificHeat, rhs.SpecificHeat);
    }

    template<typename RecordType>
    bool equalByFields(const RecordType & lhs, const RecordType & rhs)
    {
        LibraryCommon::EqualityCollector collector;
        visitFields(lhs, rhs, collector);
        return collector.equal;
    }

    bool operator==(const Coefficients & lhs, const Coefficients & rhs)
    {
        return equalByFields(lhs, rhs);
    }

    bool operator==(const GasProperties & lhs, const GasProperties & rhs)
    {
        return equalByFields(lhs, rhs);
    }

    bool operator==(const GasCommon & lhs, const GasCommon & rhs)
    {
        return lhs.UUID == rhs.UUID && lhs.Name == rhs.Name && lhs.Protected == rhs.Protected;
    }

    bool operator==(const PureGas & lhs, const PureGas & rhs)
    {
        return static_cast<const GasCommon &>(lhs) == static_cast<const GasCommon &>(rhs)
               && lhs.Properties == rhs.Properties;
    }

    bool operator==(const Component & lhs, const Component & rhs)
    {
        return lhs.PureGasName == rhs.PureGasName && LibraryCommon::isEqual(lhs.Fraction, rhs.Fraction, 1e-6);
    }

    bool operator==(const Gas & lhs, const Gas & rhs)
    {
        return static_cast<const GasCommon &>(lhs) == static_cast<const GasCommon &>(rhs)
               && lhs.Components == rhs.Components;
    }

    bool operator==(const GasesData & lhs, const GasesData & rhs)
    {
        return lhs.gas == rhs.gas && lhs.components == rhs.components;
    }

    bool arePhysicalPropertiesEqual(const PureGas & lhs, const PureGas & rhs)
    {
        return lhs.Properties == rhs.Properties;
    }

    bool arePhysicalPropertiesEqual(const std::optional<PureGas> & lhs, const std::optional<PureGas> & rhs)
    {
        bool result{true};
        if(lhs.has_value() && rhs.has_value())
        {
            result = arePhysicalPropertiesEqual(lhs.value(), rhs.value());
        }
        else if(lhs.has_value() || rhs.has_value())
        {
            result = false;
        }
        return result;
    }

    bool arePhysicalPropertiesEqual(const Gas & lhs, const Gas & rhs)
    {
        return lhs.Components == rhs.Components;
    }

    bool arePhysicalPropertiesEqual(const GasesData & lhs, const GasesData & rhs)
    {
        bool result{true};
        if(lhs.components.size() == rhs.components.size())
        {
            for(size_t i = 0; i < lhs.components.size(); ++i)
            {
                if(!arePhysicalPropertiesEqual(lhs.components[i], rhs.components[i]))
                {
                    result = false;
                    break;
                }
            }
        }
        else
        {
            result = false;
        }

        if(result)
        {
            result = arePhysicalPropertiesEqual(lhs.gas, rhs.gas);
        }

        return result;
    }

    std::vector<PropertyDifference> physicalDifferences(const GasesData & lhs, const GasesData & rhs)
    {
        std::vector<PropertyDifference> diffs;
        const std::size_t count{std::min(lhs.components.size(), rhs.components.size())};
        for(std::size_t idx = 0; idx < count; ++idx)
        {
            const std::string component{idx < rhs.gas.Components.size() ? rhs.gas.Components[idx].PureGasName
                                                                        : "Component " + std::to_string(idx + 1)};
            if(idx < lhs.gas.Components.size() && idx < rhs.gas.Components.size())
            {
                LibraryCommon::DifferenceCollector fraction;
                fraction.prefix = component;
                fraction("Fraction", lhs.gas.Components[idx].Fraction, rhs.gas.Components[idx].Fraction, 1e-6);
                diffs.insert(diffs.end(), fraction.diffs.begin(), fraction.diffs.end());
            }
            const auto & libComponent = lhs.components[idx];
            const auto & fileComponent = rhs.components[idx];
            if(libComponent.has_value() && fileComponent.has_value())
            {
                LibraryCommon::DifferenceCollector collector;
                collector.prefix = component;
                visitFields(libComponent->Properties, fileComponent->Properties, collector);
                diffs.insert(diffs.end(), collector.diffs.begin(), collector.diffs.end());
            }
            else if(libComponent.has_value() != fileComponent.has_value())
            {
                LibraryCommon::DifferenceCollector collector;
                collector.prefix = component;
                collector.absentSide = libComponent.has_value() ? 1 : -1;
                const auto & present = libComponent.has_value() ? libComponent.value() : fileComponent.value();
                visitFields(present.Properties, present.Properties, collector);
                diffs.insert(diffs.end(), collector.diffs.begin(), collector.diffs.end());
            }
        }
        if(lhs.components.size() != rhs.components.size())
        {
            diffs.push_back({"Component count",
                             static_cast<double>(lhs.components.size()),
                             static_cast<double>(rhs.components.size())});
        }
        if(diffs.empty() && !arePhysicalPropertiesEqual(lhs, rhs))
        {
            diffs.push_back({"Physical properties", std::string{"differs"}, std::string{"differs"}});
        }
        return diffs;
    }
}   // namespace GasesLibrary
