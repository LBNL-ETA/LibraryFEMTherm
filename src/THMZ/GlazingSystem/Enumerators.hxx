#pragma once

#include <string>

namespace ThermFile
{
    enum class EnvironmentalCondition
    {
        Unknown,
        Winter,
        Summer
    };

    //! \brief Converts a string to a environmental conditions.
    //! \param environmentalConditions The string to convert.
    //! \return The environmental conditions represented by the string.
    EnvironmentalCondition environmentalConditionsFromString(std::string_view environmentalConditions);

    //! \brief Converts environmental conditions to a string.
    //! \param environmentalConditions The environmental conditions to convert.
    //! \return The string representation of the environmental conditions.
    std::string environmentalConditionsToString(EnvironmentalCondition environmentalConditions);

    enum class Position
    {
        Unknown = -1,
        Interior,
        Exterior
    };

    //! \brief Converts a string to a position.
    //! \param position The string to convert.
    //! \return The position represented by the string.
    Position positionFromString(std::string_view position);

    //! \brief Converts a position to a string.
    //! \param position The position to convert.
    //! \return The string representation of the position.
    std::string positionToString(Position position);

    //! \brief Converts a string to a boundary condition style.
    //! \param boundaryConditionStyle The string to convert.
    //! \return The boundary condition style represented by the string.
    //! NOTE: The numbering of the enum class is set to be consistent with the numbering from the legacy code.
    //! If numbering changes, the legacy code conversion will not work correctly anymore.
    enum class BoundaryConditionStyle
    {
        Unknown = -1,
        Combined,
        Split,
        BlackBody,
        Enclosure,
        Existing
    };

    //! \brief Converts a boundary condition style to a string.
    //! \param boundaryConditionStyle The boundary condition style to convert.
    //! \return The string representation of the boundary condition style.
    std::string boundaryConditionStyleToString(BoundaryConditionStyle boundaryConditionStyle);

    //! \brief Converts a string to a boundary condition style.
    //! \param boundaryConditionStyle The string to convert.
    //! \return The boundary condition style represented by the string.
    BoundaryConditionStyle boundaryConditionStyleFromString(std::string_view boundaryConditionStyle);

}   // namespace ThermFile