#pragma once

#include <map>
#include <string>

#include "THMZ/Model/THMX.hxx"

namespace ThermFile::Migration
{
    //! \brief Run every model-side legacy schema migration that the THMZ load
    //! path needs, returning the migrated model. Each migration reads the raw
    //! legacy hints it needs out of the extracted archive entries (e.g. the
    //! materials entry for the frame-cavity migration) and yields a rewritten
    //! model. Idempotent for already-migrated archives.
    //!
    //! This is the **only** function the model loaders call on the migration
    //! side. Register new model-side migrations in this orchestrator so the
    //! loaders stay clean.
    [[nodiscard]] ThermFile::ThermModel applyAllToModel(const std::map<std::string, std::string> & entries,
                                                        ThermFile::ThermModel model);

    //! Convenience overload: extracts the archive once and delegates.
    [[nodiscard]] ThermFile::ThermModel applyAllToModel(const std::string & zipFileName,
                                                        ThermFile::ThermModel model);
}   // namespace ThermFile::Migration
