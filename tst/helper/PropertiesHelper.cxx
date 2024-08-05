#include <gtest/gtest.h>

#include "PropertiesHelper.hxx"

#include "MockNodeAdapter.hxx"
#include "HelperUtility.hxx"

namespace Helper
{
    void expect_eq(const ThermFile::ModelingOptions & expected, const ThermFile::ModelingOptions & actual)
    {
        EXPECT_EQ(expected.excludeWaterLiquidTransportation, actual.excludeWaterLiquidTransportation);
        EXPECT_EQ(expected.excludeHeatOfEvaporation, actual.excludeHeatOfEvaporation);
        EXPECT_EQ(expected.excludeCapillaryConduction, actual.excludeCapillaryConduction);
        EXPECT_EQ(expected.excludeVaporDiffusionConduction, actual.excludeVaporDiffusionConduction);
        EXPECT_EQ(expected.excludeLatentHeatOfFusion, actual.excludeLatentHeatOfFusion);
        EXPECT_EQ(expected.excludeThermalConductivityMoistureDependent,
                  actual.excludeThermalConductivityMoistureDependent);
        EXPECT_EQ(expected.excludeThermalConductivityTemperatureDependent,
                  actual.excludeThermalConductivityTemperatureDependent);
    }

    void expect_near(const ThermFile::ConstantInitialConditions & expected,
                     const ThermFile::ConstantInitialConditions & actual,
                     double tolerance)
    {
        EXPECT_NEAR(expected.temperature, actual.temperature, tolerance);
        EXPECT_NEAR(expected.humidity, actual.humidity, tolerance);
    }

    void expect_near(const ThermFile::EngineParameters & expected,
                     const ThermFile::EngineParameters & actual,
                     double tolerance)
    {
        EXPECT_NEAR(expected.convergenceTolerance, actual.convergenceTolerance, tolerance);
        EXPECT_NEAR(expected.relaxationParameter, actual.relaxationParameter, tolerance);
        EXPECT_NEAR(expected.timeStep, actual.timeStep, tolerance);
        EXPECT_EQ(expected.numberOfTimesteps, actual.numberOfTimesteps);
    }

    void expect_near(const ThermFile::MeshControl & expected, const ThermFile::MeshControl & actual, double tolerance)
    {
        EXPECT_EQ(expected.meshType, actual.meshType);
        EXPECT_EQ(expected.meshParameter, actual.meshParameter);
        EXPECT_EQ(expected.runErrorEstimator, actual.runErrorEstimator);
        EXPECT_NEAR(expected.errorEnergyNorm, actual.errorEnergyNorm, tolerance);
        EXPECT_EQ(expected.maximumIterations, actual.maximumIterations);
    }

    void expect_near(const ThermFile::HeatTransferModelingOptions & expected,
                     const ThermFile::HeatTransferModelingOptions & actual,
                     double tolerance)
    {
        EXPECT_EQ(expected.automaticallyAdjustRelaxationParameter, actual.automaticallyAdjustRelaxationParameter);
        EXPECT_NEAR(expected.adjustmentStep, actual.adjustmentStep, tolerance);
        EXPECT_EQ(expected.maximumIterations, actual.maximumIterations);
        EXPECT_EQ(expected.viewFactorSmoothing, actual.viewFactorSmoothing);
    }

    void expect_near(const ThermFile::ModelOrientation & expected,
                     const ThermFile::ModelOrientation & actual,
                     double tolerance)
    {
        EXPECT_EQ(expected.orientation, actual.orientation);
        EXPECT_NEAR(expected.cardinalOrientation, actual.cardinalOrientation, tolerance);
    }

    void expect_eq(const ThermFile::MiscProperties & expected, const ThermFile::MiscProperties & actual)
    {
        EXPECT_EQ(expected.radianceMode, actual.radianceMode);
        EXPECT_EQ(expected.useCRforGlazingSystem, actual.useCRforGlazingSystem);
        EXPECT_EQ(expected.checkForCorrectWINDOWBCOnGlazingSystem, actual.checkForCorrectWINDOWBCOnGlazingSystem);
    }

