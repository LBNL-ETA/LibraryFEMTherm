#include <pybind11/pybind11.h>

#include "THMZ/Geometry/Enumerators.hxx"
#include "THMZ/Preferences/Enumerators.hxx"
#include "THMZ/Properties/Enumerators.hxx"
#include "THMZ/Properties/GravityProperties.hxx"
#include "THMZ/GlazingSystem/Enumerators.hxx"
#include "THMZ/CMA/Enumerators.hxx"
#include "Materials/Definitions.hxx"
#include "fileParse/FileFormat.hxx"

namespace py = pybind11;

void bind_enums(py::module_ & mod)
{
    // --- Geometry enums ---
    py::enum_<ThermFile::Direction>(mod, "Direction")
        .value("Unknown", ThermFile::Direction::Unknown)
        .value("Right", ThermFile::Direction::Right)
        .value("Left", ThermFile::Direction::Left)
        .value("Up", ThermFile::Direction::Up)
        .value("Down", ThermFile::Direction::Down);

    py::enum_<ThermFile::SurfaceType>(mod, "SurfaceType")
        .value("Unknown", ThermFile::SurfaceType::Unknown)
        .value("NotASurface", ThermFile::SurfaceType::NotASurface)
        .value("BoundaryCondition", ThermFile::SurfaceType::BoundaryCondition)
        .value("GlazingCavity", ThermFile::SurfaceType::GlazingCavity)
        .value("FrameCavity", ThermFile::SurfaceType::FrameCavity)
        .value("Enclosure", ThermFile::SurfaceType::Enclosure);

    py::enum_<ThermFile::ShadeModifier>(mod, "ShadeModifier")
        .value("None", ThermFile::ShadeModifier::None)
        .value("Exterior", ThermFile::ShadeModifier::Exterior)
        .value("Interior", ThermFile::ShadeModifier::Interior);

    py::enum_<ThermFile::PolygonType>(mod, "PolygonType")
        .value("Unknown", ThermFile::PolygonType::Unknown)
        .value("None_", ThermFile::PolygonType::None)
        .value("Material", ThermFile::PolygonType::Material)
        .value("Glass", ThermFile::PolygonType::Glass)
        .value("Gap", ThermFile::PolygonType::Gap)
        .value("BoundaryCondition", ThermFile::PolygonType::BoundaryCondition)
        .value("Spacer", ThermFile::PolygonType::Spacer)
        .value("GlassIn", ThermFile::PolygonType::GlassIn)
        .value("GlassOut", ThermFile::PolygonType::GlassOut)
        .value("ShadeGap", ThermFile::PolygonType::ShadeGap);

    py::enum_<ThermFile::MaterialSide>(mod, "MaterialSide")
        .value("Unknown", ThermFile::MaterialSide::Unknown)
        .value("Front", ThermFile::MaterialSide::Front)
        .value("Back", ThermFile::MaterialSide::Back);

    // --- Preferences enums ---
    py::enum_<ThermFile::UnitSystem>(mod, "UnitSystem")
        .value("SI", ThermFile::UnitSystem::SI)
        .value("IP", ThermFile::UnitSystem::IP);

    py::enum_<ThermFile::ConductivityUnits>(mod, "ConductivityUnits")
        .value("BtuPerHr_Ft_Fahrenheit", ThermFile::ConductivityUnits::BtuPerHr_Ft_Fahrenheit)
        .value("Btu_InchPerHour_FootSquared_Fahrenheit", ThermFile::ConductivityUnits::Btu_InchPerHour_FootSquared_Fahrenheit);

    py::enum_<ThermFile::UValueBasisType>(mod, "UValueBasisType")
        .value("TotalLength", ThermFile::UValueBasisType::TotalLength)
        .value("XProjected", ThermFile::UValueBasisType::XProjected)
        .value("YProjected", ThermFile::UValueBasisType::YProjected)
        .value("Custom", ThermFile::UValueBasisType::Custom)
        .value("GlassRotationProjected", ThermFile::UValueBasisType::GlassRotationProjected)
        .value("CustomRotationProjected", ThermFile::UValueBasisType::CustomRotationProjected);

    py::enum_<ThermFile::UnderlayType>(mod, "UnderlayType")
        .value("None_", ThermFile::UnderlayType::None)
        .value("Bitmap", ThermFile::UnderlayType::Bitmap)
        .value("DXF", ThermFile::UnderlayType::DXF);

    // --- Properties enums ---
    py::enum_<ThermFile::ModelPurpose>(mod, "ModelPurpose")
        .value("Unknown", ThermFile::ModelPurpose::Unknown)
        .value("Window_TransparentFacade", ThermFile::ModelPurpose::Window_TransparentFacade)
        .value("OpaqueFacade", ThermFile::ModelPurpose::OpaqueFacade)
        .value("Other", ThermFile::ModelPurpose::Other);

    py::enum_<ThermFile::SimulationEngine>(mod, "SimulationEngine")
        .value("Unknown", ThermFile::SimulationEngine::Unknown)
        .value("Conrad", ThermFile::SimulationEngine::Conrad)
        .value("HygroThermFEM", ThermFile::SimulationEngine::HygroThermFEM);

    py::enum_<ThermFile::CalculationMode>(mod, "CalculationMode")
        .value("Unknown", ThermFile::CalculationMode::Unknown)
        .value("cmTransient", ThermFile::CalculationMode::cmTransient)
        .value("cmSteadyState", ThermFile::CalculationMode::cmSteadyState);

    py::enum_<ThermFile::TransientCalculationMethodology>(mod, "TransientCalculationMethodology")
        .value("Unknown", ThermFile::TransientCalculationMethodology::Unknown)
        .value("icValuesFromInputFile", ThermFile::TransientCalculationMethodology::icValuesFromInputFile)
        .value("icUseFirstTimestep", ThermFile::TransientCalculationMethodology::icUseFirstTimestep)
        .value("icConstantEverywhere", ThermFile::TransientCalculationMethodology::icConstantEverywhere);

    py::enum_<ThermFile::SteadyStateCalculationMethodology>(mod, "SteadyStateCalculationMethodology")
        .value("Unknown", ThermFile::SteadyStateCalculationMethodology::Unknown)
        .value("ssNoTimeVariable", ThermFile::SteadyStateCalculationMethodology::ssNoTimeVariable)
        .value("ssFalseTransient", ThermFile::SteadyStateCalculationMethodology::ssFalseTransient);

    py::enum_<ThermFile::MesherType>(mod, "MesherType")
        .value("Unknown", ThermFile::MesherType::Unknown)
        .value("QuadTreeMesher", ThermFile::MesherType::QuadTreeMesher)
        .value("Simmetrix_Version_2022", ThermFile::MesherType::Simmetrix_Version_2022);

    py::enum_<ThermFile::Orientation>(mod, "Orientation")
        .value("Unknown", ThermFile::Orientation::Unknown)
        .value("North", ThermFile::Orientation::North)
        .value("East", ThermFile::Orientation::East)
        .value("South", ThermFile::Orientation::South)
        .value("West", ThermFile::Orientation::West);

    py::enum_<ThermFile::WindowCrossSectionType>(mod, "WindowCrossSectionType")
        .value("Unknown", ThermFile::WindowCrossSectionType::Unknown)
        .value("Sill", ThermFile::WindowCrossSectionType::Sill)
        .value("Jamb", ThermFile::WindowCrossSectionType::Jamb)
        .value("Head", ThermFile::WindowCrossSectionType::Head)
        .value("HorizontalDivider", ThermFile::WindowCrossSectionType::HorizontalDivider)
        .value("VerticalDivider", ThermFile::WindowCrossSectionType::VerticalDivider)
        .value("HorizontalMeetingRail", ThermFile::WindowCrossSectionType::HorizontalMeetingRail)
        .value("VerticalMeetingRail", ThermFile::WindowCrossSectionType::VerticalMeetingRail)
        .value("CommonFrame", ThermFile::WindowCrossSectionType::CommonFrame)
        .value("Spacer", ThermFile::WindowCrossSectionType::Spacer);

    py::enum_<ThermFile::OpaqueAssemblyType>(mod, "OpaqueAssemblyType")
        .value("Unknown", ThermFile::OpaqueAssemblyType::Unknown)
        .value("Wall", ThermFile::OpaqueAssemblyType::Wall)
        .value("Roof", ThermFile::OpaqueAssemblyType::Roof);

    py::enum_<ThermFile::OpaqueCrossSectionType>(mod, "OpaqueCrossSectionType")
        .value("Unknown", ThermFile::OpaqueCrossSectionType::Unknown)
        .value("SillPlate", ThermFile::OpaqueCrossSectionType::SillPlate)
        .value("Header", ThermFile::OpaqueCrossSectionType::Header)
        .value("EndSection", ThermFile::OpaqueCrossSectionType::EndSection)
        .value("MiddleSection", ThermFile::OpaqueCrossSectionType::MiddleSection)
        .value("ThermalBridge", ThermFile::OpaqueCrossSectionType::ThermalBridge)
        .value("WindowFramingSill", ThermFile::OpaqueCrossSectionType::WindowFramingSill)
        .value("RoughOpeningHeader", ThermFile::OpaqueCrossSectionType::RoughOpeningHeader)
        .value("RoughOpeningJamb", ThermFile::OpaqueCrossSectionType::RoughOpeningJamb);

    py::enum_<ThermFile::OtherCrossSectionType>(mod, "OtherCrossSectionType")
        .value("Unknown", ThermFile::OtherCrossSectionType::Unknown)
        .value("CommonThermalBridge", ThermFile::OtherCrossSectionType::CommonThermalBridge)
        .value("GeneralCrossSection", ThermFile::OtherCrossSectionType::GeneralCrossSection);

    // --- Gravity orientation ---
    py::enum_<ThermFile::Gravity::Orientation>(mod, "GravityOrientation")
        .value("Unknown", ThermFile::Gravity::Orientation::Unknown)
        .value("Down", ThermFile::Gravity::Orientation::Down)
        .value("Up", ThermFile::Gravity::Orientation::Up)
        .value("Left", ThermFile::Gravity::Orientation::Left)
        .value("Right", ThermFile::Gravity::Orientation::Right)
        .value("IntoScreen", ThermFile::Gravity::Orientation::IntoScreen)
        .value("OutOfScreen", ThermFile::Gravity::Orientation::OutOfScreen);

    // --- GlazingSystem enums ---
    py::enum_<ThermFile::EnvironmentalCondition>(mod, "EnvironmentalCondition")
        .value("Unknown", ThermFile::EnvironmentalCondition::Unknown)
        .value("Winter", ThermFile::EnvironmentalCondition::Winter)
        .value("Summer", ThermFile::EnvironmentalCondition::Summer);

    py::enum_<ThermFile::Position>(mod, "Position")
        .value("Unknown", ThermFile::Position::Unknown)
        .value("Interior", ThermFile::Position::Interior)
        .value("Exterior", ThermFile::Position::Exterior);

    py::enum_<ThermFile::BoundaryConditionStyle>(mod, "BoundaryConditionStyle")
        .value("Unknown", ThermFile::BoundaryConditionStyle::Unknown)
        .value("Combined", ThermFile::BoundaryConditionStyle::Combined)
        .value("Split", ThermFile::BoundaryConditionStyle::Split)
        .value("BlackBody", ThermFile::BoundaryConditionStyle::BlackBody)
        .value("Enclosure", ThermFile::BoundaryConditionStyle::Enclosure)
        .value("Existing", ThermFile::BoundaryConditionStyle::Existing);

    // --- CMA enums ---
    py::enum_<ThermFile::CMABCPosition>(mod, "CMABCPosition")
        .value("None_", ThermFile::CMABCPosition::None)
        .value("Interior", ThermFile::CMABCPosition::Interior)
        .value("Exterior", ThermFile::CMABCPosition::Exterior);

    py::enum_<ThermFile::CMAIGUType>(mod, "CMAIGUType")
        .value("Unknown", ThermFile::CMAIGUType::Unknown)
        .value("SingleLayer", ThermFile::CMAIGUType::SingleLayer)
        .value("DoubleLayer", ThermFile::CMAIGUType::DoubleLayer);

    py::enum_<ThermFile::CMACase>(mod, "CMACase")
        .value("None_", ThermFile::CMACase::None)
        .value("Low", ThermFile::CMACase::Low)
        .value("High", ThermFile::CMACase::High);

    // --- Materials enums ---
    py::enum_<MaterialsLibrary::MaterialType>(mod, "MaterialType")
        .value("Solid", MaterialsLibrary::MaterialType::Solid)
        .value("Cavity", MaterialsLibrary::MaterialType::Cavity)
        .value("RadiationEnclosure", MaterialsLibrary::MaterialType::RadiationEnclosure);

    py::enum_<MaterialsLibrary::MaterialRoughness>(mod, "MaterialRoughness")
        .value("VeryRough", MaterialsLibrary::MaterialRoughness::VeryRough)
        .value("Rough", MaterialsLibrary::MaterialRoughness::Rough)
        .value("MediumRough", MaterialsLibrary::MaterialRoughness::MediumRough)
        .value("MediumSmooth", MaterialsLibrary::MaterialRoughness::MediumSmooth)
        .value("Smooth", MaterialsLibrary::MaterialRoughness::Smooth)
        .value("VerySmooth", MaterialsLibrary::MaterialRoughness::VerySmooth);

    py::enum_<MaterialsLibrary::CavityStandard>(mod, "CavityStandard")
        .value("NFRC", MaterialsLibrary::CavityStandard::NFRC)
        .value("CEN", MaterialsLibrary::CavityStandard::CEN)
        .value("CENVentilated", MaterialsLibrary::CavityStandard::CENVentilated)
        .value("NFRCWithUserDimensions", MaterialsLibrary::CavityStandard::NFRCWithUserDimensions)
        .value("ISO15099", MaterialsLibrary::CavityStandard::ISO15099)
        .value("ISO15099Ventilated", MaterialsLibrary::CavityStandard::ISO15099Ventilated);

    // --- FileParse enums ---
    py::enum_<FileParse::FileFormat>(mod, "FileFormat")
        .value("Unknown", FileParse::FileFormat::Unknown)
        .value("XML", FileParse::FileFormat::XML)
        .value("JSON", FileParse::FileFormat::JSON);
}
