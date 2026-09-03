#pragma once

#include <map>
#include <string>
#include <vector>

#include "BoundaryConditions/BoundaryConditions.hxx"
#include "THMZ/Model/THMX.hxx"

namespace ThermFile::Migration::BCConsolidation
{
    //! \brief Capture of the pre-consolidation BC records found in an archive, already
    //! converted to the unified form: the embedded steady-state library and transient type
    //! records become unified records (UUIDs preserved). The lookup map translates the
    //! legacy per-segment steady binding -- record name -- into the unified UUID binding.
    //! Legacy per-boundary timestep files are not converted: transient models predating
    //! the time series library are re-authored, not migrated.
    struct LegacyBCCapture
    {
        std::vector<BCLibrary::BoundaryCondition> boundaryConditions;
        std::map<std::string, std::string> bcUUIDByName;
    };

    [[nodiscard]] bool isEmpty(const LegacyBCCapture & capture);

    //! \brief Assemble a capture from already-extracted documents. Absent documents are
    //! passed as empty strings.
    [[nodiscard]] LegacyBCCapture capture(const std::string & steadyStateXml,
                                          const std::string & typeRecordsXml);

    //! \brief Scan already-extracted archive entries for pre-consolidation BC records
    //! and convert them. Returns an empty capture when the archive carries none.
    [[nodiscard]] LegacyBCCapture captureFromEntries(const std::map<std::string, std::string> & entries);

    //! \brief Convenience overload: extracts the archive once and delegates.
    [[nodiscard]] LegacyBCCapture captureFromArchive(const std::string & zipFileName);

    //! \brief Model-side migration: returns a copy of the input model where every segment
    //! whose unified record binding is absent gets it filled from the legacy binding --
    //! bcUUID from the steady record name or the transient type-record UUID. Legacy fields
    //! stay in place; segments whose legacy reference has no captured counterpart are left
    //! untouched. Idempotent: an empty capture, or a model already carrying unified
    //! bindings, is a no-op.
    [[nodiscard]] ThermFile::ThermModel applyToModel(const LegacyBCCapture & legacy,
                                                     ThermFile::ThermModel model);
}   // namespace ThermFile::Migration::BCConsolidation
