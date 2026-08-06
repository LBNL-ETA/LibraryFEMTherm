#include <variant>

#include "Capability.hxx"

#include "TimeSeriesData/Tags.hxx"
#include "Tags.hxx"

namespace BCLibrary
{
    namespace
    {
        const std::string conradReason{"not supported by the Conrad engine"};
        const std::string transientReason{"requires the transient calculation mode"};

        void addTimeSeriesSourceIssues(const BoundaryCondition & record,
                                        const std::string & reason,
                                        std::vector<CapabilityIssue> & issues)
        {
            for(const auto role : requiredRoles(record))
            {
                issues.push_back({TimeSeriesLibrary::channelRoleToString(role), reason});
            }
        }

        void addConradSurfaceExchangeIssues(const SurfaceExchange & exchange, std::vector<CapabilityIssue> & issues)
        {
            const auto & convection{exchange.convection};
            if(convection.has_value() && convection->model != ConvectionModel::Fixed_Convection_Coefficient)
            {
                issues.push_back({convectionModelToString(convection->model), conradReason});
            }

            const bool fixedRadiation{exchange.radiation.has_value()
                                      && std::holds_alternative<FixedCoefficientRadiation>(exchange.radiation.value())};
            if(fixedRadiation && !convection.has_value())
            {
                issues.push_back(
                  {"Fixed-coefficient radiation", "produces no Conrad input without a convection component"});
            }

            if(exchange.solar.has_value())
            {
                issues.push_back({"Solar", conradReason});
            }
        }

        void addConradIssues(const BoundaryCondition & record, std::vector<CapabilityIssue> & issues)
        {
            if(const auto * exchange{std::get_if<SurfaceExchange>(&record.data)}; exchange != nullptr)
            {
                addConradSurfaceExchangeIssues(*exchange, issues);
            }
            else if(std::holds_alternative<PrescribedState>(record.data))
            {
                issues.push_back({"Prescribed state", conradReason});
            }

            addTimeSeriesSourceIssues(record, conradReason, issues);
        }
    }   // namespace

    std::vector<CapabilityIssue> capabilityIssues(const BoundaryCondition & record,
                                                  ThermFile::SimulationEngine engine,
                                                  ThermFile::CalculationMode mode)
    {
        std::vector<CapabilityIssue> issues;

        // Radiation-surface records are internal companions of glazing systems; every
        // engine consumes them regardless of mode.
        if(std::holds_alternative<RadiationSurface>(record.data))
        {
            return issues;
        }

        if(engine == ThermFile::SimulationEngine::Conrad)
        {
            addConradIssues(record, issues);
            return issues;
        }

        if(mode == ThermFile::CalculationMode::cmSteadyState)
        {
            addTimeSeriesSourceIssues(record, transientReason, issues);
        }

        return issues;
    }

    bool usable(const BoundaryCondition & record,
                ThermFile::SimulationEngine engine,
                ThermFile::CalculationMode mode)
    {
        return capabilityIssues(record, engine, mode).empty();
    }
}   // namespace BCLibrary
