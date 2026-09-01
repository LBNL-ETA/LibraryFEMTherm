#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <fileParse/FileFormat.hxx>

#include "TimeSeriesData.hxx"

namespace TimeSeriesLibrary
{
    //! What a THMZ save must do to its "time series/" entries: datasets whose entry must be
    //! (re)written, and entry names that no longer correspond to any dataset.
    struct EntryPlan
    {
        std::vector<std::string> toWrite;   //!< dataset UUIDs
        std::vector<std::string> toErase;   //!< archive entry names
    };

    //! Decides which time-series entries a save can skip. A dataset is skipped only when
    //! its entry (in the requested format) already exists in the archive AND the stored
    //! fingerprint matches datasetFingerprint of the current data — so metadata edits,
    //! value edits, and format switches all schedule a rewrite. toErase reproduces the
    //! previous wholesale cleanup: every existing entry no dataset claims (unbound
    //! datasets, other-format leftovers, entries left over from document reuse).
    [[nodiscard]] EntryPlan
      planEntryUpdates(const std::vector<std::string> & existingEntryNames,
                       const std::vector<TimeSeriesData> & datasets,
                       const std::map<std::string, std::uint64_t> & knownFingerprints,
                       FileParse::FileFormat format);
}   // namespace TimeSeriesLibrary
