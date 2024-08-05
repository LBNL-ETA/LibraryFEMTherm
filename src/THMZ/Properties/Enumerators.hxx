#pragma once

#include <map>
#include <vector>
#include <string>

namespace ThermFile
{
    enum class ModelPurpose
    {
        Unknown,
        Window_TransparentFacade,
        OpaqueFacade,
        Other
    };

    //! \brief Returns all available model purposes
    extern std::vector<std::string> modelPurposes();

    //! \brief Returns model purpose from string
    //! \param value String representation of model purpose
    //! \return Model purpose
    ModelPurpose modelPurposeFromString(std::string_view value);

    //! \brief Returns string representation of model purpose
    //! \param modelPurpose Model purpose
    //! \return String representation of model purpose
    std::string modelPurposeToString(ModelPurpose modelPurpose);

    //! \brief Available simulation engines
    enum class SimulationEngine
    {
        Unknown,
        Conrad,
        HygroThermFEM
    };

    //! \brief Returns all available simulation engines
    //! \return Vector of strings
    SimulationEngine simulationEngineFromString(std::string_view value);

    //! \brief Returns string representation of simulation engine
    //! \param simulationEngine Simulation engine
    std::string simulationEngineToString(SimulationEngine simulationEngine);

    //! \brief Defines if calculation mode is transient or steady-state.
    //!
    //! @param cmTransient File is for transient calculation mode.
    //! @param cmSteadyState File is for steady-state calculation mode.
    enum class CalculationMode
    {
        Unknown,
        cmTransient,
        cmSteadyState
    };

    //! \brief Returns calculation mode from string
    //! \return Calculation mode
    CalculationMode calculationModeFromString(std::string_view value);

    //! \brief Returns string representation of calculation mode
    //! \param calculationMode Calculation mode
    //! \return String representation of calculation mode
    std::string calculationModeToString(CalculationMode calculationMode);

    //! \brief Defines how transient initial conditions will be applied to the entire domain.
    //!
    //! @param icValuesFromInputFile Initial condition will be read from input file.
    //! @param icUseFirstTimestep Initial condition will be calculated from steady-state case when
    //! applying boundary conditions from first timestep.
    //! @param icConstantEverywhere Initial conditions will be set to constant everywhere by user.
    enum class TransientCalculationMethodology
    {
        Unknown,
        icValuesFromInputFile,
        icUseFirstTimestep,
        icConstantEverywhere
    };

    //! \brief Returns transient calculation methodology from string
    //! \return Transient calculation methodology
    TransientCalculationMethodology transientCalculationMethodologyFromString(std::string_view value);

    //! \brief Returns string representation of transient calculation methodology
    //! \param transientCalculationMethodology Transient calculation methodology
    std::string
      transientCalculationMethodologyToString(TransientCalculationMethodology transientCalculationMethodology);

    enum class MesherType
    {
        Unknown,
        QuadTreeMesher,
        Simmetrix_Version_2022
    };

    //! \brief Returns mesher type from string
    //! \return Mesher type
    MesherType mesherTypeFromString(std::string_view value);

    //! \brief Returns string representation of mesher type
    //! \param mesherType Mesher type
    std::string mesherTypeToString(MesherType mesherType);

    //! \brief Steady-state can be calculated by using true steady-state equations from the engine.
    //! \brief However it is possible that user wants to calculate steady-state by running transient
    //! \brief simulation for certain number of timesteps.
    //!
    //! @param ssNoTimeVariable No time variable is applied. True steady-state calculations are applied.
    //! @param ssFalseTransient Transient approach will be used to calculate steady state solution.
    enum class SteadyStateCalculationMethodology
    {
        Unknown,
        ssNoTimeVariable,
        ssFalseTransient
    };

    //! \brief Returns steady-state calculation methodology from string
    //! \return Steady-state calculation methodology
    SteadyStateCalculationMethodology steadyStateCalculationMethodologyFromString(std::string_view value);

