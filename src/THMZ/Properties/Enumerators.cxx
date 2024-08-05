#include <set>

#include "Enumerators.hxx"

#include "LibraryUtilities/Common.hxx"

namespace ThermFile
{
    std::map<ModelPurpose, std::string> modelPurposeString{
      {ModelPurpose::Window_TransparentFacade, "Window/Transparent Facade"},
      {ModelPurpose::OpaqueFacade, "Opaque Facade"},
      {ModelPurpose::Other, "Other"}};

    std::vector<std::string> modelPurposes()
    {
        std::vector<std::string> result;

        result.reserve(modelPurposeString.size());
        for(const auto & [key, text] : modelPurposeString)
        {
            result.emplace_back(text);
        }

        return result;
    }

    ModelPurpose modelPurposeFromString(std::string_view value)
    {
        return LibraryCommon::enumFromString(value, modelPurposeString);
    }

    std::string modelPurposeToString(ModelPurpose modelPurpose)
    {
        return LibraryCommon::enumToString(modelPurpose, modelPurposeString);
    }

    std::map<SimulationEngine, std::string> simulationEngineString{{SimulationEngine::Conrad, "Conrad"},
                                                                   {SimulationEngine::HygroThermFEM, "HygroThermFEM"}};

    SimulationEngine simulationEngineFromString(std::string_view value)
    {
        return LibraryCommon::enumFromString(value, simulationEngineString);
    }

    std::string simulationEngineToString(SimulationEngine simulationEngine)
    {
        return LibraryCommon::enumToString(simulationEngine, simulationEngineString);
    }

    std::map<CalculationMode, std::string> calculationModeString{{CalculationMode::cmSteadyState, "Steady-State"},
                                                                 {CalculationMode::cmTransient, "Transient"}};

    CalculationMode calculationModeFromString(std::string_view value)
    {
        return LibraryCommon::enumFromString(value, calculationModeString);
    }

    std::string calculationModeToString(CalculationMode calculationMode)
    {
        return LibraryCommon::enumToString(calculationMode, calculationModeString);
    }

    std::map<TransientCalculationMethodology, std::string> transientCalculationMethodologyString{
      {TransientCalculationMethodology::icConstantEverywhere, "Constant everywhere"},
      {TransientCalculationMethodology::icUseFirstTimestep, "Use first timestep"},
      {TransientCalculationMethodology::icUseFirstTimestep, "Use first timestep"}};

    TransientCalculationMethodology transientCalculationMethodologyFromString(std::string_view value)
    {
        return LibraryCommon::enumFromString(value, transientCalculationMethodologyString);
    }

    std::string transientCalculationMethodologyToString(TransientCalculationMethodology transientCalculationMethodology)
    {
        return LibraryCommon::enumToString(transientCalculationMethodology, transientCalculationMethodologyString);
    }

    std::map<MesherType, std::string> mesherTypeString{{MesherType::QuadTreeMesher, "QuadTree Mesher"},
                                                       {MesherType::Simmetrix_Version_2022, "Simmetrix Version 2022"}};

    MesherType mesherTypeFromString(std::string_view value)
    {
        return LibraryCommon::enumFromString(value, mesherTypeString);
    }

    std::string mesherTypeToString(MesherType mesherType)
    {
        return LibraryCommon::enumToString(mesherType, mesherTypeString);
    }

    std::map<SteadyStateCalculationMethodology, std::string> steadyStateCalculationMethodologyString{
      {SteadyStateCalculationMethodology::ssNoTimeVariable, "No time variable"},
      {SteadyStateCalculationMethodology::ssFalseTransient, "False transient"}};

    SteadyStateCalculationMethodology steadyStateCalculationMethodologyFromString(std::string_view value)
    {
        return LibraryCommon::enumFromString(value, steadyStateCalculationMethodologyString);
    }

    std::string
      steadyStateCalculationMethodologyToString(SteadyStateCalculationMethodology steadyStateCalculationMethodology)
    {
        return LibraryCommon::enumToString(steadyStateCalculationMethodology, steadyStateCalculationMethodologyString);
    }

    std::map<Orientation, std::string> orientationStringMap = {{Orientation::North, "North"},
                                                                            {Orientation::East, "East"},
                                                                            {Orientation::South, "South"},
                                                                            {Orientation::West, "West"}};

    Orientation orientationFromString(std::string_view value)
    {
        return LibraryCommon::enumFromString(value, orientationStringMap);
    }

    std::string orientationToString(Orientation orientation)
    {
        return LibraryCommon::enumToString(orientation, orientationStringMap);
    }

    bool isJambType(const WindowCrossSectionType type)
    {
        std::set<WindowCrossSectionType> jambTypes{WindowCrossSectionType::Jamb,
                                                   WindowCrossSectionType::VerticalDivider,
                                                   WindowCrossSectionType::VerticalMeetingRail};

        return jambTypes.find(type) != jambTypes.end();
    }

