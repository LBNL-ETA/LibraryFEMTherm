#include <map>

#include "GravityProperties.hxx"
#include "Common/GeometryDefinitions.hxx"
#include "LibraryUtilities/Common.hxx"

namespace ThermFile::Gravity
{
    std::map<Orientation, std::string> orientationStringMap = {
      {Orientation::Down, "Down"},
      {Orientation::Up, "Up"},
      {Orientation::Left, "Left"},
      {Orientation::Right, "Right"},
      {Orientation::IntoScreen, "Into Screen"},
      {Orientation::OutOfScreen, "Out Of Screen"}};

    std::vector<std::string> orientationStrings()
    {
        std::vector<std::string> result;
        result.reserve(orientationStringMap.size());
        for(const auto & [orientation, text] : orientationStringMap)
        {
            result.emplace_back(text);
        }

        return result;
    }

    Orientation orientationFromString(std::string_view orientation)
    {
        return LibraryCommon::enumFromString(orientation, orientationStringMap);
    }

    std::string orientationToString(Orientation orientation)
    {
        return LibraryCommon::enumToString(orientation, orientationStringMap);
    }

    Geometry::GravityVector getGravityVector(Orientation orientation)
    {
        static std::map<Orientation, Geometry::GravityVector> coordinates{{Orientation::Up, {0, 1, 0}},
                                                                          {Orientation::Down, {0, -1, 0}},
                                                                          {Orientation::Right, {1, 0, 0}},
                                                                          {Orientation::Left, {-1, 0, 0}},
                                                                          {Orientation::IntoScreen, {0, 0, -1}},
                                                                          {Orientation::OutOfScreen, {0, 0, 1}}};
        if(coordinates.count(orientation))
        {
            return coordinates.at(orientation);
        }
        return coordinates.at(Orientation::Down);
    }

    Orientation getOrientation(const Geometry::GravityVector & vector)
    {
        const double absX = std::abs(vector.x);
        const double absY = std::abs(vector.y);
        const double absZ = std::abs(vector.z);

        if(absX > absY && absX > absZ)
        {
            return (vector.x > 0) ? Orientation::Right : Orientation::Left;
        }
        if(absY > absX && absY > absZ)
        {
            return (vector.y > 0) ? Orientation::Up : Orientation::Down;
        }
        return (vector.z > 0) ? Orientation::OutOfScreen : Orientation::IntoScreen;
    }
}   // namespace Therm::Gravity
