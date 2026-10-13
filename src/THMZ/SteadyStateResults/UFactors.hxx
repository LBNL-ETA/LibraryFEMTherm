#pragma once

#include <optional>

#include "Results.hxx"

//! Reading a U-factor out of a tag's projections the way THERM's results screen does: each
//! tag stores one projection per length type, and the one shown is the tag's default
//! display type - when it holds a number worth showing.
namespace ThermFile
{
    //! A projected length at or below this, in the file's length unit, is degenerate. THERM
    //! stores a Custom Rotation projection for every tag even when its projection angle is
    //! zero, which divides the heat flux by a length of about 5e-15 and reports a U-factor of
    //! about 1e16. Printing that as a number invites someone to believe it.
    inline constexpr double degenerateProjectionLength{1e-9};

    //! The projection has a length, and that length is degenerate.
    [[nodiscard]] bool isDegenerate(const Projection & projection);

    //! The projection carries a U-factor and a length, and the length is not degenerate.
    [[nodiscard]] bool isReportable(const Projection & projection);

    //! The tag's projection of that length type; empty when the tag stores none.
    [[nodiscard]] std::optional<Projection> projectionOf(const SteadyStateUFactors & tag,
                                                         UValueDimensionType lengthType);

    //! The projection THERM shows for the tag: the one of its default display type.
    [[nodiscard]] std::optional<Projection> displayedProjection(const SteadyStateUFactors & tag);

    //! The U-factor THERM shows for the tag; empty when that projection is missing or not
    //! reportable.
    [[nodiscard]] std::optional<double> displayedUFactor(const SteadyStateUFactors & tag);
}   // namespace ThermFile
