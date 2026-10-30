#pragma once

#include <fileParse/Optional.hxx>
#include <fileParse/Variant.hxx>
#include <fileParse/Vector.hxx>

#include "EnumSerializers.hxx"
#include "LibraryUtilities/TimeRoutines.hxx"

namespace ThermFile
{
    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, UniformStart & start)
    {
        node >> FileParse::Child{"Temperature", start.temperature};
        node >> FileParse::Child{"Humidity", start.humidity};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const UniformStart & start)
    {
        node << FileParse::Child{"Temperature", start.temperature};
        node << FileParse::Child{"Humidity", start.humidity};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, StartCondition & condition)
    {
        node >> FileParse::Child{"Segment", condition.segment};
        node >> FileParse::Child{"BCUUID", condition.bcUUID};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const StartCondition & condition)
    {
        node << FileParse::Child{"Segment", condition.segment};
        node << FileParse::Child{"BCUUID", condition.bcUUID};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, StartPhysics & physics)
    {
        node >> FileParse::Child{"SimulateThermal", physics.simulateThermal};
        node >> FileParse::Child{"SimulateMoisture", physics.simulateMoisture};
        node >> FileParse::Child{"ModelingOptions", physics.modelingOptions};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const StartPhysics & physics)
    {
        node << FileParse::Child{"SimulateThermal", physics.simulateThermal};
        node << FileParse::Child{"SimulateMoisture", physics.simulateMoisture};
        node << FileParse::Child{"ModelingOptions", physics.modelingOptions};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, SteadyStart & start)
    {
        node >> FileParse::Child{"Start", start.start};
        node >> FileParse::Child{"Physics", start.physics};
        node >> FileParse::Child{"Condition", start.conditions};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const SteadyStart & start)
    {
        node << FileParse::Child{"Start", start.start};
        node << FileParse::Child{"Physics", start.physics};
        node << FileParse::Child{"Condition", start.conditions};

        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, TransientStart & start)
    {
        node >> FileParse::Child{"Start", start.start};
        node >> FileParse::Child{"Physics", start.physics};
        node >> FileParse::Child{"Condition", start.conditions};
        node >> FileParse::Child{"Step", start.stepLengths};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const TransientStart & start)
    {
        node << FileParse::Child{"Start", start.start};
        node << FileParse::Child{"Physics", start.physics};
        node << FileParse::Child{"Condition", start.conditions};
        node << FileParse::Child{"Step", start.stepLengths};

        return node;
    }

    //! The alternatives of InitialConditions, as the file names them.
    inline const std::vector<std::string> initialConditionsNodeNames{"Uniform", "FromSteadyState", "FromTransient"};

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, InitialConditions & initialConditions)
    {
        FileParse::deserializeVariant(node, initialConditionsNodeNames, initialConditions.state);

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const InitialConditions & initialConditions)
    {
        FileParse::serializeVariant(node, initialConditionsNodeNames, initialConditions.state);

        return node;
    }

    //! The initial conditions as written since Model.xml version 4, or converted from the
    //! two constant pairs of earlier files, whose methodologies were always "constant
    //! everywhere" and "no time variable": the pair of the file's calculation mode.
    template<typename NodeAdapter>
    inline void readInitialConditions(const NodeAdapter & node, CalculationOptions & options)
    {
        if(node.nChildNode("InitialConditions") > 0)
        {
            node >> FileParse::Child{"InitialConditions", options.initialConditions};
            return;
        }
        UniformStart transientStart;
        UniformStart steadyStart;
        node >> FileParse::Child{"ConstantInitialConditionsTransient", transientStart};
        node >> FileParse::Child{"ConstantInitialConditionsSteadyState", steadyStart};
        options.initialConditions.state =
          options.calculationMode == CalculationMode::cmSteadyState ? steadyStart : transientStart;
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

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const EngineParameters & parameters)
    {
        node << FileParse::Child{"ConvergenceTolerance", parameters.convergenceTolerance};
        node << FileParse::Child{"RelaxationParameter", parameters.relaxationParameter};

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
    inline const NodeAdapter & operator>>(const NodeAdapter & node, RadiationModelParameters & /*radiationModelParameters*/)
    {
        // Reserved slot; no fields enumerated yet.
        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const RadiationModelParameters & /*radiationModelParameters*/)
    {
        // Reserved slot; no fields enumerated yet.
        return node;
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, FrameCavityProperties & properties)
    {
        node >> FileParse::Child{"ConvectionModel", properties.convectionModel};
        node >> FileParse::Child{"RadiationCalculation", properties.radiationCalculation};
        node >> FileParse::Child{"RadiationModelParameters", properties.radiationModelParameters};

        return node;
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const FrameCavityProperties & properties)
    {
        node << FileParse::Child{"ConvectionModel", properties.convectionModel};
        node << FileParse::Child{"RadiationCalculation", properties.radiationCalculation};
        node << FileParse::Child{"RadiationModelParameters", properties.radiationModelParameters};

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
        readInitialConditions(node, options);
        node >> FileParse::Child{"EngineParameters", options.engineParameters};
        node >> FileParse::Child{"MeshControl", options.meshControl};
        node >> FileParse::Child{"HeatTransferModelingOptions", options.heatTransferModelingOptions};
        node >> FileParse::Child{"MiscellaneousProperties", options.miscProperties};
        node >> FileParse::Child{"FrameCavityProperties", options.frameCavityProperties};

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
        node << FileParse::Child{"InitialConditions", options.initialConditions};
        // Mirrors for readers of Model.xml version 3 (THERM 8.1), which take the constant
        // pair of their calculation mode and know no other methodology.
        const auto mirror{uniformStartOf(options.initialConditions)};
        node << FileParse::Child{"ConstantInitialConditionsTransient", mirror};
        node << FileParse::Child{"ConstantInitialConditionsSteadyState", mirror};
        node << FileParse::Child{"EngineParameters", options.engineParameters};
        node << FileParse::Child{"MeshControl", options.meshControl};
        node << FileParse::Child{"HeatTransferModelingOptions", options.heatTransferModelingOptions};
        node << FileParse::Child{"MiscellaneousProperties", options.miscProperties};
        node << FileParse::Child{"FrameCavityProperties", options.frameCavityProperties};

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