    void expect_near(const ThermFile::CalculationOptions & expected,
                     const ThermFile::CalculationOptions & actual,
                     double tolerance)
    {
        EXPECT_EQ(expected.simulationEngine, actual.simulationEngine);
        EXPECT_EQ(expected.calculationMode, actual.calculationMode);
        EXPECT_EQ(expected.simulateMoisture, actual.simulateMoisture);
        EXPECT_EQ(expected.simulateThermal, actual.simulateMoisture);
        EXPECT_EQ(expected.transientCalculationMethodology, actual.transientCalculationMethodology);
        EXPECT_EQ(expected.steadyStateCalculationMethodology, actual.steadyStateCalculationMethodology);
        expect_eq(expected.modelingOptions, actual.modelingOptions);
        expect_near(expected.constantInitialConditionsTransient, actual.constantInitialConditionsTransient, tolerance);
        expect_near(
          expected.constantInitialConditionsSteadyState, actual.constantInitialConditionsSteadyState, tolerance);
        expect_near(expected.engineParameters, actual.engineParameters, tolerance);
        expect_near(expected.meshControl, actual.meshControl, tolerance);
        expect_near(expected.heatTransferModelingOptions, actual.heatTransferModelingOptions, tolerance);
        expect_eq(expected.miscProperties, actual.miscProperties);
    }

    void expect_eq(const ThermFile::General & expected, const ThermFile::General & actual)
    {
        EXPECT_EQ(expected.calcVersion, actual.calcVersion);
        EXPECT_EQ(expected.fileName, actual.fileName);
        EXPECT_EQ(expected.directory, actual.directory);
        EXPECT_EQ(expected.creationDate, actual.creationDate);
        EXPECT_EQ(expected.lastModified, actual.lastModified);
        EXPECT_EQ(expected.creationVersion, actual.creationVersion);
        EXPECT_EQ(expected.lastModifiedVersion, actual.lastModifiedVersion);
        EXPECT_EQ(expected.title, actual.title);
        EXPECT_EQ(expected.createdBy, actual.createdBy);
        EXPECT_EQ(expected.company, actual.company);
        EXPECT_EQ(expected.client, actual.client);
        EXPECT_EQ(expected.notes, actual.notes);
    }

    void expect_eq(const ThermFile::ExposureType & expected, const ThermFile::ExposureType & actual)
    {
        EXPECT_EQ(expected.modelPurpose, actual.modelPurpose);
        EXPECT_EQ(expected.crossSection, actual.crossSection);
        ASSERT_EQ(expected.assemblyType.has_value(), actual.assemblyType.has_value());
        if(expected.assemblyType.has_value() && actual.assemblyType.has_value())
        {
            EXPECT_EQ(expected.assemblyType.value(), actual.assemblyType.value());
        }
    }

    void expect_eq(const ThermFile::ModelExposure & expected, const ThermFile::ModelExposure & actual)
    {
        EXPECT_EQ(expected.modelOrientation, actual.modelOrientation);
        EXPECT_EQ(expected.gravityOrientation, actual.gravityOrientation);
        expect_eq(expected.exposureType, actual.exposureType);
    }

    void
      expect_near(const ThermFile::Miscellaneous & expected, const ThermFile::Miscellaneous & actual, double tolerance)
    {
        EXPECT_NEAR(expected.floatTolerance, actual.floatTolerance, tolerance);
        EXPECT_NEAR(expected.checkingTolerance, actual.checkingTolerance, tolerance);
    }

