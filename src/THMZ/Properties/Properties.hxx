#pragma once

#include <optional>
#include <map>
#include <vector>
#include <string>
#include <variant>
#include <chrono>

#include "Enumerators.hxx"
#include "GravityProperties.hxx"

#include "Common/GeometryDefinitions.hxx"
#include "LibraryUtilities/TimeRoutines.hxx"

namespace ThermFile
{
    //! \brief Defines initial condition values for case when constant values are used in either
    //! transient or steady-state conditions.
    struct ConstantInitialConditions
    {
        inline static double DefaultTemperature{21};
        inline static double DefaultHumidity{0.5};

        ConstantInitialConditions() = default;

        double temperature{DefaultTemperature};
        double humidity{DefaultHumidity};
    };

    ////////////////////////////////////////////////////////////////////////////////////
    /// ModelingOptions
    ////////////////////////////////////////////////////////////////////////////////////

    //! \brief Certain parts of equations can be excluded from calculation.
    //!
    //! @param excludeWaterLiquidTransportation Defines whether or not water liquid transportation
    //! will be excluded from mass transfer equation
    //! @param excludeHeatOfEvaporation Defines whether or not heat of evaporation will be
    //! excluded from heat transfer equation
    //! @param excludeCapillaryConduction Defines whether or not capillary conduction will be
    //! excluded from heat transfer equation
    //! @param excludeVaporDiffusionConduction Defines whether or not vapor diffusion conduction
    //! will be excluded from heat transfer equation
    //! @param excludeLatentHeatOfFusion Defines whether or not latent heat of fusion will be
    //! excluded from heat transfer equation
    //! @param excludeThermalConductivityMoistureDependent Defines if thermal conductivity will be
    //! constant or moisture dependent
    //! @param excludeThermalConductivityTemperatureDependent Defines if thermal conductivity will be
    //! constant or temperature dependent
    struct ModelingOptions
    {
        // Defaults
        inline static bool DefaultExcludeWaterLiquidTransportation{true};
        inline static bool DefaultExcludeHeatOfEvaporation{true};
        inline static bool DefaultExcludeCapillaryConduction{true};
        inline static bool DefaultExcludeVaporDiffusionConduction{true};
        inline static bool DefaultExcludeLatentHeatOfFusion{true};
        inline static bool DefaultExcludeThermalConductivityMoistureDependent{true};
        inline static bool DefaultExcludeThermalConductivityTemperatureDependent{true};

        ModelingOptions() = default;

        bool excludeWaterLiquidTransportation{DefaultExcludeWaterLiquidTransportation};
        bool excludeHeatOfEvaporation{DefaultExcludeHeatOfEvaporation};
        bool excludeCapillaryConduction{DefaultExcludeCapillaryConduction};
        bool excludeVaporDiffusionConduction{DefaultExcludeVaporDiffusionConduction};
        bool excludeLatentHeatOfFusion{DefaultExcludeLatentHeatOfFusion};
        bool excludeThermalConductivityMoistureDependent{DefaultExcludeThermalConductivityMoistureDependent};
        bool excludeThermalConductivityTemperatureDependent{DefaultExcludeThermalConductivityTemperatureDependent};
    };

    ////////////////////////////////////////////////////////////////////////////////////
    /// EngineParameters
    ////////////////////////////////////////////////////////////////////////////////////

    //! \brief Basic iterations setup for simulation engine
    //!
    //! @param convergenceTolerance Defines convergence tolerance for the solution.
    //! @param relaxationParameter Defines relaxation parameter that will be used in solver's iterations.
    //! @param timeStep Defines timestep size in seconds
    //! @param numberOfTimesteps Defines number of timesteps to be used in the simulation
    struct EngineParameters
    {
        // Defaults
        inline static double DefaultConvergenceTolerance{1e-5};
        inline static double DefaultRelaxationParameter{1.0};
        inline static double DefaultTimeStep{3600};
        inline static size_t DefaultNumberOfTimesteps{8670};

        EngineParameters() = default;

