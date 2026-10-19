#include "Operators.hxx"

#include <string>
#include <variant>

#include "Tags.hxx"
#include "LibraryUtilities/Common.hxx"
#include "TimeSeriesData/Tags.hxx"

namespace BCLibrary
{
    using LibraryCommon::PropertyDifference;
    using LibraryCommon::PropertyValue;

    // ---- Leaf values: a Source compares by value within tolerance when both are constants ----
    // Found by ADL from the LibraryCommon collectors (Source is a variant of BCLibrary types).
    bool fieldEqual(const Source & lhs, const Source & rhs, const double tolerance)
    {
        if(std::holds_alternative<Constant>(lhs) && std::holds_alternative<Constant>(rhs))
        {
            return LibraryCommon::isEqual(std::get<Constant>(lhs).value, std::get<Constant>(rhs).value, tolerance);
        }
        return lhs == rhs;
    }

    bool fieldEqual(const std::optional<Source> & lhs, const std::optional<Source> & rhs, const double tolerance)
    {
        if(lhs.has_value() != rhs.has_value())
        {
            return false;
        }
        return !lhs.has_value() || fieldEqual(lhs.value(), rhs.value(), tolerance);
    }

    PropertyValue toValue(const Source & source)
    {
        if(const auto * constant{std::get_if<Constant>(&source)})
        {
            return constant->value;
        }
        return "series: " + TimeSeriesLibrary::seriesRoleToString(std::get<FromTimeSeries>(source).role);
    }

    PropertyValue toValue(const std::optional<Source> & source)
    {
        if(!source.has_value())
        {
            return std::monostate{};
        }
        return toValue(source.value());
    }

    PropertyValue toValue(const ConvectionModel model)
    {
        return convectionModelToString(model);
    }

    template<typename Visitor> void visitFields(const Convection & lhs, const Convection & rhs, Visitor & vis);
    template<typename Visitor> void visitFields(const AutomaticEnclosure & lhs, const AutomaticEnclosure & rhs, Visitor & vis);
    template<typename Visitor> void visitFields(const BlackBodyRadiation & lhs, const BlackBodyRadiation & rhs, Visitor & vis);
    template<typename Visitor>
    void visitFields(const FixedCoefficientRadiation & lhs, const FixedCoefficientRadiation & rhs, Visitor & vis);
    template<typename Visitor> void visitFields(const Solar & lhs, const Solar & rhs, Visitor & vis);
    template<typename Visitor> void visitFields(const SurfaceExchange & lhs, const SurfaceExchange & rhs, Visitor & vis);
    template<typename Visitor> void visitFields(const PrescribedState & lhs, const PrescribedState & rhs, Visitor & vis);
    template<typename Visitor> void visitFields(const RadiationSurface & lhs, const RadiationSurface & rhs, Visitor & vis);

    // ---- The single physical-field list per struct ----
    template<typename Visitor>
    void visitFields(const Convection & lhs, const Convection & rhs, Visitor & vis)
    {
        vis("Model", lhs.model, rhs.model);
        vis("Air temperature", lhs.airTemperature, rhs.airTemperature);
        vis("Film coefficient", lhs.filmCoefficient, rhs.filmCoefficient);
        vis("Wind speed", lhs.windSpeed, rhs.windSpeed);
        vis("Wind direction", lhs.windDirection, rhs.windDirection);
    }

    template<typename Visitor>
    void visitFields(const AutomaticEnclosure & lhs, const AutomaticEnclosure & rhs, Visitor & vis)
    {
        vis("Emissivity", lhs.emissivity, rhs.emissivity);
        vis("Temperature", lhs.temperature, rhs.temperature);
    }

    template<typename Visitor>
    void visitFields(const BlackBodyRadiation & lhs, const BlackBodyRadiation & rhs, Visitor & vis)
    {
        vis("Temperature", lhs.temperature, rhs.temperature);
        vis("Emissivity", lhs.emissivity, rhs.emissivity);
        vis("View factor", lhs.viewFactor, rhs.viewFactor);
    }

    template<typename Visitor>
    void visitFields(const FixedCoefficientRadiation & lhs, const FixedCoefficientRadiation & rhs, Visitor & vis)
    {
        vis("Temperature", lhs.temperature, rhs.temperature);
        vis("Coefficient", lhs.coefficient, rhs.coefficient);
    }

    template<typename Visitor>
    void visitFields(const Solar & lhs, const Solar & rhs, Visitor & vis)
    {
        vis("Irradiance", lhs.irradiance, rhs.irradiance);
        vis("Absorptance", lhs.absorptance, rhs.absorptance);
    }