    void expect_near(const ThermFile::Properties & expected, const ThermFile::Properties & actual, double tolerance)
    {
        expect_eq(expected.general, actual.general);
        expect_near(expected.calculationOptions, actual.calculationOptions, tolerance);
        expect_eq(expected.modelExposure, actual.modelExposure);
        expect_near(expected.miscellaneous, actual.miscellaneous, tolerance);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //// Helper functions to fill the data
    //////////////////////////////////////////////////////////////////////////////////////////

    MockModelingOptions::MockModelingOptions(std::string excludeWaterLiquidTransportation,
                                             std::string excludeHeatOfEvaporation,
                                             std::string excludeCapillaryConduction,
                                             std::string excludeVaporDiffusionConduction,
                                             std::string excludeLatentHeatOfFusion,
                                             std::string excludeThermalConductivityMoistureDependent,
                                             std::string excludeThermalConductivityTemperatureDependent) :
        excludeWaterLiquidTransportation(std::move(excludeWaterLiquidTransportation)),
        excludeHeatOfEvaporation(std::move(excludeHeatOfEvaporation)),
        excludeCapillaryConduction(std::move(excludeCapillaryConduction)),
        excludeVaporDiffusionConduction(std::move(excludeVaporDiffusionConduction)),
        excludeLatentHeatOfFusion(std::move(excludeLatentHeatOfFusion)),
        excludeThermalConductivityMoistureDependent(std::move(excludeThermalConductivityMoistureDependent)),
        excludeThermalConductivityTemperatureDependent(std::move(excludeThermalConductivityTemperatureDependent))
    {}

    Helper::MockNode modelingOptionsNode(const MockModelingOptions & options)
    {
        Helper::MockNode node{"ModelingOptions"};
        addChildNode(node, "ExcludeWaterLiquidTransportation", options.excludeWaterLiquidTransportation);
        addChildNode(node, "ExcludeHeatOfEvaporation", options.excludeHeatOfEvaporation);
        addChildNode(node, "ExcludeCapillaryConduction", options.excludeCapillaryConduction);
        addChildNode(node, "ExcludeVaporDiffusionConduction", options.excludeVaporDiffusionConduction);
        addChildNode(node, "ExcludeLatentHeatOfFusion", options.excludeLatentHeatOfFusion);
        addChildNode(
          node, "ExcludeThermalConductivityMoistureDependent", options.excludeThermalConductivityMoistureDependent);
        addChildNode(node,
                     "ExcludeThermalConductivityTemperatureDependent",
                     options.excludeThermalConductivityTemperatureDependent);

        return node;
    }

    MockInitialConditions::MockInitialConditions(std::string temperature, std::string humidity) :
        temperature(std::move(temperature)), humidity(std::move(humidity))
    {}

    Helper::MockNode constantInitialConditionsTransientNode(const MockInitialConditions & initialConditions)
    {
        MockNode node("ConstantInitialConditionsTransient");
        addChildNode(node, "Temperature", initialConditions.temperature);
        addChildNode(node, "Humidity", initialConditions.humidity);

        return node;
    }

    Helper::MockNode constantInitialConditionsSteadyStateNode(const MockInitialConditions & initialConditions)
    {
        Helper::MockNode node{"ConstantInitialConditionsSteadyState"};
        addChildNode(node, "Temperature", initialConditions.temperature);
        addChildNode(node, "Humidity", initialConditions.humidity);

        return node;
    }

    MockEngineParameters::MockEngineParameters(std::string convergenceTolerance,
                                               std::string relaxationParameter,
                                               std::string timeStep,
                                               std::string numberOfTimesteps) :
        convergenceTolerance(std::move(convergenceTolerance)),
        relaxationParameter(std::move(relaxationParameter)),
        timeStep(std::move(timeStep)),
        numberOfTimesteps(std::move(numberOfTimesteps))
    {}

    Helper::MockNode engineParametersNode(const MockEngineParameters & engineParameters)
    {
        MockNode node{"EngineParameters"};
        addChildNode(node, "ConvergenceTolerance", engineParameters.convergenceTolerance);
        addChildNode(node, "RelaxationParameter", engineParameters.relaxationParameter);
        addChildNode(node, "TimeStep", engineParameters.timeStep);
        addChildNode(node, "NumberOfTimesteps", engineParameters.numberOfTimesteps);

        return node;
    }

    MockMeshControl::MockMeshControl(std::string meshType,
                                     std::string meshParameter,
                                     std::string runErrorEstimator,
                                     std::string errorEnergyNorm,
                                     std::string maximumIterations) :
        meshType(std::move(meshType)),
        meshParameter(std::move(meshParameter)),
        runErrorEstimator(std::move(runErrorEstimator)),
        errorEnergyNorm(std::move(errorEnergyNorm)),
        maximumIterations(std::move(maximumIterations))
    {}

    Helper::MockNode meshControlNode(const MockMeshControl & meshControl)
    {
        MockNode node{"MeshControl"};
        addChildNode(node, "MeshType", meshControl.meshType);
        addChildNode(node, "MeshParameter", meshControl.meshParameter);
        addChildNode(node, "RunErrorEstimator", meshControl.runErrorEstimator);
        addChildNode(node, "ErrorEnergyNorm", meshControl.errorEnergyNorm);
        addChildNode(node, "MaximumIterations", meshControl.maximumIterations);

        return node;
    }

    MockHeatTransferModelingOptions::MockHeatTransferModelingOptions(std::string automaticallyAdjustRelaxationParameter,
                                                                     std::string adjustmentStep,
                                                                     std::string maximumIterations,
                                                                     std::string viewFactorSmoothing) :
        automaticallyAdjustRelaxationParameter(std::move(automaticallyAdjustRelaxationParameter)),
        adjustmentStep(std::move(adjustmentStep)),
        maximumIterations(std::move(maximumIterations)),
        viewFactorSmoothing(std::move(viewFactorSmoothing))
    {}

    Helper::MockNode heatTransferModelingOptionsNode(const MockHeatTransferModelingOptions & options)
    {
        Helper::MockNode node{"HeatTransferModelingOptions"};
        addChildNode(node, "AutomaticallyAdjustRelaxationParameter", options.automaticallyAdjustRelaxationParameter);
        addChildNode(node, "AdjustmentStep", options.adjustmentStep);
        addChildNode(node, "MaximumIterations", options.maximumIterations);
        addChildNode(node, "ViewFactorSmoothing", options.viewFactorSmoothing);
        return node;
    }

    MockModelOrientation::MockModelOrientation(std::string orientation, std::string cardinalOrientation) :
        orientation(std::move(orientation)), cardinalOrientation(std::move(cardinalOrientation))
    {}

    Helper::MockNode modelOrientationNode(const MockModelOrientation & orientation)
    {
        Helper::MockNode node{"ModelOrientation"};
        addChildNode(node, "Orientation", orientation.orientation);
        addChildNode(node, "CardinalOrientation", orientation.cardinalOrientation);

        return node;
    }

    MockMiscProperties::MockMiscProperties(std::string radianceMode,
                                           std::string useCRforGlazingSystem,
                                           std::string checkForCorrectWINDOWBCOnGlazingSystem) :
        radianceMode(std::move(radianceMode)),
        useCRforGlazingSystem(std::move(useCRforGlazingSystem)),
        checkForCorrectWINDOWBCOnGlazingSystem(std::move(checkForCorrectWINDOWBCOnGlazingSystem))
    {}

    Helper::MockNode miscPropertiesNode(const MockMiscProperties & properties)
    {
        Helper::MockNode node{"MiscellaneousProperties"};
        addChildNode(node, "RadianceMode", properties.radianceMode);
        addChildNode(node, "UseCRForGlazingSystem", properties.useCRforGlazingSystem);
        addChildNode(node, "CheckForCorrectBCOnGlazingSystem", properties.checkForCorrectWINDOWBCOnGlazingSystem);

        return node;
    }

    MockGeneral::MockGeneral(std::string calcVersion,
                             std::string fileName,
                             std::string directory,
                             std::string creationDate,
                             std::string lastModified,
                             std::string creationVersion,
                             std::string lastModifiedVersion,
                             std::string title,
                             std::string createdBy,
                             std::string company,
                             std::string client,
                             std::string notes) :
        calcVersion(std::move(calcVersion)),
        fileName(std::move(fileName)),
        directory(std::move(directory)),
        creationDate(std::move(creationDate)),
        lastModified(std::move(lastModified)),
        creationVersion(std::move(creationVersion)),
        lastModifiedVersion(std::move(lastModifiedVersion)),
        title(std::move(title)),
        createdBy(std::move(createdBy)),
        company(std::move(company)),
        client(std::move(client)),
        notes(std::move(notes))
    {}

    Helper::MockNode generalNode(const MockGeneral & general)
    {
        Helper::MockNode node{"General"};
        addChildNode(node, "CalculationVersion", general.calcVersion);
        addChildNode(node, "FileName", general.fileName);
        addChildNode(node, "Directory", general.directory);
        addChildNode(node, "CreationDate", general.creationDate);
        addChildNode(node, "LastModified", general.lastModified);
        addChildNode(node, "CreationVersion", general.creationVersion);
        addChildNode(node, "LastModifiedVersion", general.lastModifiedVersion);
        addChildNode(node, "Title", general.title);
        addChildNode(node, "CreatedBy", general.createdBy);
        addChildNode(node, "Company", general.company);
        addChildNode(node, "Client", general.client);
        addChildNode(node, "Notes", general.notes);

        return node;
    }

    MockCalculationOptions::MockCalculationOptions(std::string simulationEngine,
                                                   std::string calculationMode,
                                                   std::string simulateMoisture,
                                                   std::string simulateThermal,
                                                   MockModelingOptions modelingOptions,
                                                   std::string transientCalculationMethodology,
                                                   std::string steadyStateCalculationMethodology,
                                                   MockInitialConditions transientInitialConditions,
                                                   MockInitialConditions steadyStateInitialConditions,
                                                   MockEngineParameters engineParameters,
                                                   MockMeshControl meshControl,
                                                   MockHeatTransferModelingOptions heatTransferModelingOptions,
                                                   MockMiscProperties miscProperties) :
        simulationEngine(std::move(simulationEngine)),
        calculationMode(std::move(calculationMode)),
        simulateMoisture(std::move(simulateMoisture)),
        simulateThermal(std::move(simulateThermal)),
        modelingOptions(std::move(modelingOptions)),
        transientCalculationMethodology(std::move(transientCalculationMethodology)),
        steadyStateCalculationMethodology(std::move(steadyStateCalculationMethodology)),
        transientInitialConditions(std::move(transientInitialConditions)),
        steadyStateInitialConditions(std::move(steadyStateInitialConditions)),
        engineParameters(std::move(engineParameters)),
        meshControl(std::move(meshControl)),
        heatTransferModelingOptions(std::move(heatTransferModelingOptions)),
        miscProperties(std::move(miscProperties))
    {}

    Helper::MockNode calculationOptionsNode(const MockCalculationOptions & options)
    {
        Helper::MockNode node{"CalculationOptions"};
        addChildNode(node, "SimulationEngine", options.simulationEngine);
        addChildNode(node, "CalculationMode", options.calculationMode);
        addChildNode(node, "SimulateMoisture", options.simulateMoisture);
        addChildNode(node, "SimulateThermal", options.simulateThermal);
        addChildNode(node, modelingOptionsNode(options.modelingOptions));
        addChildNode(node, "TransientCalculationMethodology", options.transientCalculationMethodology);
        addChildNode(node, "SteadyStateCalculationMethodology", options.steadyStateCalculationMethodology);
        addChildNode(node, constantInitialConditionsTransientNode(options.transientInitialConditions));
        addChildNode(node, constantInitialConditionsSteadyStateNode(options.steadyStateInitialConditions));
        addChildNode(node, engineParametersNode(options.engineParameters));
        addChildNode(node, meshControlNode(options.meshControl));
        addChildNode(node, heatTransferModelingOptionsNode(options.heatTransferModelingOptions));
        addChildNode(node, miscPropertiesNode(options.miscProperties));

        return node;
    }

    Helper::MockNode exposureTypeWindowNode(std::string_view crossSection)
    {
        Helper::MockNode node{"Exposure"};
        addChildNode(node, "ModelPurpose", "Window/Transparent Facade");
        addChildNode(node, "WindowCrossSection", crossSection);

        return node;
    }

    Helper::MockNode exposureTypeOpaqueNode(std::string_view crossSection, std::string_view assembly)
    {
        Helper::MockNode node{"Exposure"};
        addChildNode(node, "ModelPurpose", "Opaque Facade");
        addChildNode(node, "OpaqueCrossSection", crossSection);
        addChildNode(node, "Assembly", assembly);

        return node;
    }

    Helper::MockNode exposureTypeOtherNode(std::string_view crossSection)
    {
        Helper::MockNode node{"Exposure"};
        addChildNode(node, "ModelPurpose", "Other");
        addChildNode(node, "OtherCrossSection", crossSection);

        return node;
    }

    MockModelExposure::MockModelExposure(std::string modelOrientation,
                                         std::string gravityOrientation,
                                         ThermFile::ModelPurpose modelPurpose,
                                         std::string crossSection,
                                         std::string assembly) :
        modelOrientation(std::move(modelOrientation)),
        gravityOrientation(std::move(gravityOrientation)),
        modelPurpose(modelPurpose),
        crossSection(std::move(crossSection)),
        assembly(std::move(assembly))
    {}

    Helper::MockNode modelExposureNode(const MockModelExposure & exposure)
    {
        Helper::MockNode node{"ModelExposure"};
        // Add ModelExposure specific properties
        addChildNode(node, "ModelOrientation", exposure.modelOrientation);
        addChildNode(node, "GravityOrientation", exposure.gravityOrientation);

        // Based on ModelPurpose, create the appropriate ExposureType node
        Helper::MockNode exposureNode{"Exposure"};
        switch(exposure.modelPurpose)
        {
            case ThermFile::ModelPurpose::Window_TransparentFacade:
                exposureNode = exposureTypeWindowNode(exposure.crossSection);
                break;
            case ThermFile::ModelPurpose::OpaqueFacade:
                if(exposure.assembly.empty())
                {
                    // Handle error or default case
                    throw std::invalid_argument("Assembly required for OpaqueFacade");
                }
                exposureNode = exposureTypeOpaqueNode(exposure.crossSection, exposure.assembly);
                break;
            case ThermFile::ModelPurpose::Other:
                exposureNode = exposureTypeOtherNode(exposure.crossSection);
                break;
            default:
                throw std::invalid_argument("Invalid ModelPurpose");
        }

        addChildNode(node, exposureNode);

        return node;
    }

    MockMiscellaneous::MockMiscellaneous(std::string floatTolerance, std::string checkingTolerance) :
        floatTolerance(std::move(floatTolerance)), checkingTolerance(std::move(checkingTolerance))
    {}

    Helper::MockNode miscellaneousNode(const MockMiscellaneous & miscellaneous)
    {
        Helper::MockNode node{"Miscellaneous"};
        addChildNode(node, "FloatTolerance", miscellaneous.floatTolerance);
        addChildNode(node, "CheckingTolerance", miscellaneous.checkingTolerance);

        return node;
    }
}   // namespace Helper