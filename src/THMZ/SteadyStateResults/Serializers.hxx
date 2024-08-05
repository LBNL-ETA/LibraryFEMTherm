#pragma once

#include "Results.hxx"

#include <fileParse/Vector.hxx>
#include <fileParse/Optional.hxx>

#include "EnumSerializers.hxx"
#include "THMZ/Common/EnumSerializers.hxx"
#include "THMZ/CMA/EnumSerializers.hxx"

namespace ThermFile
{
    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, Projection & projection)
    {
        node >> FileParse::Child{"Length-type", projection.lengthType};
        node >> FileParse::Child{"Length", projection.length};
        node >> FileParse::Child{"U-factor", projection.uFactor};
        node >> FileParse::Child{"ProjectionAngle", projection.projectionAngle};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const Projection & projection)
    {
        node << FileParse::Child{"Length-type", projection.lengthType};
        node << FileParse::Child{"Length", projection.length};
        node << FileParse::Child{"U-factor", projection.uFactor};
        node << FileParse::Child{"ProjectionAngle", projection.projectionAngle};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, SteadyStateUFactors & steadyStateUFactors)
    {
        node >> FileParse::Child{"Tag", steadyStateUFactors.tag};
        node >> FileParse::Child{"DeltaT", steadyStateUFactors.deltaT};
        node >> FileParse::Child{"HeatFlux", steadyStateUFactors.heatFlux};
        node >> FileParse::Child{"DefaultDisplayType", steadyStateUFactors.defaultDisplayType};
        // Vector is intentionally at the same level as the other nodes
        node >> FileParse::Child{"Projection", steadyStateUFactors.projections};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const SteadyStateUFactors & steadyStateUFactors)
    {
        node << FileParse::Child{"Tag", steadyStateUFactors.tag};
        node << FileParse::Child{"DeltaT", steadyStateUFactors.deltaT};
        node << FileParse::Child{"HeatFlux", steadyStateUFactors.heatFlux};
        node << FileParse::Child{"DefaultDisplayType", steadyStateUFactors.defaultDisplayType};
        // Vector is intentionally at the same level as the other nodes
        node << FileParse::Child{"Projection", steadyStateUFactors.projections};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, SteadyStateResultsCase & steadyStateResultsCase)
    {
        node >> FileParse::Child{"ModelType", steadyStateResultsCase.modelType};
        node >> FileParse::Child{"SimulationError", steadyStateResultsCase.simulationError};
        node >> FileParse::Child{"GlazingCase", steadyStateResultsCase.glazingCase};
        node >> FileParse::Child{"SpacerCase", steadyStateResultsCase.spacerCase};
        // Vector is intentionally at the same level as the other nodes
        node >> FileParse::Child{"U-factors", steadyStateResultsCase.steadyStateUFactors};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const SteadyStateResultsCase & steadyStateResultsCase)
    {
        node << FileParse::Child{"ModelType", steadyStateResultsCase.modelType};
        node << FileParse::Child{"SimulationError", steadyStateResultsCase.simulationError};
        node << FileParse::Child{"GlazingCase", steadyStateResultsCase.glazingCase};
        node << FileParse::Child{"SpacerCase", steadyStateResultsCase.spacerCase};
        // Vector is intentionally at the same level as the other nodes
        node << FileParse::Child{"U-factors", steadyStateResultsCase.steadyStateUFactors};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, SteadyStateResults & steadyStateResults)
    {
        // Vector is intentionally at the same level as the other nodes
        node >> FileParse::Child{"Version", steadyStateResults.version};
        node >> FileParse::Child{"Case", steadyStateResults.cases};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const SteadyStateResults & steadyStateResults)
    {
        // Vector is intentionally at the same level as the other nodes
        node << FileParse::Child{"Version", steadyStateResults.version};
        node << FileParse::Child{"Case", steadyStateResults.cases};

        return node;
    }
}   // namespace ThermFile