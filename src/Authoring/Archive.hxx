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

#include "Model.hxx"

//! A ModelCase as the THMZ THERM would save: the model and its libraries.
//!
//! The mediator reads a THMZ the way THERM writes one, so the generated archive follows
//! THERM's own layout entry by entry:
//!
//! - Model: the drawing and the calculation options. Regions become polygons in page
//!   millimetres around THERM's default drawing origin (cosmetic; the mediator subtracts
//!   the origin and divides by 1000 to get the mesh's metres). Segments become boundary
//!   records that name their condition record by UUID and, when the record reads series,
//!   their dataset by UUID. The calculation options are the case's Physics, Initial and
//!   Numerics written as the file's switches.
//! - Materials: the regions' materials, converted by Materials.hxx.
//! - BoundaryConditions: the consolidated records the segments refer to. They are the same
//!   in every generated file: a sealed kind, two prescribed kinds and a convective kind,
//!   all reading their values from the segment's dataset.
//! - time series/<uuid>: one dataset per segment that reads values, holding the constant
//!   boundary values repeated for every timestep. The datasets also set the clock: their
//!   row count is the number of timesteps and their step length is Schedule::dtime. A model
//!   whose segments are all sealed still needs a clock, so it binds one dataset to its
//!   first segment; the sealed record ignores the values.
//! - Gases, Mesh: version headers only, as THERM writes for a model without cavities or a
//!   stored mesh.
namespace ThermFile::Authoring
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
    [[nodiscard]] std::string caseUuid(const ModelCase & modelCase, std::string_view key);

    //! The UUID of a boundary-condition record, the same in every generated file.
    [[nodiscard]] std::string recordUuid(std::string_view recordName);

    //! The library record for a boundary kind. Values live in the dataset, not here.
    //! ``project`` is written as the record's project name: THERM imports an archive's
    //! records into its library only when they are marked as the project's own.
    [[nodiscard]] BCLibrary::BoundaryCondition boundaryRecord(const Boundary & boundary, std::string_view project);

    //! (role name, constant value) pairs, one per series the segment's dataset carries, in
    //! the order the dataset writes them: temperature before humidity for a prescribed
    //! state; air temperature, film coefficient, humidity for an exchange. Empty for a
    //! sealed segment.
    [[nodiscard]] std::vector<std::pair<std::string, double>> seriesValues(const Boundary & boundary);

    //! Segment index -> dataset for every segment that reads values, plus a clock on the
    //! first segment when none does.
    [[nodiscard]] std::map<std::size_t, TimeSeriesLibrary::TimeSeriesData> boundaryDatasets(const ModelCase & modelCase);

    //! The THERM model of the case: polygons, boundary segments and calculation options.
    //! Precondition: issues(modelCase) is empty.
    [[nodiscard]] ThermModel buildModel(const ModelCase & modelCase);

    //! The materials, the boundary-condition records and the datasets the model uses.
    //! Precondition: issues(modelCase) is empty.
    [[nodiscard]] Libraries buildLibraries(const ModelCase & modelCase);

    //! Every entry of the case's THMZ, name -> content, before zipping. Fails with the
    //! case's issues, one per line, when it cannot be written.
    [[nodiscard]] lbnl::ExpectedExt<std::map<std::string, std::string>, std::string>
      archiveEntries(const ModelCase & modelCase);

    //! Writes the case's THMZ and returns the path written. Creates the parent directory
    //! and overwrites an existing file. Fails with the case's issues when it cannot be
    //! written.
    [[nodiscard]] lbnl::ExpectedExt<std::string, std::string> writeArchive(const ModelCase & modelCase,
                                                                          const std::string & path);

    //! The clock the mediator will run, read from the first dataset as the mediator does.
    [[nodiscard]] Schedule scheduleOf(const std::vector<TimeSeriesLibrary::TimeSeriesData> & datasets);
}   // namespace ThermFile::Authoring