        double convergenceTolerance{DefaultConvergenceTolerance};
        double relaxationParameter{DefaultRelaxationParameter};
        double timeStep{DefaultTimeStep};
        size_t numberOfTimesteps{DefaultNumberOfTimesteps};
    };

    ////////////////////////////////////////////////////////////////////////////////////
    /// MeshControl
    ////////////////////////////////////////////////////////////////////////////////////

    //! \brief Mesh control parameters
    //!
    //! @param meshType Defines type of mesher to be used
    //! @param runErrorEstimator Whether or not error estimator will be used.
    //! @param errorEnergyNorm Defines percentage for error energy norm.
    //! @param maximumIterations Number of iterations that will be used to change mesh parameters in
    //! order to achieve error norm.
    struct MeshControl
    {
        // Defaults
        inline static size_t DefaultMesherType{1u};
        inline static size_t DefaultMeshParameter{6};
        inline static bool DefaultRunErrorEstimator{true};
        inline static double DefaultErrorEnergyNorm{10};
        inline static size_t DefaultMaximumIterations{5};

        MeshControl() = default;

        MesherType meshType{MesherType::Simmetrix_Version_2022};
        size_t meshParameter{DefaultMeshParameter};
        bool runErrorEstimator{DefaultRunErrorEstimator};
        double errorEnergyNorm{DefaultErrorEnergyNorm};
        size_t maximumIterations{DefaultMaximumIterations};
    };

    ////////////////////////////////////////////////////////////////////////////////////
    /// MiscProperties
    ////////////////////////////////////////////////////////////////////////////////////

    //! \brief Miscellaneous properties
    struct MiscProperties
    {
        // Defaults
        inline static bool DefaultRadianceMode{false};
        inline static bool DefaultUseCRForGlazingSystem{false};
        inline static bool DefaultCheckForCorrectWINDOWBCOnGlazingSystem{false};

        MiscProperties() = default;

        bool radianceMode{DefaultRadianceMode};
        bool useCRforGlazingSystem{DefaultUseCRForGlazingSystem};
        bool checkForCorrectWINDOWBCOnGlazingSystem{DefaultCheckForCorrectWINDOWBCOnGlazingSystem};
    };

    ////////////////////////////////////////////////////////////////////////////////////
    /// HeatTransferModelingOptions
    ////////////////////////////////////////////////////////////////////////////////////

    //! \brief Various heat transfer modeling options
    struct HeatTransferModelingOptions
    {
        // Defaults
        inline static bool DefaultAutomaticallyAdjustRelaxationParameter{true};
        inline static double DefaultAdjustmentStep{-0.01};
        inline static size_t DefaultMaximumIterations{25};
        inline static bool DefaultViewFactorSmoothing{true};

        HeatTransferModelingOptions() = default;

        bool automaticallyAdjustRelaxationParameter{DefaultAutomaticallyAdjustRelaxationParameter};
        double adjustmentStep{DefaultAdjustmentStep};
        size_t maximumIterations{DefaultMaximumIterations};
        bool viewFactorSmoothing{DefaultViewFactorSmoothing};
    };

    ////////////////////////////////////////////////////////////////////////////////////
    /// ModelOrientation
    ////////////////////////////////////////////////////////////////////////////////////

    //! \brief Data about model orientation
    //!
    //! @param orientation Model orientation.
    //! @param cardinalOrientation Finer orientation setting.
    struct ModelOrientation
    {
        // Defaults
        inline static Orientation DefaultOrientation{Orientation::North};
        inline static double DefaultCardinalOrientation{0};

        ModelOrientation() = default;

        Orientation orientation{DefaultOrientation};
        double cardinalOrientation{DefaultCardinalOrientation};
    };

    ////////////////////////////////////////////////////////////////////////////////////
    /// General
    ////////////////////////////////////////////////////////////////////////////////////

    //! \brief Class used to group General settings tab in Therm File Options screen.
    struct General
    {
        // Defaults
        inline static std::string DefaultCreationDate{Timer::timeToString(std::chrono::system_clock::now())};
        inline static std::string DefaultLastModified{Timer::timeToString(std::chrono::system_clock::now())};