    //! \brief Returns string representation of steady-state calculation methodology
    //! \param steadyStateCalculationMethodology Steady-state calculation methodology
    std::string
      steadyStateCalculationMethodologyToString(SteadyStateCalculationMethodology steadyStateCalculationMethodology);

    //! \brief Simple orientation enumerator
    enum class Orientation
    {
        Unknown,
        North,
        East,
        South,
        West
    };

    //! \brief Returns orientation from string
    //! \param value String representation of orientation
    //! \return Orientation
    Orientation orientationFromString(std::string_view value);

    //! \brief Returns string representation of orientation
    //! \param orientation Orientation
    //! \return String representation of orientation
    std::string orientationToString(Orientation orientation);

    enum class WindowCrossSectionType
    {
        Unknown,
        Sill = 1,
        Jamb,
        Head,
        HorizontalDivider,
        VerticalDivider,
        HorizontalMeetingRail,
        VerticalMeetingRail,
        CommonFrame,
        Spacer
    };

    //! \brief Checks if given window cross section type is jamb type
    //! \param type Window cross section type
    //! \return True if type is jamb type
    bool isJambType(WindowCrossSectionType type);

    //! \brief Returns window cross section type from string
    std::vector<std::string> windowCrossSectionStrings();

    //! \brief Returns window cross section type from string
    //! \param value String representation of window cross section type
    //! \return Window cross section type
    WindowCrossSectionType windowCrossSectionTypeFromString(std::string_view value);

    //! \brief Returns string representation of window cross section type
    //! \param type Window cross section type
    //! \return String representation of window cross section type
    std::string windowCrossSectionTypeToString(WindowCrossSectionType type);

    enum class OpaqueAssemblyType
    {
        Unknown,
        Wall,
        Roof
    };

    //! \brief Returns all available opaque assembly types
    std::vector<std::string> opaqueAssemblyTypeStrings();

    //! \brief Returns opaque assembly type from string
    //! \param value String representation of opaque assembly type
    //! \return Opaque assembly type
    OpaqueAssemblyType opaqueAssemblyTypeFromString(std::string_view value);

    //! \brief Returns string representation of opaque assembly type
    //! \param opaqueAssemblyType Opaque assembly type
    //! \return String representation of opaque assembly type
    std::string opaqueAssemblyTypeToString(OpaqueAssemblyType opaqueAssemblyType);

    enum class OpaqueCrossSectionType
    {
        Unknown,
        SillPlate,
        Header,
        EndSection,
        MiddleSection,
        ThermalBridge,
        WindowFramingSill,
        RoughOpeningHeader,
        RoughOpeningJamb
    };

    //! \brief Returns all available opaque cross section types
    std::vector<std::string> opaqueCrossSectionStrings();

    //! \brief Returns opaque cross section type from string
    //! \param value String representation of opaque cross section type
    //! \return Opaque cross section type
    OpaqueCrossSectionType opaqueCrossSectionFromString(std::string_view value);

    //! \brief Returns string representation of opaque cross section type
    //! \param opaqueCrossSection Opaque cross section type
    //! \return String representation of opaque cross section type
    std::string opaqueCrossSectionToString(OpaqueCrossSectionType opaqueCrossSection);

    enum class OtherCrossSectionType
    {
        Unknown,
        CommonThermalBridge,
        GeneralCrossSection
    };

    //! \brief Returns all available other cross section types
    std::vector<std::string> otherCrossSectionTypeStrings();

    //! \brief Returns other cross section type from string
    //! \param value String representation of other cross section type
    //! \return Other cross section type
    OtherCrossSectionType otherCrossSectionTypeFromString(std::string_view value);

    //! \brief Returns string representation of other cross section type
    //! \param otherCrossSectionType Other cross section type
    //! \return String representation of other cross section type
    std::string otherCrossSectionTypeToString(OtherCrossSectionType otherCrossSectionType);
}   // namespace ThermFile