#pragma once

#include <string>

#include "THMZ/Model/THMX.hxx"

namespace ThermFile::Migration
{
    //! \brief Run every model-side legacy schema migration that the THMZ load
    //! path needs, returning the migrated model. Each migration reads the raw
    //! legacy hints it needs out of the archive (e.g. Materials.xml for the
    //! frame-cavity migration) and yields a rewritten model. Idempotent for
    //! already-migrated archives.
    //!
    //! This is the **only** function `loadThermModelFromZipFile` calls on the
    //! migration side. Register new model-side migrations in this orchestrator
    //! so the loader stays clean.
    [[nodiscard]] ThermFile::ThermModel applyAllToModel(const std::string & zipFileName,
                                                       ThermFile::ThermModel model);
}   // namespace ThermFile::Migration
