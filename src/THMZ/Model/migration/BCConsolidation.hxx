#pragma once

#include <map>
#include <string>
#include <vector>

#include "BoundaryConditions/BoundaryConditions.hxx"
#include "EnvironmentData/EnvironmentData.hxx"
#include "THMZ/Model/THMX.hxx"

namespace ThermFile::Migration::BCConsolidation
{
    //! \brief Capture of the pre-consolidation BC artifacts found in an archive, already
    //! converted to the unified form: the embedded steady-state library and transient type
    //! records become unified records (UUIDs preserved), timestep entries become environment
    //! datasets (content-hash UUIDs). The lookup maps translate the legacy per-segment
    //! bindings -- record name (steady) and bare timestep file name (transient) -- into the
    //! unified UUID binding.
    struct LegacyBCCapture
    {
        std::vector<BCLibrary::BoundaryCondition> boundaryConditions;
        std::vector<EnvironmentDataLibrary::EnvironmentData> datasets;
        std::map<std::string, std::string> bcUUIDByName;
        std::map<std::string, std::string> datasetUUIDByFileName;
    };

    [[nodiscard]] bool isEmpty(const LegacyBCCapture & capture);

    //! \brief Assemble a capture from already-extracted documents. Absent documents are
    //! passed as empty strings. Timestep files are keyed by their bare file name, exactly
    //! as segments reference them. A timestep file that fails to parse is skipped: the
    //! old transient format is migrated best-effort, while steady-state records always
    //! convert (the unified record subsumes them field by field).
    [[nodiscard]] LegacyBCCapture capture(const std::string & steadyStateXml,
                                          const std::string & typeRecordsXml,
                                          const std::map<std::string, std::string> & timestepFilesByName);

    //! \brief Scan a THMZ archive for pre-consolidation BC artifacts and convert them.
    //! Returns an empty capture when the archive carries none.
    [[nodiscard]] LegacyBCCapture captureFromArchive(const std::string & zipFileName);

    //! \brief Model-side migration: returns a copy of the input model where every segment
    //! whose unified binding is absent gets it filled from the legacy binding -- bcUUID
    //! from the steady record name or the transient type-record UUID, environmentUUID from
    //! the timestep file name via the dataset map. Legacy fields stay in place; segments
    //! whose legacy reference has no captured counterpart are left untouched (best-effort
    //! transient migration). Idempotent: an empty capture, or a model already carrying
    //! unified bindings, is a no-op.
    [[nodiscard]] ThermFile::ThermModel applyToModel(const LegacyBCCapture & legacy,
                                                     ThermFile::ThermModel model);
}   // namespace ThermFile::Migration::BCConsolidation