        General() = default;

        void setModifiedIsEqualToCreate();

        std::string calcVersion;
        std::string fileName;
        std::string directory;
        std::string creationDate{DefaultCreationDate};
        std::string lastModified{DefaultLastModified};
        std::string creationVersion;
        std::string lastModifiedVersion;
        std::string title;
        std::string createdBy;
        std::string company;
        std::string client;
        std::string notes;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    /// CalculationOptions
    ////////////////////////////////////////////////////////////////////////////////////

    struct CalculationOptions
    {
        // Defaults
        inline static SimulationEngine DefaultSimulationEngine{SimulationEngine::Conrad};
        inline static CalculationMode DefaultCalculationMode{CalculationMode::cmTransient};
        inline static bool DefaultSimulateMoisture{true};
        inline static bool DefaultSimulateThermal{true};
        inline static TransientCalculationMethodology DefaultTransientCalculationMethodology{
          TransientCalculationMethodology::icConstantEverywhere};
        inline static SteadyStateCalculationMethodology DefaultSteadyStateCalculationMethodology{
          SteadyStateCalculationMethodology::ssNoTimeVariable};

        CalculationOptions() = default;

        SimulationEngine simulationEngine{DefaultSimulationEngine};
        CalculationMode calculationMode{DefaultCalculationMode};
        bool simulateMoisture{DefaultSimulateMoisture};
        bool simulateThermal{DefaultSimulateThermal};
        ModelingOptions modelingOptions;
        TransientCalculationMethodology transientCalculationMethodology{DefaultTransientCalculationMethodology};
        SteadyStateCalculationMethodology steadyStateCalculationMethodology{DefaultSteadyStateCalculationMethodology};
        ConstantInitialConditions constantInitialConditionsTransient;
        ConstantInitialConditions constantInitialConditionsSteadyState;
        EngineParameters engineParameters;
        MeshControl meshControl;
        HeatTransferModelingOptions heatTransferModelingOptions;
        MiscProperties miscProperties;
    };

    //! \brief Base class to keep data for assembly and cross section type.
    struct ExposureType
    {
        ExposureType() = default;

        ExposureType(ModelPurpose modelPurpose,
                     std::variant<WindowCrossSectionType, OpaqueCrossSectionType, OtherCrossSectionType> crossSection,
                     std::optional<OpaqueAssemblyType> assemblyType = std::nullopt);

        ModelPurpose modelPurpose{ModelPurpose::Window_TransparentFacade};
        std::variant<WindowCrossSectionType, OpaqueCrossSectionType, OtherCrossSectionType> crossSection{
          WindowCrossSectionType::Sill};
        std::optional<OpaqueAssemblyType> assemblyType;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    /// ModelExposure
    ////////////////////////////////////////////////////////////////////////////////////

    //! \brief Keeps all necessary data to used on Calculation Option tab in Therm File Properties screen
    struct ModelExposure
    {
        // Defaults
        inline static Gravity::Orientation DefaultGravityOrientation{Gravity::Orientation::Down};
        inline static double DefaultModelOrientation{0.0};

        ModelExposure() = default;

        double modelOrientation{DefaultModelOrientation};
        Gravity::Orientation gravityOrientation{DefaultGravityOrientation};

        ExposureType exposureType{};
    };

    void setGravityOrientation(const Geometry::GravityVector & vector, ModelExposure & modelExposure);

    struct Miscellaneous
    {
        inline static double DefaultFloatTolerance{0.02};
        inline static double DefaultCheckingTolerance{0.1};

        Miscellaneous() = default;

        double floatTolerance{DefaultFloatTolerance};
        double checkingTolerance{DefaultCheckingTolerance};
    };

    ////////////////////////////////////////////////////////////////////////////////////
    /// FileProperties
    ////////////////////////////////////////////////////////////////////////////////////

    //! \brief Structure to hold all properties defined in THERM File Properties screen.
    struct Properties
    {
        General general;
        CalculationOptions calculationOptions;
        ModelExposure modelExposure;
        Miscellaneous miscellaneous;
    };

}   // namespace Therm