    std::map<WindowCrossSectionType, std::string> windowCrossSectionTypeString{
      {WindowCrossSectionType::Sill, "Sill"},
      {WindowCrossSectionType::Jamb, "Jamb"},
      {WindowCrossSectionType::Head, "Head"},
      {WindowCrossSectionType::HorizontalDivider, "Horizontal Divider"},
      {WindowCrossSectionType::VerticalDivider, "Vertical Divider"},
      {WindowCrossSectionType::HorizontalMeetingRail, "Horizontal Meeting Rail"},
      {WindowCrossSectionType::VerticalMeetingRail, "Vertical Meeting Rail"},
      {WindowCrossSectionType::CommonFrame, "Common Frame"},
      {WindowCrossSectionType::Spacer, "Spacer"}};

    std::vector<std::string> windowCrossSectionStrings()
    {
        std::vector<std::string> result;
        result.reserve(windowCrossSectionTypeString.size());
        for(const auto & [key, strValue] : windowCrossSectionTypeString)
        {
            result.emplace_back(strValue);
        }
        return result;
    }

    WindowCrossSectionType windowCrossSectionTypeFromString(std::string_view value)
    {
        return LibraryCommon::enumFromString(value, windowCrossSectionTypeString);
    }

    std::string windowCrossSectionTypeToString(WindowCrossSectionType type)
    {
        return LibraryCommon::enumToString(type, windowCrossSectionTypeString);
    }

    std::map<OpaqueAssemblyType, std::string> opaqueAssemblyTypeString{
      {OpaqueAssemblyType::Wall, "Wall"}, {OpaqueAssemblyType::Roof, "Roof"}};

    std::vector<std::string> opaqueAssemblyTypeStrings()
    {
        std::vector<std::string> result;
        result.reserve(opaqueAssemblyTypeString.size());
        for(const auto & [key, strValue] : opaqueAssemblyTypeString)
        {
            result.emplace_back(strValue);
        }
        return result;
    }

    OpaqueAssemblyType opaqueAssemblyTypeFromString(std::string_view value)
    {
        return LibraryCommon::enumFromString(value, opaqueAssemblyTypeString);
    }

    std::string opaqueAssemblyTypeToString(OpaqueAssemblyType opaqueAssemblyType)
    {
        return LibraryCommon::enumToString(opaqueAssemblyType, opaqueAssemblyTypeString);
    }

    std::map<OpaqueCrossSectionType, std::string> opaqueCrossSectionString{
      {OpaqueCrossSectionType::SillPlate, "Sill Plate"},
      {OpaqueCrossSectionType::Header, "Header"},
      {OpaqueCrossSectionType::EndSection, "End Section"},
      {OpaqueCrossSectionType::MiddleSection, "Middle Section"},
      {OpaqueCrossSectionType::ThermalBridge, "Thermal Bridge"},
      {OpaqueCrossSectionType::WindowFramingSill, "Window Framing - Sill"},
      {OpaqueCrossSectionType::RoughOpeningHeader, "Rough Opening - Header"},
      {OpaqueCrossSectionType::RoughOpeningJamb, "Rough Opening - Jamb"}};

    std::vector<std::string> opaqueCrossSectionStrings()
    {
        std::vector<std::string> result;
        result.reserve(opaqueCrossSectionString.size());
        for(const auto & [key, strValue] : opaqueCrossSectionString)
        {
            result.emplace_back(strValue);
        }
        return result;
    }

    OpaqueCrossSectionType opaqueCrossSectionFromString(std::string_view value)
    {
        return LibraryCommon::enumFromString(value, opaqueCrossSectionString);
    }

    std::string opaqueCrossSectionToString(OpaqueCrossSectionType opaqueCrossSection)
    {
        return LibraryCommon::enumToString(opaqueCrossSection, opaqueCrossSectionString);
    }

    std::map<OtherCrossSectionType, std::string> otherCrossSectionTypeString{
      {OtherCrossSectionType::CommonThermalBridge, "Common Thermal Bridge"},
      {OtherCrossSectionType::GeneralCrossSection, "General Cross Section"}};

    std::vector<std::string> otherCrossSectionTypeStrings()
    {
        std::vector<std::string> result;
        result.reserve(otherCrossSectionTypeString.size());
        for(const auto & [key, strValue] : otherCrossSectionTypeString)
        {
            result.emplace_back(strValue);
        }
        return result;
    }

    OtherCrossSectionType otherCrossSectionTypeFromString(std::string_view value)
    {
        return LibraryCommon::enumFromString(value, otherCrossSectionTypeString);
    }

    std::string otherCrossSectionTypeToString(OtherCrossSectionType otherCrossSectionType)
    {
        return LibraryCommon::enumToString(otherCrossSectionType, otherCrossSectionTypeString);
    }
}   // namespace ThermFile
