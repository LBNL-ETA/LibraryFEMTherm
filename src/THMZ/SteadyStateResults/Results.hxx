#pragma once

#include <optional>
#include <string>
#include <vector>

#include "THMZ/Common/Enumerators.hxx"
#include "THMZ/CMA/Enumerators.hxx"
#include "THMZ/Geometry/Enumerators.hxx"

#include "Enumerators.hxx"

namespace ThermFile
{
    //! \brief The state of one frame cavity after the run.
    //!
    //! Side 1 is the warm side and side 2 the cold side of the equivalent rectangular cavity:
    //! the side's average temperature and the length-weighted emissivity of the cavity surfaces
    //! on it. The program shows these values for the cavity until the next calculation and
    //! starts that calculation from them.
    //! @polygonUUID - the frame cavity polygon the state belongs to.
    //! @heatFlowDirection - screen direction of the heat flow through the cavity.
    //! @side1Temperature - temperature of the warm side [C].
    //! @side1Emissivity - emissivity of the warm side.
    //! @side2Temperature - temperature of the cold side [C].
    //! @side2Emissivity - emissivity of the cold side.
    struct FrameCavityResult
    {
        std::string polygonUUID;
        Direction heatFlowDirection{Direction::Unknown};
        double side1Temperature{0.0};
        double side1Emissivity{0.0};
        double side2Temperature{0.0};
        double side2Emissivity{0.0};
    };

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
        std::vector<FrameCavityResult> frameCavities;   // U-factor run only; empty otherwise
    };

    struct SteadyStateResults
    {
        std::string version{"1"};
        std::vector<SteadyStateResultsCase> cases;
    };
}   // namespace ThermFile