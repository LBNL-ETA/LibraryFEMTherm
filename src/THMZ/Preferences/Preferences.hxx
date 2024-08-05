#pragma once

#include <string>

#include "THMZ/Geometry/Geometry.hxx"
#include "Enumerators.hxx"

namespace ThermFile
{
    struct Underlay
    {
        std::string name;
        bool isUnderlayOn{false};
        UnitSystem unitSystem{UnitSystem::SI};
        Point origin{0.0, 0.0};
        UnderlayType underlayType{UnderlayType::None};
        double horizontalScale{1.0};
        double verticalScale{1.0};
    };

    struct Settings
    {
        Point origin{0.0, 0.0};
        double scale{1.0};
        ConductivityUnits conductivityUnits{ConductivityUnits::BtuPerHr_Ft_Fahrenheit};
        UValueBasisType uValueBasis{UValueBasisType::YProjected};
        bool condensationResistanceModel{false};
    };

    struct Snap
    {
        bool snapToGrid{false};
        bool smartSnap{false};
        bool snapToUnderlay{false};
        bool snapToVertical{false};
        bool snapToHorizontal{false};
        bool snapToAngle{false};
        bool verticalAndHorizontalLock{false};
        bool drawGrid{false};
        double snapAngle{0.0};
        double gridWidth{0.0};
        double gridHeight{0.0};
        Point origin;
    };

    struct Preferences
    {
        Underlay underlay;
        Settings settings;
        Snap snap;
    };
}