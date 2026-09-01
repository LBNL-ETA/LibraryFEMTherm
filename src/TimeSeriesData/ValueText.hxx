#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace TimeSeriesLibrary
{
    //! Series values as one comma-separated text node. A dataset of EPW size (8760 steps,
    //! several series) serialized as one <Value> element per point costs ~50,000 DOM nodes
    //! per save/load; a single joined text node reduces that to a handful. Each value is
    //! rendered through FileParse::formatDouble with the active serialization configuration,
    //! so the per-value text is identical to what the per-element form produced.
    [[nodiscard]] std::string formatSeriesValues(const std::vector<double> & values);

    //! Inverse of formatSeriesValues; empty text yields an empty vector.
    [[nodiscard]] std::vector<double> parseSeriesValues(std::string_view text);
}   // namespace TimeSeriesLibrary
