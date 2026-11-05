#include "ConradTolerance.hxx"

#include <utility>

namespace ThermFile::Migration::ConradTolerance
{
    namespace
    {
        // Spelled out rather than taken from EngineParameters: this is the history of a
        // past release, and it stays the same if the default ever moves again.
        constexpr double applicationDefaultTolerance{1e-6};
    }   // namespace

    ThermFile::ThermModel applyToModel(ThermFile::ThermModel model)
    {
        auto & options{model.properties.calculationOptions};
        if(options.simulationEngine == ThermFile::SimulationEngine::Conrad)
        {
            options.engineParameters.convergenceTolerance = applicationDefaultTolerance;
        }

        return model;
    }
}   // namespace ThermFile::Migration::ConradTolerance
