#pragma once

#include <optional>
#include <map>
#include <vector>
#include <string>
#include <variant>
#include <chrono>

#include "Enumerators.hxx"
#include "GravityProperties.hxx"
#include "InitialConditions.hxx"
#include "ModelingOptions.hxx"

#include "Common/GeometryDefinitions.hxx"
#include "LibraryUtilities/TimeRoutines.hxx"
#include "THMZ/Mesh/Mesh.hxx"

namespace ThermFile
{
    ////////////////////////////////////////////////////////////////////////////////////
    /// EngineParameters
    ////////////////////////////////////////////////////////////////////////////////////

    //! \brief Basic iterations setup for simulation engine
    //!
    //! @param convergenceTolerance Defines convergence tolerance for the solution.
    //! @param relaxationParameter Defines relaxation parameter that will be used in solver's iterations.
    //!
    //! A transient run's step length and step count are not properties of the model: the
    //! time series datasets its boundaries bind set both.
    struct EngineParameters
    {
        double convergenceTolerance{1e-6};
        double relaxationParameter{1.0};
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

        [[nodiscard]] friend bool operator==(const MeshControl & lhs, const MeshControl & rhs) = default;
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
    /// RadiationModelParameters
    ////////////////////////////////////////////////////////////////////////////////////

    //! \brief Parameters for the detailed radiation model. Active only when
    //! FrameCavityProperties::radiationCalculation == RadiationCalculation::Detailed.
    //! Reserved as a slot; concrete fields (view-factor segment count, convergence
    //! tolerance, enclosure-detection threshold) are pending design decisions.
    struct RadiationModelParameters
    {
    };

    ////////////////////////////////////////////////////////////////////////////////////
    /// FrameCavityProperties
    ////////////////////////////////////////////////////////////////////////////////////

    //! \brief Project-wide frame-cavity policy: convection model + radiation
    //! calculation mode. Per-polygon attributes (gas, ventilated) live on
    //! Polygon::cavity; new cavities are initialised with hardcoded defaults
    //! ("Air", non-ventilated) at the conversion site.
    struct FrameCavityProperties
    {
        ConvectionModel convectionModel{ConvectionModel::ISO15099};
        RadiationCalculation radiationCalculation{RadiationCalculation::Detailed};
        RadiationModelParameters radiationModelParameters;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    /// CalculationOptions
    ////////////////////////////////////////////////////////////////////////////////////

    struct CalculationOptions
    {
        SimulationEngine simulationEngine{SimulationEngine::Conrad};
        CalculationMode calculationMode{CalculationMode::cmSteadyState};
        bool simulateMoisture{true};
        bool simulateThermal{true};
        ModelingOptions modelingOptions;
        InitialConditions initialConditions{};
        EngineParameters engineParameters;
        MeshControl meshControl;
        HeatTransferModelingOptions heatTransferModelingOptions;
        MiscProperties miscProperties;
        FrameCavityProperties frameCavityProperties;
    };

    using CrossSectionType = std::variant<WindowCrossSectionType, OpaqueCrossSectionType, OtherCrossSectionType>;

    //! \brief The assembly a cross section belongs to: a window assembly for a window or
    //! transparent facade, a wall or roof for an opaque facade, none for anything else.
    using AssemblyType = std::variant<WindowAssemblyType, OpaqueAssemblyType>;

    //! \brief Base class to keep data for assembly and cross-section type.
    struct ExposureType
    {
        ModelPurpose modelPurpose{ModelPurpose::Window_TransparentFacade};
        CrossSectionType crossSection{WindowCrossSectionType::Sill};
        std::optional<AssemblyType> assemblyType{std::nullopt};
    };

    //! \brief The building the cross section belongs to, in metres: one pair per file. Only
    //! the Montazeri convection model reads it, to pick its windward, leeward, side or roof
    //! coefficient from the model orientation and the wind direction.
    struct BuildingData
    {
        double width{20};
        double height{5};

        [[nodiscard]] friend bool operator==(const BuildingData & lhs, const BuildingData & rhs) = default;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    /// ModelDetails
    ////////////////////////////////////////////////////////////////////////////////////

    //! \brief Keeps all necessary data to used on Model Details tab in Therm File Properties screen
    struct ModelDetails
    {
        double modelOrientation{0.0};
        Gravity::Orientation gravityOrientation{Gravity::Orientation::Down};

        ExposureType exposureType;
        BuildingData building;
    };

    void setGravityOrientation(const Geometry::GravityVector & vector, ModelDetails & modelDetails);

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
        ModelDetails modelDetails;
        Miscellaneous miscellaneous;
    };

}   // namespace Therm
