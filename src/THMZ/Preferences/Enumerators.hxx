#pragma once

#include <string>

namespace ThermFile
{
    enum class UnitSystem
    {
        SI,
        IP
    };

    //! \brief Converts a string to a unit system.
    //! \param unitSystem The string to convert.
    //! \return The unit system represented by the string.
    UnitSystem unitSystemFromString(std::string_view unitSystem);

    //! \brief Converts a unit system to a string.
    //! \param unitSystem The unit system to convert.
    //! \return The string representation of the unit system.
    std::string unitSystemToString(UnitSystem unitSystem);

    enum class ConductivityUnits
    {
        BtuPerHr_Ft_Fahrenheit,
        Btu_InchPerHour_FootSquared_Fahrenheit
    };

    //! \brief Converts a string to a conductivity unit.
    //! \param conductivityUnit The string to convert.
    //! \return The conductivity unit represented by the string.
    ConductivityUnits conductivityUnitsFromString(std::string_view conductivityUnit);

    //! \brief Converts a conductivity unit to a string.
    //! \param conductivityUnit The conductivity unit to convert.
    //! \return The string representation of the conductivity unit.
    std::string conductivityUnitsToString(ConductivityUnits conductivityUnit);

    enum class UValueBasisType
    {
        TotalLength,
        XProjected,
        YProjected,
        Custom,
        GlassRotationProjected,
        CustomRotationProjected
    };

    //! \brief Converts a string to a u-value basis type.
    //! \param uValueBasisType The string to convert.
    //! \return The u-value basis type represented by the string.
    UValueBasisType uValueBasisTypeFromString(std::string_view uValueBasisType);

    //! \brief Converts a u-value basis type to a string.
    //! \param uValueBasisType The u-value basis type to convert.
    //! \return The string representation of the u-value basis type.
    std::string uValueBasisTypeToString(UValueBasisType uValueBasisType);

    enum class UnderlayType
    {
        None,
        Bitmap,
        DXF
    };

    //! \brief Converts a string to a underlay type.
    //! \param underlayType The string to convert.
    //! \return The underlay type represented by the string.
    UnderlayType underlayTypeFromString(std::string_view underlayType);

    //! \brief Converts a underlay type to a string.
    //! \param underlayType The underlay type to convert.
    //! \return The string representation of the underlay type.
    std::string underlayTypeToString(UnderlayType underlayType);

}   // namespace Therm