#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include <lbnl/expected.hxx>

#include "BoundaryConditions/BoundaryConditions.hxx"
#include "BoundaryConditions/DB.hxx"
#include "Materials/DB.hxx"
#include "THMZ/Model/THMX.hxx"
#include "TimeSeriesData/TimeSeriesData.hxx"

#include "Case.hxx"

//! A ModelCase as the THMZ THERM would save: the model and its libraries.
//!
//! The mediator reads a THMZ the way THERM writes one, so the generated archive follows
//! THERM's own layout entry by entry:
//!
//! - Model: the drawing and the calculation options. Regions become polygons in page
//!   millimetres around THERM's default drawing origin (cosmetic; the mediator subtracts
//!   the origin and divides by 1000 to get the mesh's metres). Segments become boundary
//!   records that name their condition record by UUID and, when the record reads series,
//!   their dataset by UUID. The case is completed first (Model::completed): every outside
//!   face the caller left unstated is written adiabatic, so a caller states only the
//!   faces that exchange something. The calculation options are the case's Physics, Initial and
//!   Numerics written as the file's switches.
//! - Materials: the regions' materials, converted by Materials.hxx.
//! - BoundaryConditions: the consolidated records the segments refer to. Each segment's
//!   condition is lowered to a record: a constant input stays a constant on the record, a
//!   series input becomes a source reading a dataset role. Segments whose records come out
//!   the same share one record, named after the first segment that uses it (its own name,
//!   or "segment N") and drawn in that segment's colour; an adiabatic segment uses THERM's
//!   own built-in Adiabatic record, by its shipped UUID, so THERM recognises it rather than
//!   importing a duplicate. Records are marked as the project's own.
//! - time series/<uuid>: one dataset per distinct bundle of series inputs, holding each
//!   series under its role on the schedule's axis. Its UUID is the content hash, so segments
//!   reading the same weather share one dataset. A segment whose inputs are all constants
//!   binds no dataset. The datasets also set the clock: their row count is the number of
//!   timesteps and their step length is Schedule::dtime. A model in which no segment reads a
//!   series still needs a clock, so it binds one dataset to its first segment; the record
//!   ignores the values.
//! - Gases, Mesh: version headers only, as THERM writes for a model without cavities or a
//!   stored mesh.
//!
//! A steady-state case (ModelCase::mode) has no clock, so it writes no datasets; issues()
//! has refused any series input. The records go into the consolidated BoundaryConditions
//! entry only, as in a transient case: the legacy SteadyStateBC library is not written. The
//! calculation options carry the case's MeshControl, which drives the error estimator.
namespace ThermFile::Build
{
    //! THERM's default drawing origin in page millimetres. The drawing is placed around it,
    //! as THERM places a model drawn by hand, so the file opens in the middle of THERM's
    //! canvas rather than in its corner. Cosmetic and not necessary for the solve.
    constexpr ThermFile::Point drawingOriginMm{250.0, -250.0};

    //! Everything a model refers to by UUID.
    struct Libraries
    {
        MaterialsLibrary::DB materials;
        BCLibrary::DB boundaryConditions;
        std::vector<TimeSeriesLibrary::TimeSeriesData> datasets;
    };

    //! A UUID that depends on the case id and a key, stable across runs.
    [[nodiscard]] std::string caseUuid(const Model::ModelCase & modelCase, std::string_view key);

    //! The UUID of THERM's built-in "Adiabatic" boundary-condition record, as shipped in
    //! its transient and steady-state libraries.
    constexpr std::string_view adiabaticRecordUuid{"61d7bd1c-22c6-4ea0-8720-0696e8c194ad"};

    //! The data of a boundary-condition record: what a boundary kind lowers to.
    using RecordData = decltype(BCLibrary::BoundaryCondition::data);

    //! A boundary kind lowered to the file's terms: the record data, with a constant input
    //! as a constant source and a series input as a source reading its role, and the series
    //! the record reads, each under that role.
    struct Lowered
    {
        RecordData data{};
        std::vector<TimeSeriesLibrary::Series> series{};
    };

    [[nodiscard]] Lowered lower(const Model::Boundary & boundary);

    //! What a segment refers to by UUID: its record, and its dataset when the record reads
    //! series (or when the segment carries the clock).
    struct Binding
    {
        std::string recordUuid{};
        std::optional<std::string> datasetUuid{};

        [[nodiscard]] friend bool operator==(const Binding & lhs, const Binding & rhs) = default;
    };

    //! Every record the case's segments refer to, each once, in the order first used:
    //! THERM's built-in Adiabatic record, and one record per distinct lowered condition.
    [[nodiscard]] std::vector<BCLibrary::BoundaryCondition> segmentRecords(const Model::ModelCase & modelCase);

    //! The record and dataset each segment of the completed case refers to, one entry per
    //! segment in the completed case's order.
    [[nodiscard]] std::vector<Binding> segmentBindings(const Model::ModelCase & modelCase);

    //! Segment index -> dataset for every segment of the completed case that reads series,
    //! plus a clock on the first segment when none does. Segments sharing a dataset repeat
    //! it under each index.
    [[nodiscard]] std::map<std::size_t, TimeSeriesLibrary::TimeSeriesData>
      boundaryDatasets(const Model::ModelCase & modelCase);

    //! The THERM model of the case: polygons, boundary segments and calculation options.
    //! Precondition: issues(modelCase) is empty.
    [[nodiscard]] ThermModel model(const Model::ModelCase & modelCase);

    //! The materials, the boundary-condition records and the datasets the model uses.
    //! Precondition: issues(modelCase) is empty.
    [[nodiscard]] Libraries libraries(const Model::ModelCase & modelCase);

    //! Every entry of the case's THMZ, name -> content, before zipping. Fails with the
    //! case's issues, one per line, when it cannot be written.
    [[nodiscard]] lbnl::ExpectedExt<std::map<std::string, std::string>, std::string>
      thmzEntries(const Model::ModelCase & modelCase);

    //! Writes the case's THMZ and returns the path written. Creates the parent directory
    //! and overwrites an existing file. Fails with the case's issues when it cannot be
    //! written.
    [[nodiscard]] lbnl::ExpectedExt<std::string, std::string> thmz(const Model::ModelCase & modelCase,
                                                                   const std::string & path);

    //! The clock the mediator will run, read from the first dataset as the mediator does.
    [[nodiscard]] Model::Schedule scheduleOf(const std::vector<TimeSeriesLibrary::TimeSeriesData> & datasets);
}   // namespace ThermFile::Build
