#include "Operators.hxx"

#include <variant>

#include "SteadyState.hxx"
#include "Tags.hxx"
#include "LibraryUtilities/Common.hxx"
#include "LibraryUtilities/PropertyDifference.hxx"

namespace BCSteadyStateLibrary
{
    using LibraryCommon::presenceValue;
    using LibraryCommon::PropertyDifference;

    template<typename Visitor> void visitFields(const Convection & lhs, const Convection & rhs, Visitor & vis);
    template<typename Visitor> void visitFields(const ConstantFlux & lhs, const ConstantFlux & rhs, Visitor & vis);
    template<typename Visitor> void visitFields(const AutomaticEnclosure & lhs, const AutomaticEnclosure & rhs, Visitor & vis);
    template<typename Visitor> void visitFields(const BlackBodyRadiation & lhs, const BlackBodyRadiation & rhs, Visitor & vis);
    template<typename Visitor> void visitFields(const LinearizedRadiation & lhs, const LinearizedRadiation & rhs, Visitor & vis);
    template<typename Visitor> void visitFields(const Simplified & lhs, const Simplified & rhs, Visitor & vis);
    template<typename Visitor> void visitFields(const RadiationSurface & lhs, const RadiationSurface & rhs, Visitor & vis);
    template<typename Visitor> void visitFields(const Comprehensive & lhs, const Comprehensive & rhs, Visitor & vis);

    // ---- The single physical-field list per struct ----
    template<typename Visitor>
    void visitFields(const Convection & lhs, const Convection & rhs, Visitor & vis)
    {
        vis("Temperature", lhs.temperature, rhs.temperature);
        vis("Film coefficient", lhs.filmCoefficient, rhs.filmCoefficient);
    }

    template<typename Visitor>
    void visitFields(const ConstantFlux & lhs, const ConstantFlux & rhs, Visitor & vis)
    {
        vis("Flux", lhs.flux, rhs.flux);
    }

    template<typename Visitor>
    void visitFields(const AutomaticEnclosure & lhs, const AutomaticEnclosure & rhs, Visitor & vis)
    {
        vis("Temperature", lhs.temperature, rhs.temperature);
        vis("Emissivity", lhs.emissivity, rhs.emissivity);
    }

    template<typename Visitor>
    void visitFields(const BlackBodyRadiation & lhs, const BlackBodyRadiation & rhs, Visitor & vis)
    {
        vis("Temperature", lhs.temperature, rhs.temperature);
        vis("Emissivity", lhs.emissivity, rhs.emissivity);
        vis("View factor", lhs.viewFactor, rhs.viewFactor);
    }

    template<typename Visitor>
    void visitFields(const LinearizedRadiation & lhs, const LinearizedRadiation & rhs, Visitor & vis)
    {
        vis("Temperature", lhs.temperature, rhs.temperature);
        vis("Film coefficient", lhs.filmCoefficient, rhs.filmCoefficient);
    }

    template<typename Visitor>
    void visitFields(const Simplified & lhs, const Simplified & rhs, Visitor & vis)
    {
        vis("Relative humidity", lhs.relativeHumidity, rhs.relativeHumidity);
        vis("Temperature", lhs.temperature, rhs.temperature);
        vis("Film coefficient", lhs.filmCoefficient, rhs.filmCoefficient);
    }

    template<typename Visitor>
    void visitFields(const RadiationSurface & lhs, const RadiationSurface & rhs, Visitor & vis)
    {
        vis("Is default", lhs.isDefault, rhs.isDefault);
        vis("Temperature", lhs.temperature, rhs.temperature);
        vis("Emissivity", lhs.emissivity, rhs.emissivity);
    }

    // The radiation variant does not fit the scalar/nested/optional vocabulary, so it is handled
    // outside visitFields (see below); Comprehensive's other members flow through normally.
    template<typename Visitor>
    void visitFields(const Comprehensive & lhs, const Comprehensive & rhs, Visitor & vis)
    {
        vis("Relative humidity", lhs.relativeHumidity, rhs.relativeHumidity);
        vis.optional("Convection", lhs.convection, rhs.convection);
        vis.optional("Constant flux", lhs.constantFlux, rhs.constantFlux);
    }

    namespace
    {
        std::string radiationTypeName(const RadiationOptions & option)
        {
            if(std::holds_alternative<AutomaticEnclosure>(option))
            {
                return "Automatic enclosure";
            }
            if(std::holds_alternative<ManualEnclosure>(option))
            {
                return "Manual enclosure";
            }
            if(std::holds_alternative<BlackBodyRadiation>(option))
            {
                return "Black body radiation";
            }
            return "Linearized radiation";
        }

        template<typename RecordType>
        void appendPrefixed(std::vector<PropertyDifference> & diffs, const std::string & prefix, const RecordType & lhs, const RecordType & rhs)
        {
            LibraryCommon::DifferenceCollector collector;
            collector.prefix = prefix;
            visitFields(lhs, rhs, collector);
            diffs.insert(diffs.end(), collector.diffs.begin(), collector.diffs.end());
        }

