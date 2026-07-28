#pragma once

#include <string>
#include <vector>

#include "THMZ/Properties/Enumerators.hxx"

#include "BoundaryConditions.hxx"

namespace BCLibrary
{
    //! One reason a record cannot run under an engine-mode combination, anchored to the
    //! offending feature so dialogs can mark it in place next to that feature.
    struct CapabilityIssue
    {
        std::string feature;
        std::string reason;
    };

    //! The single capability gate of the BC consolidation: assignment filtering, the
    //! engine-switch scan and the simulate pre-flight all evaluate records through this
    //! function. The Conrad column is verified against the actual hand-off code (see the
    //! capability matrix and audit findings in the design document): only fixed-coefficient
    //! convection, constant flux, enclosure and black-body radiation reach the engine;
    //! fixed-coefficient radiation rides on the convection coefficient and is silently
    //! dropped without a convection component; prescribed state has no user-facing path.
    //! An empty result means the record runs under the given engine and mode.
    [[nodiscard]] std::vector<CapabilityIssue> capabilityIssues(const BoundaryCondition & record,
                                                                ThermFile::SimulationEngine engine,
                                                                ThermFile::CalculationMode mode);

    [[nodiscard]] bool usable(const BoundaryCondition & record,
                              ThermFile::SimulationEngine engine,
                              ThermFile::CalculationMode mode);
}   // namespace BCLibrary
