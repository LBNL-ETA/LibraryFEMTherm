#pragma once

#include <string>

#include <lbnl/expected.hxx>

#include "Legacy/Step1/BCSteadyState/SteadyState.hxx"
#include "Legacy/Step1/BCTransient/Timestep.hxx"
#include "Legacy/Step1/BCTransient/Types.hxx"
#include "TimeSeriesData/TimeSeriesData.hxx"

#include "BoundaryConditions/BoundaryConditions.hxx"

namespace BCLibrary
{
    //! Legacy steady-state record to unified record. UUIDs are preserved so per-segment
    //! references survive migration unchanged. All resulting sources are constants.
    [[nodiscard]] BoundaryCondition fromSteadyState(const BCSteadyStateLibrary::BoundaryCondition & legacy);

    //! Unified record back to the legacy steady-state shape, for the consumers that still
    //! speak it.
    //!
    //! Every surface exchange comes back as a Comprehensive record. The legacy split
    //! between Simplified and Comprehensive was a choice the user had to make before
    //! describing a boundary; the unified record has one shape and no such choice, so
    //! rebuilding the distinction here would invent information the record does not
    //! carry. Comprehensive is the shape that can hold all of it, adiabatic included.
    //!
    //! What steady state has no room for - a prescribed state, or any input reading a
    //! time series - is reported rather than approximated.
    [[nodiscard]] lbnl::ExpectedExt<BCSteadyStateLibrary::BoundaryCondition, std::string>
      toSteadyState(const BoundaryCondition & unified);

    //! Legacy transient type record to unified record (UUID preserved). Source inference:
    //! for bcType SteadyState every model-required input becomes a Constant taken from the
    //! record's optional values; for bcType Transient every model-required input becomes
    //! FromTimeSeries with the matching role. This reproduces exactly what the legacy
    //! validator demanded of a timestep file, so every previously valid record-file pairing
    //! stays valid after migration.
    [[nodiscard]] BoundaryCondition fromTypeRecord(const BCTypesLibrary::TypeRecord & legacy);

    //! Legacy timestep file to time series dataset: model-specific row types decompose into
    //! model-agnostic role series. The dataset UUID is the deterministic content hash, so
    //! identical files embedded in different projects converge to one dataset.
    [[nodiscard]] TimeSeriesLibrary::TimeSeriesData
      environmentFromTimestep(const BCInputFileLibrary::BoundaryConditionTimestep & legacy,
                              const std::string & datasetName);
}   // namespace BCLibrary
