#pragma once

#include <optional>
#include <vector>

#include "BoundaryConditions/BoundaryConditions.hxx"
#include "BoundaryConditions/DB.hxx"
#include "THMZ/Geometry/Geometry.hxx"
#include "TimeSeriesData/TimeSeriesData.hxx"

//! What a model's boundary segment refers to in the archive's libraries.
//!
//! A segment names its boundary-condition record and, when the record reads series, its
//! dataset, both by UUID. These resolve the references against the libraries, for any
//! model: one THERM saved, or one Build wrote. Whether the pair can run is
//! BCLibrary::missingRoles(record, dataset).
namespace ThermFile
{
    //! The record the boundary is bound to: by bcUUID in a consolidated archive, by the
    //! boundary's name in a pre-consolidation one, which migration does not always
    //! translate. nullopt when the library holds neither.
    [[nodiscard]] std::optional<BCLibrary::BoundaryCondition> boundaryRecord(const BCLibrary::DB & library,
                                                                             const Boundary & boundary);

    //! The dataset the boundary is bound to, by timeSeriesUUID. nullopt when the boundary
    //! binds none or no dataset carries that UUID.
    [[nodiscard]] std::optional<TimeSeriesLibrary::TimeSeriesData>
      boundaryDataset(const std::vector<TimeSeriesLibrary::TimeSeriesData> & datasets, const Boundary & boundary);
}   // namespace ThermFile
