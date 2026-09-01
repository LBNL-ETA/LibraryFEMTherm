#pragma once

#include <cstdint>
#include <string>

#include "TimeSeriesData.hxx"

namespace TimeSeriesLibrary
{
    //! Deterministic UUID-formatted content hash over the dataset's series (roles and raw
    //! value bits). Equal content yields an equal UUID regardless of file name or origin, so
    //! the same legacy timestep file embedded in many projects converges to one dataset, and
    //! name collisions between different files are neutralized. The envelope (Name,
    //! Protected, Description) deliberately does not participate.
    [[nodiscard]] std::string contentUuid(const TimeSeriesData & data);

    //! Hash over the whole dataset: envelope metadata AND series. Answers "would
    //! re-serializing this dataset produce a different archive entry", which contentUuid
    //! cannot: the UUID excludes metadata (and a copied record may carry a random UUID),
    //! so "UUID unchanged" does not imply "entry unchanged". Series order does not
    //! affect the result, matching contentUuid.
    [[nodiscard]] std::uint64_t datasetFingerprint(const TimeSeriesData & data);
}   // namespace TimeSeriesLibrary
