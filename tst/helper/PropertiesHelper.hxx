#pragma once

#include "THMZ/Properties/Properties.hxx"

namespace Helper
{
    struct MockNode;

    void expect_eq(const ThermFile::ModelingOptions & expected, const ThermFile::ModelingOptions & actual);

    void expect_near(const ThermFile::ConstantInitialConditions & expected,
                     const ThermFile::ConstantInitialConditions & actual,
                     double tolerance);

    void expect_near(const ThermFile::EngineParameters & expected,
                     const ThermFile::EngineParameters & actual,
                     double tolerance);

    void expect_near(const ThermFile::MeshControl & expected, const ThermFile::MeshControl & actual, double tolerance);

    void expect_near(const ThermFile::HeatTransferModelingOptions & expected,
                     const ThermFile::HeatTransferModelingOptions & actual,
                     double tolerance);

    void expect_near(const ThermFile::ModelOrientation & expected,
                     const ThermFile::ModelOrientation & actual,
                     double tolerance);

    void expect_eq(const ThermFile::MiscProperties & expected, const ThermFile::MiscProperties & actual);

    void expect_near(const ThermFile::CalculationOptions & expected,
                     const ThermFile::CalculationOptions & actual,
                     double tolerance);

    void expect_eq(const ThermFile::General & expected, const ThermFile::General & actual);

    void expect_eq(const ThermFile::ExposureType & expected, const ThermFile::ExposureType & actual);

    void expect_eq(const ThermFile::ModelExposure & expected, const ThermFile::ModelExposure & actual);

    void
      expect_near(const ThermFile::Miscellaneous & expected, const ThermFile::Miscellaneous & actual, double tolerance);

    void expect_near(const ThermFile::Properties & expected, const ThermFile::Properties & actual, double tolerance);

    //////////////////////////////////////////////////////////////////////////////////////////
    //// Helper functions to fill the data
    //////////////////////////////////////////////////////////////////////////////////////////

    struct MockModelingOptions
    {
        std::string excludeWaterLiquidTransportation;
        std::string excludeHeatOfEvaporation;
        std::string excludeCapillaryConduction;
        std::string excludeVaporDiffusionConduction;
        std::string excludeLatentHeatOfFusion;
        std::string excludeThermalConductivityMoistureDependent;
        std::string excludeThermalConductivityTemperatureDependent;

        MockModelingOptions(std::string excludeWaterLiquidTransportation,
                            std::string excludeHeatOfEvaporation,
                            std::string excludeCapillaryConduction,
                            std::string excludeVaporDiffusionConduction,
                            std::string excludeLatentHeatOfFusion,
                            std::string excludeThermalConductivityMoistureDependent,
                            std::string excludeThermalConductivityTemperatureDependent);
    };

    //! \brief Creates ModelingOptions node.
    Helper::MockNode modelingOptionsNode(const MockModelingOptions & options);

    struct MockInitialConditions
    {
        std::string temperature;
        std::string humidity;

        MockInitialConditions(std::string temperature, std::string humidity);
    };

    //! \brief Creates initial conditions for transient state node.
    Helper::MockNode constantInitialConditionsTransientNode(const MockInitialConditions & initialConditions);

    //! \brief Inserts a ConstantInitialConditions node into the given mock node for the steady state case.
    Helper::MockNode constantInitialConditionsSteadyStateNode(const MockInitialConditions & initialConditions);

    struct MockEngineParameters
    {
        std::string convergenceTolerance;
        std::string relaxationParameter;
        std::string timeStep;
        std::string numberOfTimesteps;

        MockEngineParameters(std::string convergenceTolerance,
                             std::string relaxationParameter,
                             std::string timeStep,
                             std::string numberOfTimesteps);
    };

    //! \brief Inserts an EngineParameters node into the given mock node.
    Helper::MockNode engineParametersNode(const MockEngineParameters & engineParameters);

    struct MockMeshControl
    {
        std::string meshType;
        std::string meshParameter;
        std::string runErrorEstimator;
        std::string errorEnergyNorm;
        std::string maximumIterations;

        MockMeshControl(std::string meshType,
                        std::string meshParameter,
                        std::string runErrorEstimator,
                        std::string errorEnergyNorm,
                        std::string maximumIterations);
    };

