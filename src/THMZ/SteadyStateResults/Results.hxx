#pragma once

#include <vector>
#include <optional>

#include "THMZ/Common/Enumerators.hxx"
#include "THMZ/CMA/Enumerators.hxx"

#include "Enumerators.hxx"

namespace ThermFile
{
    struct Projection
    {
        UValueDimensionType lengthType{UValueDimensionType::Unknown};
        std::optional<double> length;
        std::optional<double> uFactor;
        std::optional<double> projectionAngle;
    };

    struct SteadyStateUFactors
    {
        std::string tag;
        double deltaT{0.0};
        double heatFlux{0.0};
        UValueDimensionType defaultDisplayType{UValueDimensionType::YProjected};
        std::vector<Projection> projections;
    };

    //! Case is used to store the results for standard run as well as for CMA runs.
    //! This is why most of the fields are optional.
    struct SteadyStateResultsCase
    {
        std::optional<RunType> modelType;        // Used in standard run
        std::optional<double> simulationError;   // Used in both runs but not always available
        std::optional<CMACase> glazingCase;      // Used in CMA run
        std::optional<CMACase> spacerCase;       // Used in CMA run
        std::vector<SteadyStateUFactors> steadyStateUFactors;
    };

    struct SteadyStateResults
    {
        std::string version{"1"};
        std::vector<SteadyStateResultsCase> cases;
    };
}   // namespace ThermFile