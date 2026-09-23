#pragma once

#include <map>
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
//! - BoundaryConditions: the consolidated records the segments refer to. They are the same
//!   in every generated file: THERM's own built-in Adiabatic record, by its shipped UUID,
//!   so THERM recognises it rather than importing a duplicate; and two prescribed kinds and
//!   a convective kind, marked as the project's own and reading their values from the
//!   segment's dataset.
//! - time series/<uuid>: one dataset per segment that reads values, holding the constant
//!   boundary values repeated for every timestep. The datasets also set the clock: their
//!   row count is the number of timesteps and their step length is Schedule::dtime. A model
//!   whose segments are all adiabatic still needs a clock, so it binds one dataset to its
//!   first segment; the adiabatic record ignores the values.
//! - Gases, Mesh: version headers only, as THERM writes for a model without cavities or a
//!   stored mesh.
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

    //! The UUID of a boundary-condition record, the same in every generated file. The
    //! built-in Adiabatic record keeps THERM's UUID; every other record derives one from its
    //! name.
    [[nodiscard]] std::string recordUuid(std::string_view recordName);

    //! The library record for a boundary kind. Values live in the dataset, not here.
    //! ``project`` is written as the record's project name: THERM imports an archive's
    //! records into its library only when they are marked as the project's own. The
    //! Adiabatic record is THERM's built-in and carries no project name.
    [[nodiscard]] BCLibrary::BoundaryCondition boundaryRecord(const Model::Boundary & boundary, std::string_view project);

    //! (role name, constant value) pairs, one per series the segment's dataset carries, in
    //! the order the dataset writes them: temperature before humidity for a prescribed
    //! state; air temperature, film coefficient, humidity for an exchange. Empty for a
    //! sealed segment.
    [[nodiscard]] std::vector<std::pair<std::string, double>> seriesValues(const Model::Boundary & boundary);

    //! Segment index -> dataset for every segment that reads values, plus a clock on the
    //! first segment when none does.
    [[nodiscard]] std::map<std::size_t, TimeSeriesLibrary::TimeSeriesData> boundaryDatasets(const Model::ModelCase & modelCase);

    //! The THERM model of the case: polygons, boundary segments and calculation options.
    //! Precondition: issues(modelCase) is empty.
    [[nodiscard]] ThermModel model(const Model::ModelCase & modelCase);

    //! The materials, the boundary-condition records and the datasets the model uses.
    //! Precondition: issues(modelCase) is empty.
    [[nodiscard]] Libraries libraries(const Model::ModelCase & modelCase);

    //! Every entry of the case's THMZ, name -> content, before zipping. Fails with the
    //! case's issues, one per line, when it cannot be written.
    [[nodiscard]] lbnl::ExpectedExt<std::map<std::string, std::string>, std::string>
      archiveEntries(const Model::ModelCase & modelCase);

    //! Writes the case's THMZ and returns the path written. Creates the parent directory
    //! and overwrites an existing file. Fails with the case's issues when it cannot be
    //! written.
    [[nodiscard]] lbnl::ExpectedExt<std::string, std::string> archive(const Model::ModelCase & modelCase,
                                                                          const std::string & path);

    //! The clock the mediator will run, read from the first dataset as the mediator does.
    [[nodiscard]] Model::Schedule scheduleOf(const std::vector<TimeSeriesLibrary::TimeSeriesData> & datasets);
}   // namespace ThermFile::Build
