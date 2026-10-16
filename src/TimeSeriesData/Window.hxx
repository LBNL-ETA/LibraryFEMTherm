#pragma once

#include <cstddef>
#include <string>

#include <lbnl/expected.hxx>

#include "TimeSeriesData.hxx"

namespace TimeSeriesLibrary
{
    //! Rows [firstStep, firstStep + count) of every series, as a dataset of its own: the
    //! axis moved to the first row kept and the UUID recomputed from the content, since the
    //! same values a month apart are different data. Name, colour and the other metadata
    //! are kept. This is how one month of a yearly weather file becomes the clock of a run.
    //! Fails when the window is empty or reaches past the end of the shortest series.
    [[nodiscard]] lbnl::ExpectedExt<TimeSeriesData, std::string>
      window(const TimeSeriesData & data, std::size_t firstStep, std::size_t count);
}   // namespace TimeSeriesLibrary
