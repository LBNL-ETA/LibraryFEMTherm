#include "Enumerators.hxx"

#include <map>

#include "LibraryUtilities/Common.hxx"

namespace ThermFile
{
    const std::map<UnitSystem, std::string> unitSystemToStringMap = {{UnitSystem::SI, "SI"}, {UnitSystem::IP, "IP"}};

    UnitSystem unitSystemFromString(std::string_view unitSystem)
    {
        return LibraryCommon::enumFromString(unitSystem, unitSystemToStringMap);
    }

    std::string unitSystemToString(UnitSystem unitSystem)
    {
        return LibraryCommon::enumToString(unitSystem, unitSystemToStringMap);
    }

    const std::map<ConductivityUnits, std::string> conductivityUnitsToStringMap = {
      {ConductivityUnits::BtuPerHr_Ft_Fahrenheit, "Btu/hr-ft-F"},
      {ConductivityUnits::Btu_InchPerHour_FootSquared_Fahrenheit, "Btu-in/hr-ft2-F"}};

    ConductivityUnits conductivityUnitsFromString(std::string_view conductivityUnit)
    {
        return LibraryCommon::enumFromString(conductivityUnit, conductivityUnitsToStringMap);
    }

    std::string conductivityUnitsToString(ConductivityUnits conductivityUnit)
    {
        return LibraryCommon::enumToString(conductivityUnit, conductivityUnitsToStringMap);
    }

    const std::map<UValueBasisType, std::string> uValueBasisTypeToStringMap = {
      {UValueBasisType::TotalLength, "Total length"},
      {UValueBasisType::XProjected, "X-projected"},
      {UValueBasisType::YProjected, "Y-projected"},
      {UValueBasisType::Custom, "Custom"},
      {UValueBasisType::GlassRotationProjected, "Glass rotation projected"},
      {UValueBasisType::CustomRotationProjected, "Custom rotation projected"}};

    UValueBasisType uValueBasisTypeFromString(std::string_view uValueBasisType)
    {
        return LibraryCommon::enumFromString(uValueBasisType, uValueBasisTypeToStringMap);
    }

    std::string uValueBasisTypeToString(UValueBasisType uValueBasisType)
    {
        return LibraryCommon::enumToString(uValueBasisType, uValueBasisTypeToStringMap);
    }

    const std::map<UnderlayType, std::string> underlayTypeToStringMap = {
      {UnderlayType::None, "None"}, {UnderlayType::Bitmap, "Bitmap"}, {UnderlayType::DXF, "DXF"}};

    UnderlayType underlayTypeFromString(std::string_view underlayType)
    {
        return LibraryCommon::enumFromString(underlayType, underlayTypeToStringMap);
    }

    std::string underlayTypeToString(UnderlayType underlayType)
    {
        return LibraryCommon::enumToString(underlayType, underlayTypeToStringMap);
    }
}   // namespace ThermFile
