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
#include "THMZ/Mesh/Mesh.hxx"

namespace ThermFile
{
    //! \brief Defines initial condition values for case when constant values are used in either
    //! transient or steady-state conditions.
    struct ConstantInitialConditions
    {
        double temperature{21};
        double humidity{0.5};
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
        bool excludeWaterLiquidTransportation{true};
        bool excludeHeatOfEvaporation{true};
        bool excludeCapillaryConduction{true};
        bool excludeVaporDiffusionConduction{true};
        bool excludeLatentHeatOfFusion{true};
        bool excludeThermalConductivityMoistureDependent{true};
        bool excludeThermalConductivityTemperatureDependent{true};
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
        double convergenceTolerance{1e-5};
        double relaxationParameter{1.0};
        double timeStep{3600};
        size_t numberOfTimesteps{8670};
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
        MesherType meshType{MesherType::Simmetrix_Version_2022};
        size_t meshParameter{6u};
        bool runErrorEstimator{true};
        double errorEnergyNorm{10};
        size_t maximumIterations{5};
    };

    ////////////////////////////////////////////////////////////////////////////////////
    /// MiscProperties
    ////////////////////////////////////////////////////////////////////////////////////

    //! \brief Miscellaneous properties
    struct MiscProperties
    {
        bool radianceMode{false};
        bool useCRforGlazingSystem{false};
        bool checkForCorrectWINDOWBCOnGlazingSystem{false};
    };

    ////////////////////////////////////////////////////////////////////////////////////
    /// HeatTransferModelingOptions
    ////////////////////////////////////////////////////////////////////////////////////

    //! \brief Various heat transfer modeling options
    struct HeatTransferModelingOptions
    {
        bool automaticallyAdjustRelaxationParameter{true};
        double adjustmentStep{-0.01};
        size_t maximumIterations{25};
        bool viewFactorSmoothing{true};
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
        Orientation orientation{Orientation::North};
        double cardinalOrientation{0};
    };

    ////////////////////////////////////////////////////////////////////////////////////
    /// General
    ////////////////////////////////////////////////////////////////////////////////////

    //! \brief Class used to group General settings tab in Therm File Options screen.
    struct General
    {
        void setModifiedIsEqualToCreate();

        std::string calcVersion;
        std::string fileName;
        std::string directory;
        std::string creationDate{Timer::timeToString(std::chrono::system_clock::now())};
        std::string lastModified{Timer::timeToString(std::chrono::system_clock::now())};
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
        SimulationEngine simulationEngine{SimulationEngine::Conrad};
        CalculationMode calculationMode{CalculationMode::cmTransient};
        bool simulateMoisture{true};
        bool simulateThermal{true};
        ModelingOptions modelingOptions;
        TransientCalculationMethodology transientCalculationMethodology{TransientCalculationMethodology::icConstantEverywhere};
        SteadyStateCalculationMethodology steadyStateCalculationMethodology{SteadyStateCalculationMethodology::ssNoTimeVariable};
        ConstantInitialConditions constantInitialConditionsTransient;
        ConstantInitialConditions constantInitialConditionsSteadyState;
        EngineParameters engineParameters;
        MeshControl meshControl;
        HeatTransferModelingOptions heatTransferModelingOptions;
        MiscProperties miscProperties;
    };

    using CrossSectionType = std::variant<WindowCrossSectionType, OpaqueCrossSectionType, OtherCrossSectionType>;

    //! \brief Base class to keep data for assembly and cross-section type.
    struct ExposureType
    {
        ModelPurpose modelPurpose{ModelPurpose::Window_TransparentFacade};
        CrossSectionType crossSection{WindowCrossSectionType::Sill};
        std::optional<OpaqueAssemblyType> assemblyType{std::nullopt};
    };

    ////////////////////////////////////////////////////////////////////////////////////
    /// ModelExposure
    ////////////////////////////////////////////////////////////////////////////////////

    //! \brief Keeps all necessary data to used on Calculation Option tab in Therm File Properties screen
    struct ModelExposure
    {
        double modelOrientation{0.0};
        Gravity::Orientation gravityOrientation{Gravity::Orientation::Down};

        ExposureType exposureType;
    };

    void setGravityOrientation(const Geometry::GravityVector & vector, ModelExposure & modelExposure);

    struct Miscellaneous
    {
        double floatTolerance{0.02};
        double checkingTolerance{0.1};
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