    // The radiation variant does not fit the scalar/nested/optional vocabulary, so it is
    // handled outside visitFields (see below); the exchange's other members flow through.
    template<typename Visitor>
    void visitFields(const SurfaceExchange & lhs, const SurfaceExchange & rhs, Visitor & vis)
    {
        vis("Relative humidity", lhs.relativeHumidity, rhs.relativeHumidity);
        vis.optional("Convection", lhs.convection, rhs.convection);
        vis.optional("Solar", lhs.solar, rhs.solar);
        vis("Flux", lhs.flux, rhs.flux);
    }

    template<typename Visitor>
    void visitFields(const PrescribedState & lhs, const PrescribedState & rhs, Visitor & vis)
    {
        vis("Temperature", lhs.temperature, rhs.temperature);
        vis("Relative humidity", lhs.relativeHumidity, rhs.relativeHumidity);
    }

    template<typename Visitor>
    void visitFields(const RadiationSurface & lhs, const RadiationSurface & rhs, Visitor & vis)
    {
        vis("Is default", lhs.isDefault, rhs.isDefault);
        vis("Temperature", lhs.temperature, rhs.temperature);
        vis("Emissivity", lhs.emissivity, rhs.emissivity);
    }

    namespace
    {
        std::string kindName(const BoundaryCondition & record)
        {
            const Tags tags;
            if(std::holds_alternative<SurfaceExchange>(record.data))
            {
                return tags.surfaceExchange();
            }
            if(std::holds_alternative<PrescribedState>(record.data))
            {
                return tags.prescribedState();
            }
            if(std::holds_alternative<RadiationSurface>(record.data))
            {
                return tags.radiationSurface();
            }
            return "NoExchange";
        }

        std::string radiationTypeName(const RadiationOptions & option)
        {
            const Tags tags;
            if(std::holds_alternative<AutomaticEnclosure>(option))
            {
                return tags.automaticEnclosure();
            }
            if(std::holds_alternative<ManualEnclosure>(option))
            {
                return tags.manualEnclosure();
            }
            if(std::holds_alternative<BlackBodyRadiation>(option))
            {
                return tags.blackBodyRadiation();
            }
            return tags.fixedCoefficientRadiation();
        }

        template<typename RecordType>
        std::vector<PropertyDifference> collect(const std::string & prefix, const RecordType & lhs, const RecordType & rhs)
        {
            LibraryCommon::DifferenceCollector collector;
            collector.prefix = prefix;
            visitFields(lhs, rhs, collector);
            return collector.diffs;
        }

        //! ManualEnclosure has no physical fields, so for it only the type can differ.
        std::vector<PropertyDifference> radiationDifferences(const std::optional<RadiationOptions> & lhs,
                                                             const std::optional<RadiationOptions> & rhs)
        {
            if(lhs.has_value() != rhs.has_value())
            {
                return {{"Radiation", LibraryCommon::presenceValue(lhs.has_value()), LibraryCommon::presenceValue(rhs.has_value())}};
            }
            if(!lhs.has_value())
            {
                return {};
            }
            if(lhs->index() != rhs->index())
            {
                return {{"Radiation-type", radiationTypeName(lhs.value()), radiationTypeName(rhs.value())}};
            }
            return std::visit(
              [&rhs](const auto & lhsOption) -> std::vector<PropertyDifference> {
                  using Option = std::decay_t<decltype(lhsOption)>;
                  if constexpr(std::is_same_v<Option, ManualEnclosure>)
                  {
                      return {};
                  }
                  else
                  {
                      return collect("Radiation", lhsOption, std::get<Option>(rhs.value()));
                  }
              },
              lhs.value());
        }

        std::vector<PropertyDifference> exchangeDifferences(const SurfaceExchange & lhs, const SurfaceExchange & rhs)
        {
            auto diffs{collect("", lhs, rhs)};
            const auto radiation{radiationDifferences(lhs.radiation, rhs.radiation)};
            diffs.insert(diffs.end(), radiation.begin(), radiation.end());
            return diffs;
        }
    }   // namespace

    std::vector<PropertyDifference> physicalDifferences(const BoundaryCondition & lhs, const BoundaryCondition & rhs)
    {
        if(lhs.data.index() != rhs.data.index())
        {
            return {{"Type", kindName(lhs), kindName(rhs)}};
        }
        return std::visit(
          [&rhs](const auto & lhsData) -> std::vector<PropertyDifference> {
              using Kind = std::decay_t<decltype(lhsData)>;
              const auto & rhsData{std::get<Kind>(rhs.data)};
              if constexpr(std::is_same_v<Kind, NoExchange>)
              {
                  return {};
              }
              else if constexpr(std::is_same_v<Kind, SurfaceExchange>)
              {
                  return exchangeDifferences(lhsData, rhsData);
              }
              else
              {
                  return collect("", lhsData, rhsData);
              }
          },
          lhs.data);
    }

    bool arePhysicalPropertiesEqual(const BoundaryCondition & lhs, const BoundaryCondition & rhs)
    {
        return physicalDifferences(lhs, rhs).empty();
    }
}   // namespace BCLibrary
