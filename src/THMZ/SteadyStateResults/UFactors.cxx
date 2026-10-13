#include <cmath>

#include <lbnl/algorithm.hxx>

#include "UFactors.hxx"

namespace ThermFile
{
    bool isDegenerate(const Projection & projection)
    {
        return projection.length.has_value() && std::abs(projection.length.value()) <= degenerateProjectionLength;
    }

    bool isReportable(const Projection & projection)
    {
        return projection.uFactor.has_value() && projection.length.has_value() && !isDegenerate(projection);
    }

    std::optional<Projection> projectionOf(const SteadyStateUFactors & tag, const UValueDimensionType lengthType)
    {
        return lbnl::find_element(
          tag.projections, [lengthType](const Projection & projection) { return projection.lengthType == lengthType; });
    }

    std::optional<Projection> displayedProjection(const SteadyStateUFactors & tag)
    {
        return projectionOf(tag, tag.defaultDisplayType);
    }

    std::optional<double> displayedUFactor(const SteadyStateUFactors & tag)
    {
        const auto projection{displayedProjection(tag)};
        if(!projection.has_value() || !isReportable(projection.value()))
        {
            return std::nullopt;
        }
        return projection->uFactor;
    }
}   // namespace ThermFile
