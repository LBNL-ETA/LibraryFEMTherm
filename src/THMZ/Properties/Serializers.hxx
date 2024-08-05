#pragma once

#include <fileParse/Optional.hxx>
#include <fileParse/Variant.hxx>

#include "EnumSerializers.hxx"
#include "LibraryUtilities/TimeRoutines.hxx"

namespace ThermFile
{
    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, ConstantInitialConditions & initialConditions)
    {
        node >> FileParse::Child{"Temperature", initialConditions.temperature};
        node >> FileParse::Child{"Humidity", initialConditions.humidity};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const ConstantInitialConditions & initialConditions)
    {
        node << FileParse::Child{"Temperature", initialConditions.temperature};
        node << FileParse::Child{"Humidity", initialConditions.humidity};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, ModelingOptions & options)
    {
        node >> FileParse::Child{"ExcludeWaterLiquidTransportation", options.excludeWaterLiquidTransportation};
        node >> FileParse::Child{"ExcludeHeatOfEvaporation", options.excludeHeatOfEvaporation};
        node >> FileParse::Child{"ExcludeCapillaryConduction", options.excludeCapillaryConduction};
        node >> FileParse::Child{"ExcludeVaporDiffusionConduction", options.excludeVaporDiffusionConduction};
        node >> FileParse::Child{"ExcludeLatentHeatOfFusion", options.excludeLatentHeatOfFusion};
        node >> FileParse::Child{"ExcludeThermalConductivityMoistureDependent",
                             options.excludeThermalConductivityMoistureDependent};
        node >> FileParse::Child{"ExcludeThermalConductivityTemperatureDependent",
                             options.excludeThermalConductivityTemperatureDependent};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const ModelingOptions & options)
    {
        node << FileParse::Child{"ExcludeWaterLiquidTransportation", options.excludeWaterLiquidTransportation};
        node << FileParse::Child{"ExcludeHeatOfEvaporation", options.excludeHeatOfEvaporation};
        node << FileParse::Child{"ExcludeCapillaryConduction", options.excludeCapillaryConduction};
        node << FileParse::Child{"ExcludeVaporDiffusionConduction", options.excludeVaporDiffusionConduction};
        node << FileParse::Child{"ExcludeLatentHeatOfFusion", options.excludeLatentHeatOfFusion};
        node << FileParse::Child{"ExcludeThermalConductivityMoistureDependent",
                             options.excludeThermalConductivityMoistureDependent};
        node << FileParse::Child{"ExcludeThermalConductivityTemperatureDependent",
                             options.excludeThermalConductivityTemperatureDependent};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, EngineParameters & parameters)
    {
        node >> FileParse::Child{"ConvergenceTolerance", parameters.convergenceTolerance};
        node >> FileParse::Child{"RelaxationParameter", parameters.relaxationParameter};
        node >> FileParse::Child{"TimeStep", parameters.timeStep};
        node >> FileParse::Child{"NumberOfTimesteps", parameters.numberOfTimesteps};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const EngineParameters & parameters)
    {
        node << FileParse::Child{"ConvergenceTolerance", parameters.convergenceTolerance};
        node << FileParse::Child{"RelaxationParameter", parameters.relaxationParameter};
        node << FileParse::Child{"TimeStep", parameters.timeStep};
        node << FileParse::Child{"NumberOfTimesteps", parameters.numberOfTimesteps};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, MeshControl & meshControl)
    {
        node >> FileParse::Child{"MeshType", meshControl.meshType};
        node >> FileParse::Child{"MeshParameter", meshControl.meshParameter};
        node >> FileParse::Child{"RunErrorEstimator", meshControl.runErrorEstimator};
        node >> FileParse::Child{"ErrorEnergyNorm", meshControl.errorEnergyNorm};
        node >> FileParse::Child{"MaximumIterations", meshControl.maximumIterations};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const MeshControl & meshControl)
    {
        node << FileParse::Child{"MeshType", meshControl.meshType};
        node << FileParse::Child{"MeshParameter", meshControl.meshParameter};
        node << FileParse::Child{"RunErrorEstimator", meshControl.runErrorEstimator};
        node << FileParse::Child{"ErrorEnergyNorm", meshControl.errorEnergyNorm};
        node << FileParse::Child{"MaximumIterations", meshControl.maximumIterations};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, MiscProperties & properties)
    {
        node >> FileParse::Child{"RadianceMode", properties.radianceMode};
        node >> FileParse::Child{"UseCRForGlazingSystem", properties.useCRforGlazingSystem};
        node >> FileParse::Child{"CheckForCorrectBCOnGlazingSystem", properties.checkForCorrectWINDOWBCOnGlazingSystem};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const MiscProperties & properties)
    {
        node << FileParse::Child{"RadianceMode", properties.radianceMode};
        node << FileParse::Child{"UseCRForGlazingSystem", properties.useCRforGlazingSystem};
        node << FileParse::Child{"CheckForCorrectBCOnGlazingSystem", properties.checkForCorrectWINDOWBCOnGlazingSystem};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, HeatTransferModelingOptions & modelingOptions)
    {
        node >> FileParse::Child{"AutomaticallyAdjustRelaxationParameter",
                             modelingOptions.automaticallyAdjustRelaxationParameter};
        node >> FileParse::Child{"AdjustmentStep", modelingOptions.adjustmentStep};
        node >> FileParse::Child{"MaximumIterations", modelingOptions.maximumIterations};
        node >> FileParse::Child{"ViewFactorSmoothing", modelingOptions.viewFactorSmoothing};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const HeatTransferModelingOptions & modelingOptions)
    {
        node << FileParse::Child{"AutomaticallyAdjustRelaxationParameter",
                             modelingOptions.automaticallyAdjustRelaxationParameter};
        node << FileParse::Child{"AdjustmentStep", modelingOptions.adjustmentStep};
        node << FileParse::Child{"MaximumIterations", modelingOptions.maximumIterations};
        node << FileParse::Child{"ViewFactorSmoothing", modelingOptions.viewFactorSmoothing};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, ModelOrientation & orientation)
    {
        node >> FileParse::Child{"Orientation", orientation.orientation};
        node >> FileParse::Child{"CardinalOrientation", orientation.cardinalOrientation};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const ModelOrientation & orientation)
    {
        node << FileParse::Child{"Orientation", orientation.orientation};
        node << FileParse::Child{"CardinalOrientation", orientation.cardinalOrientation};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, General & general)
    {
        node >> FileParse::Child{"CalculationVersion", general.calcVersion};
        node >> FileParse::Child{"FileName", general.fileName};
        node >> FileParse::Child{"Directory", general.directory};
        node >> FileParse::Child{"CreationDate", general.creationDate};
        node >> FileParse::Child{"LastModified", general.lastModified};
        node >> FileParse::Child{"CreationVersion", general.creationVersion};
        node >> FileParse::Child{"LastModifiedVersion", general.lastModifiedVersion};
        node >> FileParse::Child{"Title", general.title};
        node >> FileParse::Child{"CreatedBy", general.createdBy};
        node >> FileParse::Child{"Company", general.company};
        node >> FileParse::Child{"Client", general.client};
        node >> FileParse::Child{"Notes", general.notes};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const General & general)
    {
        node << FileParse::Child{"CalculationVersion", general.calcVersion};
        node << FileParse::Child{"FileName", general.fileName};
        node << FileParse::Child{"Directory", general.directory};
        node << FileParse::Child{"CreationDate", general.creationDate};
        node << FileParse::Child{"LastModified", general.lastModified};
        node << FileParse::Child{"CreationVersion", general.creationVersion};
        node << FileParse::Child{"LastModifiedVersion", general.lastModifiedVersion};
        node << FileParse::Child{"Title", general.title};
        node << FileParse::Child{"CreatedBy", general.createdBy};
        node << FileParse::Child{"Company", general.company};
        node << FileParse::Child{"Client", general.client};
        node << FileParse::Child{"Notes", general.notes};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, CalculationOptions & options)
    {
        node >> FileParse::Child{"SimulationEngine", options.simulationEngine};
        node >> FileParse::Child{"CalculationMode", options.calculationMode};
        node >> FileParse::Child{"SimulateMoisture", options.simulateMoisture};
        node >> FileParse::Child{"SimulateThermal", options.simulateThermal};
        node >> FileParse::Child{"ModelingOptions", options.modelingOptions};
        node >> FileParse::Child{"TransientCalculationMethodology", options.transientCalculationMethodology};
        node >> FileParse::Child{"SteadyStateCalculationMethodology", options.steadyStateCalculationMethodology};
        node >> FileParse::Child{"ConstantInitialConditionsTransient", options.constantInitialConditionsTransient};
        node >> FileParse::Child{"ConstantInitialConditionsSteadyState", options.constantInitialConditionsSteadyState};
        node >> FileParse::Child{"EngineParameters", options.engineParameters};
        node >> FileParse::Child{"MeshControl", options.meshControl};
        node >> FileParse::Child{"HeatTransferModelingOptions", options.heatTransferModelingOptions};
        node >> FileParse::Child{"MiscellaneousProperties", options.miscProperties};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const CalculationOptions & options)
    {
        node << FileParse::Child{"SimulationEngine", options.simulationEngine};
        node << FileParse::Child{"CalculationMode", options.calculationMode};
        node << FileParse::Child{"SimulateMoisture", options.simulateMoisture};
        node << FileParse::Child{"SimulateThermal", options.simulateThermal};
        node << FileParse::Child{"ModelingOptions", options.modelingOptions};
        node << FileParse::Child{"TransientCalculationMethodology", options.transientCalculationMethodology};
        node << FileParse::Child{"SteadyStateCalculationMethodology", options.steadyStateCalculationMethodology};
        node << FileParse::Child{"ConstantInitialConditionsTransient", options.constantInitialConditionsTransient};
        node << FileParse::Child{"ConstantInitialConditionsSteadyState", options.constantInitialConditionsSteadyState};
        node << FileParse::Child{"EngineParameters", options.engineParameters};
        node << FileParse::Child{"MeshControl", options.meshControl};
        node << FileParse::Child{"HeatTransferModelingOptions", options.heatTransferModelingOptions};
        node << FileParse::Child{"MiscellaneousProperties", options.miscProperties};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, ExposureType & exposureType)
    {
        node >> FileParse::Child{"ModelPurpose", exposureType.modelPurpose};
        FileParse::deserializeVariant(
          node,
          {"WindowCrossSection", "OpaqueCrossSection", "OtherCrossSection"},
          exposureType.crossSection);
        node >> FileParse::Child{"Assembly", exposureType.assemblyType};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const ExposureType & exposureType)
    {
        node << FileParse::Child{"ModelPurpose", exposureType.modelPurpose};
        FileParse::serializeVariant(
          node,
          {"WindowCrossSection", "OpaqueCrossSection", "OtherCrossSection"},
          exposureType.crossSection);
        node << FileParse::Child{"Assembly", exposureType.assemblyType};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, ModelExposure & exposure)
    {
        node >> FileParse::Child{"ModelOrientation", exposure.modelOrientation};
        node >> FileParse::Child{"GravityOrientation", exposure.gravityOrientation};
        node >> FileParse::Child{"Exposure", exposure.exposureType};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const ModelExposure & exposure)
    {
        node << FileParse::Child{"ModelOrientation", exposure.modelOrientation};
        node << FileParse::Child{"GravityOrientation", exposure.gravityOrientation};
        node << FileParse::Child{"Exposure", exposure.exposureType};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, Miscellaneous & misc)
    {
        node >> FileParse::Child{"FloatTolerance", misc.floatTolerance};
        node >> FileParse::Child{"CheckingTolerance", misc.checkingTolerance};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const Miscellaneous & misc)
    {
        node << FileParse::Child{"FloatTolerance", misc.floatTolerance};
        node << FileParse::Child{"CheckingTolerance", misc.checkingTolerance};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, Properties & properties)
    {
        node >> FileParse::Child{"General", properties.general};
        node >> FileParse::Child{"CalculationOptions", properties.calculationOptions};
        node >> FileParse::Child{"ModelExposure", properties.modelExposure};
        node >> FileParse::Child{"Miscellaneous", properties.miscellaneous};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const Properties & properties)
    {
        node << FileParse::Child{"General", properties.general};
        node << FileParse::Child{"CalculationOptions", properties.calculationOptions};
        node << FileParse::Child{"ModelExposure", properties.modelExposure};
        node << FileParse::Child{"Miscellaneous", properties.miscellaneous};

        return node;
    }
}   // namespace Therm