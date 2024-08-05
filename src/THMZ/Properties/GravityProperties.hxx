#pragma once

#include <string>
#include <vector>

namespace Geometry
{
    struct GravityVector;
}

namespace ThermFile::Gravity
{
    enum class Orientation
    {
        Unknown,
        Down,
        Up,
        Left,
        Right,
        IntoScreen,
        OutOfScreen
    };

    //! \brief Returns all available valid orientations
    std::vector<std::string> orientationStrings();

    //! \brief Returns orientation from string
    //! \param orientation String representation of orientation
    //! \return Orientation
    Orientation orientationFromString(std::string_view orientation);

    //! \brief Returns string representation of orientation
    //! \param orientation Orientation
    //! \return String representation of orientation
    std::string orientationToString(Orientation orientation);

    //! \brief Returns gravity vector from orientation
    //! \param orientation Orientation
    //! \return Gravity vector
    Geometry::GravityVector getGravityVector(Orientation orientation);

    //! \brief Returns orientation from gravity vector
    //! \param vector Gravity vector
    //! \return Orientation
    Orientation getOrientation(const Geometry::GravityVector & vector);

}   // namespace Therm::Gravity
