#include <stdexcept>

#include "Properties.hxx"

#include "LibraryUtilities/Common.hxx"

namespace ThermFile
{
    //! Section name in ini file that all properties will be read from and saved to.
    const std::string FilePropertiesSectionName{"File Properties"};

    ////////////////////////////////////////////////////////////////////////////////////
    /// General
    ////////////////////////////////////////////////////////////////////////////////////

    ConstantInitialConditions::ConstantInitialConditions(double temperature, double humidity) :
        temperature(temperature), humidity(humidity)
    {}
    ModelingOptions::ModelingOptions(bool excludeWaterLiquidTransportation,
                                     bool excludeHeatOfEvaporation,
                                     bool excludeCapillaryConduction,
                                     bool excludeVaporDiffusionConduction,
                                     bool excludeLatentHeatOfFusion,
                                     bool excludeThermalConductivityMoistureDependent,
                                     bool excludeThermalConductivityTemperatureDependent) :
        excludeWaterLiquidTransportation(excludeWaterLiquidTransportation),
        excludeHeatOfEvaporation(excludeHeatOfEvaporation),
        excludeCapillaryConduction(excludeCapillaryConduction),
        excludeVaporDiffusionConduction(excludeVaporDiffusionConduction),
        excludeLatentHeatOfFusion(excludeLatentHeatOfFusion),
        excludeThermalConductivityMoistureDependent(excludeThermalConductivityMoistureDependent),
        excludeThermalConductivityTemperatureDependent(excludeThermalConductivityTemperatureDependent)
    {}
    EngineParameters::EngineParameters(double convergenceTolerance,
                                       double relaxationParameter,
                                       double timeStep,
                                       size_t numberOfTimesteps) :
        convergenceTolerance(convergenceTolerance),
        relaxationParameter(relaxationParameter),
        timeStep(timeStep),
        numberOfTimesteps(numberOfTimesteps)
    {}
    MeshControl::MeshControl(MesherType meshType,
                             size_t meshParameter,
                             bool runErrorEstimator,
                             double errorEnergyNorm,
                             size_t maximumIterations) :
        meshType(meshType),
        meshParameter(meshParameter),
        runErrorEstimator(runErrorEstimator),
        errorEnergyNorm(errorEnergyNorm),
        maximumIterations(maximumIterations)
    {}
    MiscProperties::MiscProperties(bool radianceMode,
                                   bool useCRforGlazingSystem,
                                   bool checkForCorrectWINDOWBCOnGlazingSystem) :
        radianceMode(radianceMode),
        useCRforGlazingSystem(useCRforGlazingSystem),
        checkForCorrectWINDOWBCOnGlazingSystem(checkForCorrectWINDOWBCOnGlazingSystem)
    {}
    HeatTransferModelingOptions::HeatTransferModelingOptions(bool automaticallyAdjustRelaxationParameter,
                                                             double adjustmentStep,
                                                             size_t maximumIterations,
                                                             bool viewFactorSmoothing) :
        automaticallyAdjustRelaxationParameter(automaticallyAdjustRelaxationParameter),
        adjustmentStep(adjustmentStep),
        maximumIterations(maximumIterations),
        viewFactorSmoothing(viewFactorSmoothing)
    {}

    ModelOrientation::ModelOrientation(Orientation orientation, double cardinalOrientation) :
        orientation(orientation), cardinalOrientation(cardinalOrientation)
    {}

    General::General(std::string calcVersion,
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

    void General::setModifiedIsEqualToCreate()
    {
        lastModified = creationDate;
        lastModifiedVersion = creationVersion;
    }

    CalculationOptions::CalculationOptions(SimulationEngine simulationEngine,
                                           CalculationMode calculationMode,
                                           bool simulateMoisture,
                                           bool simulateThermal,
                                           ModelingOptions modelingOptions,
                                           TransientCalculationMethodology transientCalculationMethodology,
                                           SteadyStateCalculationMethodology steadyStateCalculationMethodology,
                                           ConstantInitialConditions constantInitialConditionsTransient,
                                           ConstantInitialConditions constantInitialConditionsSteadyState,
                                           EngineParameters engineParameters,
                                           MeshControl meshControl,
                                           HeatTransferModelingOptions heatTransferModelingOptions,
                                           MiscProperties miscProperties) :
        simulationEngine(simulationEngine),
        calculationMode(calculationMode),
        simulateMoisture(simulateMoisture),
        simulateThermal(simulateThermal),
        modelingOptions(modelingOptions),
        transientCalculationMethodology(transientCalculationMethodology),
        steadyStateCalculationMethodology(steadyStateCalculationMethodology),
        constantInitialConditionsTransient(constantInitialConditionsTransient),
        constantInitialConditionsSteadyState(constantInitialConditionsSteadyState),
        engineParameters(engineParameters),
        meshControl(meshControl),
        heatTransferModelingOptions(heatTransferModelingOptions),
        miscProperties(miscProperties)
    {}

    ExposureType::ExposureType(
      ModelPurpose modelPurpose,
      std::variant<WindowCrossSectionType, OpaqueCrossSectionType, OtherCrossSectionType> crossSection,
      std::optional<OpaqueAssemblyType> assemblyType) :
        modelPurpose(modelPurpose), crossSection(crossSection), assemblyType(assemblyType)
    {}

    ////////////////////////////////////////////////////////////////////////////////////
    /// ModelExposure
    ////////////////////////////////////////////////////////////////////////////////////

    ModelExposure::ModelExposure(double modelOrientation,
                                 Gravity::Orientation gravityOrientation,
                                 ExposureType exposureType) :
        modelOrientation(modelOrientation), gravityOrientation(gravityOrientation), exposureType(exposureType)
    {}

    void setGravityOrientation(const Geometry::GravityVector & vector, ModelExposure & exposure)
    {
        exposure.gravityOrientation = ThermFile::Gravity::getOrientation(vector);
    }

    Miscellaneous::Miscellaneous(double floatTolerance, double checkingTolerance) :
        floatTolerance(floatTolerance), checkingTolerance(checkingTolerance)
    {}
}   // namespace ThermFile