        // Radiation = optional<Radiation{ optional<variant<...>> }>. Handle presence + variant here.
        void appendRadiationDifferences(std::vector<PropertyDifference> & diffs,
                                        const std::optional<Radiation> & lhs,
                                        const std::optional<Radiation> & rhs)
        {
            if(lhs.has_value() != rhs.has_value())
            {
                diffs.push_back({"Radiation", presenceValue(lhs.has_value()), presenceValue(rhs.has_value())});
                return;
            }
            if(!lhs.has_value())
            {
                return;
            }
            const auto & lhsOption = lhs->radiation;
            const auto & rhsOption = rhs->radiation;
            if(lhsOption.has_value() != rhsOption.has_value())
            {
                diffs.push_back({"Radiation", presenceValue(lhsOption.has_value()), presenceValue(rhsOption.has_value())});
                return;
            }
            if(!lhsOption.has_value())
            {
                return;
            }
            if(lhsOption->index() != rhsOption->index())
            {
                diffs.push_back({"Radiation-type", radiationTypeName(lhsOption.value()), radiationTypeName(rhsOption.value())});
                return;
            }
            if(std::holds_alternative<AutomaticEnclosure>(lhsOption.value()))
            {
                appendPrefixed(diffs, "Radiation", std::get<AutomaticEnclosure>(lhsOption.value()), std::get<AutomaticEnclosure>(rhsOption.value()));
            }
            else if(std::holds_alternative<BlackBodyRadiation>(lhsOption.value()))
            {
                appendPrefixed(diffs, "Radiation", std::get<BlackBodyRadiation>(lhsOption.value()), std::get<BlackBodyRadiation>(rhsOption.value()));
            }
            else if(std::holds_alternative<LinearizedRadiation>(lhsOption.value()))
            {
                appendPrefixed(diffs, "Radiation", std::get<LinearizedRadiation>(lhsOption.value()), std::get<LinearizedRadiation>(rhsOption.value()));
            }
            // ManualEnclosure has no physical fields.
        }
    }   // namespace

    template<typename RecordType>
    bool equalByFields(const RecordType & lhs, const RecordType & rhs)
    {
        LibraryCommon::EqualityCollector collector;
        visitFields(lhs, rhs, collector);
        return collector.equal;
    }

    bool operator==(const Convection & lhs, const Convection & rhs)
    {
        return equalByFields(lhs, rhs);
    }

    bool operator==(const ConstantFlux & lhs, const ConstantFlux & rhs)
    {
        return equalByFields(lhs, rhs);
    }

    bool operator==(const AutomaticEnclosure & lhs, const AutomaticEnclosure & rhs)
    {
        return equalByFields(lhs, rhs);
    }

    bool operator==(const ManualEnclosure &, const ManualEnclosure &)
    {
        return true;
    }

    bool operator==(const BlackBodyRadiation & lhs, const BlackBodyRadiation & rhs)
    {
        return equalByFields(lhs, rhs);
    }

    bool operator==(const LinearizedRadiation & lhs, const LinearizedRadiation & rhs)
    {
        return equalByFields(lhs, rhs);
    }

    bool operator==(const Radiation & lhs, const Radiation & rhs)
    {
        return lhs.radiation == rhs.radiation;
    }

    bool operator==(const Comprehensive & lhs, const Comprehensive & rhs)
    {
        return equalByFields(lhs, rhs) && lhs.radiation == rhs.radiation;
    }

    bool operator==(const Simplified & lhs, const Simplified & rhs)
    {
        return equalByFields(lhs, rhs);
    }

    bool operator==(const RadiationSurface & lhs, const RadiationSurface & rhs)
    {
        return equalByFields(lhs, rhs);
    }

    bool operator==(const BoundaryCondition & lhs, const BoundaryCondition & rhs)
    {
        return lhs.UUID == rhs.UUID && lhs.Name == rhs.Name && lhs.Protected == rhs.Protected && lhs.Color == rhs.Color
               && lhs.data == rhs.data;
    }

    bool arePhysicalPropertiesEqual(const BoundaryCondition & lhs, const BoundaryCondition & rhs)
    {
        return lhs.data == rhs.data;
    }

    std::vector<PropertyDifference> physicalDifferences(const BoundaryCondition & lhs, const BoundaryCondition & rhs)
    {
        std::vector<PropertyDifference> diffs;
        if(lhs.data.index() != rhs.data.index())
        {
            diffs.push_back({"Type",
                             std::visit(BoundaryConditionTagVisitor{}, lhs.data),
                             std::visit(BoundaryConditionTagVisitor{}, rhs.data)});
            return diffs;
        }
        if(std::holds_alternative<Comprehensive>(lhs.data))
        {
            const auto & lhsBc = std::get<Comprehensive>(lhs.data);
            const auto & rhsBc = std::get<Comprehensive>(rhs.data);
            LibraryCommon::DifferenceCollector collector;
            visitFields(lhsBc, rhsBc, collector);
            diffs = std::move(collector.diffs);
            appendRadiationDifferences(diffs, lhsBc.radiation, rhsBc.radiation);
        }
        else if(std::holds_alternative<Simplified>(lhs.data))
        {
            LibraryCommon::DifferenceCollector collector;
            visitFields(std::get<Simplified>(lhs.data), std::get<Simplified>(rhs.data), collector);
            diffs = std::move(collector.diffs);
        }
        else
        {
            LibraryCommon::DifferenceCollector collector;
            visitFields(std::get<RadiationSurface>(lhs.data), std::get<RadiationSurface>(rhs.data), collector);
            diffs = std::move(collector.diffs);
        }
        if(diffs.empty() && !arePhysicalPropertiesEqual(lhs, rhs))
        {
            diffs.push_back({"Physical properties", std::string{"differs"}, std::string{"differs"}});
        }
        return diffs;
    }

}   // namespace BCSteadyStateLibrary
