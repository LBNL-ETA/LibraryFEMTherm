#pragma once

#include <string>

#include "TimeSeriesData.hxx"

namespace TimeSeriesLibrary
{
    //! Deterministic UUID-formatted content hash over the dataset's channels (roles and raw
    //! value bits). Equal content yields an equal UUID regardless of file name or origin, so
    //! the same legacy timestep file embedded in many projects converges to one dataset, and
    //! name collisions between different files are neutralized. The envelope (Name,
    //! Protected, Description) deliberately does not participate.
    [[nodiscard]] std::string contentUuid(const TimeSeriesData & data);
}   // namespace TimeSeriesLibrary
