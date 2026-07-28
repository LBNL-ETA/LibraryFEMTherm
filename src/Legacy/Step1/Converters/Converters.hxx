#pragma once

#include <string>

#include "Legacy/Step1/BCSteadyState/SteadyState.hxx"
#include "Legacy/Step1/BCTransient/Timestep.hxx"
#include "Legacy/Step1/BCTransient/Types.hxx"
#include "EnvironmentData/EnvironmentData.hxx"

#include "BoundaryConditions/BoundaryConditions.hxx"

namespace BCLibrary
{
    //! Legacy steady-state record to unified record. UUIDs are preserved so per-segment
    //! references survive migration unchanged. All resulting sources are constants.
    [[nodiscard]] BoundaryCondition fromSteadyState(const BCSteadyStateLibrary::BoundaryCondition & legacy);

    //! Legacy transient type record to unified record (UUID preserved). Source inference:
    //! for bcType SteadyState every model-required input becomes a Constant taken from the
    //! record's optional values; for bcType Transient every model-required input becomes
    //! FromEnvironment with the matching role. This reproduces exactly what the legacy
    //! validator demanded of a timestep file, so every previously valid record-file pairing
    //! stays valid after migration.
    [[nodiscard]] BoundaryCondition fromTypeRecord(const BCTypesLibrary::TypeRecord & legacy);

    //! Legacy timestep file to environment dataset: model-specific row types decompose into
    //! model-agnostic role channels. The dataset UUID is the deterministic content hash, so
    //! identical files embedded in different projects converge to one dataset.
    [[nodiscard]] EnvironmentDataLibrary::EnvironmentData
      environmentFromTimestep(const BCInputFileLibrary::BoundaryConditionTimestep & legacy,
                              const std::string & datasetName);
}   // namespace BCLibrary