    //! \brief Inserts a MeshControl node into the given mock node.
    Helper::MockNode meshControlNode(const MockMeshControl & meshControl);

    struct MockHeatTransferModelingOptions
    {
        std::string automaticallyAdjustRelaxationParameter;
        std::string adjustmentStep;
        std::string maximumIterations;
        std::string viewFactorSmoothing;

        MockHeatTransferModelingOptions(std::string automaticallyAdjustRelaxationParameter,
                                        std::string adjustmentStep,
                                        std::string maximumIterations,
                                        std::string viewFactorSmoothing);
    };

    //! \brief Creates heat transfer modeling options node.
    Helper::MockNode heatTransferModelingOptionsNode(const MockHeatTransferModelingOptions & options);

    struct MockModelOrientation
    {
        std::string orientation;
        std::string cardinalOrientation;

        MockModelOrientation(std::string orientation, std::string cardinalOrientation);
    };

    //! \brief Creates Model orientation node.
    Helper::MockNode modelOrientationNode(const MockModelOrientation & orientation);

    struct MockMiscProperties
    {
        std::string radianceMode;
        std::string useCRforGlazingSystem;
        std::string checkForCorrectWINDOWBCOnGlazingSystem;

        MockMiscProperties(std::string radianceMode,
                           std::string useCRforGlazingSystem,
                           std::string checkForCorrectWINDOWBCOnGlazingSystem);
    };

    //! \brief Inserts a MiscProperties node into the given mock node.
    Helper::MockNode miscPropertiesNode(const MockMiscProperties & properties);

    struct MockGeneral
    {
        std::string calcVersion;
        std::string fileName;
        std::string directory;
        std::string creationDate;
        std::string lastModified;
        std::string creationVersion;
        std::string lastModifiedVersion;
        std::string title;
        std::string createdBy;
        std::string company;
        std::string client;
        std::string notes;

        MockGeneral(std::string calcVersion,
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
                    std::string notes);
    };

    //! \brief Creates general node.
    Helper::MockNode generalNode(const MockGeneral & general);

    struct MockCalculationOptions
    {
        std::string simulationEngine;
        std::string calculationMode;
        std::string simulateMoisture;
        std::string simulateThermal;
        MockModelingOptions modelingOptions;
        std::string transientCalculationMethodology;
        std::string steadyStateCalculationMethodology;
        MockInitialConditions transientInitialConditions;
        MockInitialConditions steadyStateInitialConditions;
        MockEngineParameters engineParameters;
        MockMeshControl meshControl;
        MockHeatTransferModelingOptions heatTransferModelingOptions;
        MockMiscProperties miscProperties;

        MockCalculationOptions(std::string simulationEngine,
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
                               MockMiscProperties miscProperties);
    };

    Helper::MockNode calculationOptionsNode(const MockCalculationOptions & options);

    //! \brief Creates exposure type node for WINDOW cross section
    Helper::MockNode exposureTypeWindowNode(std::string_view crossSection);

    //! \brief Creates exposure type node for Opaque cross section.
    Helper::MockNode exposureTypeOpaqueNode(std::string_view crossSection, std::string_view assembly);

    //! \brief Creates exposure type node for Other cross section.
    Helper::MockNode exposureTypeOtherNode(std::string_view crossSection);

    struct MockModelExposure
    {
        std::string modelOrientation;
        std::string gravityOrientation;
        ThermFile::ModelPurpose modelPurpose;
        std::string crossSection;
        std::string assembly;

        MockModelExposure(std::string modelOrientation,
                          std::string gravityOrientation,
                          ThermFile::ModelPurpose modelPurpose,
                          std::string crossSection,
                          std::string assembly);
    };

    Helper::MockNode modelExposureNode(const MockModelExposure & exposure);

    struct MockMiscellaneous
    {
        std::string floatTolerance;
        std::string checkingTolerance;

        MockMiscellaneous(std::string floatTolerance, std::string checkingTolerance);
    };

    //! \brief Creates Miscellaneous properties node.
    Helper::MockNode miscellaneousNode(const MockMiscellaneous & miscellaneous);

    struct MockProperties
    {
        MockGeneral general;
        MockCalculationOptions calculationOptions;
        MockModelExposure modelExposure;
        MockMiscellaneous miscellaneous;
    };
}   // namespace Helper