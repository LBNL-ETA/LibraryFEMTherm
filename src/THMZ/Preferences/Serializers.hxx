#pragma once

#include <fileParse/Common.hxx>

#include "Preferences.hxx"

#include "THMZ/Geometry/Serializers.hxx"
#include "EnumSerializers.hxx"

namespace ThermFile
{
    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::Underlay & underlay)
    {
        node >> FileParse::Child{"Name", underlay.name};
        node >> FileParse::Child{"IsUnderlayOn", underlay.isUnderlayOn};
        node >> FileParse::Child{"UnitSystem", underlay.unitSystem};
        node >> FileParse::Child{"Origin", underlay.origin};
        node >> FileParse::Child{"UnderlayType", underlay.underlayType};
        node >> FileParse::Child{"HorizontalScale", underlay.horizontalScale};
        node >> FileParse::Child{"VerticalScale", underlay.verticalScale};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::Underlay & underlay)
    {
        node << FileParse::Child{"Name", underlay.name};
        node << FileParse::Child{"IsUnderlayOn", underlay.isUnderlayOn};
        node << FileParse::Child{"UnitSystem", underlay.unitSystem};
        node << FileParse::Child{"Origin", underlay.origin};
        node << FileParse::Child{"UnderlayType", underlay.underlayType};
        node << FileParse::Child{"HorizontalScale", underlay.horizontalScale};
        node << FileParse::Child{"VerticalScale", underlay.verticalScale};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::Settings & modelPreferences)
    {
        node >> FileParse::Child{"Origin", modelPreferences.origin};
        node >> FileParse::Child{"Scale", modelPreferences.scale};
        node >> FileParse::Child{"ConductivityUnits", modelPreferences.conductivityUnits};
        node >> FileParse::Child{"UValueBasis", modelPreferences.uValueBasis};
        node >> FileParse::Child{"CondensationResistanceModel",
                                              modelPreferences.condensationResistanceModel};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::Settings & modelPreferences)
    {
        node << FileParse::Child{"Origin", modelPreferences.origin};
        node << FileParse::Child{"Scale", modelPreferences.scale};
        node << FileParse::Child{"ConductivityUnits", modelPreferences.conductivityUnits};
        node << FileParse::Child{"UValueBasis", modelPreferences.uValueBasis};
        node << FileParse::Child{"CondensationResistanceModel",
                                              modelPreferences.condensationResistanceModel};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::Snap & snap)
    {
        node >> FileParse::Child{"SnapToGrid", snap.snapToGrid};
        node >> FileParse::Child{"SmartSnap", snap.smartSnap};
        node >> FileParse::Child{"SnapToUnderlay", snap.snapToUnderlay};
        node >> FileParse::Child{"SnapToVertical", snap.snapToVertical};
        node >> FileParse::Child{"SnapToHorizontal", snap.snapToHorizontal};
        node >> FileParse::Child{"SnapToAngle", snap.snapToAngle};
        node >> FileParse::Child{"VerticalAndHorizontalLock", snap.verticalAndHorizontalLock};
        node >> FileParse::Child{"DrawGrid", snap.drawGrid};
        node >> FileParse::Child{"SnapAngle", snap.snapAngle};
        node >> FileParse::Child{"GridWidth", snap.gridWidth};
        node >> FileParse::Child{"GridHeight", snap.gridHeight};
        node >> FileParse::Child{"Origin", snap.origin};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::Snap & snap)
    {
        node << FileParse::Child{"SnapToGrid", snap.snapToGrid};
        node << FileParse::Child{"SmartSnap", snap.smartSnap};
        node << FileParse::Child{"SnapToUnderlay", snap.snapToUnderlay};
        node << FileParse::Child{"SnapToVertical", snap.snapToVertical};
        node << FileParse::Child{"SnapToHorizontal", snap.snapToHorizontal};
        node << FileParse::Child{"SnapToAngle", snap.snapToAngle};
        node << FileParse::Child{"VerticalAndHorizontalLock", snap.verticalAndHorizontalLock};
        node << FileParse::Child{"DrawGrid", snap.drawGrid};
        node << FileParse::Child{"SnapAngle", snap.snapAngle};
        node << FileParse::Child{"GridWidth", snap.gridWidth};
        node << FileParse::Child{"GridHeight", snap.gridHeight};
        node << FileParse::Child{"Origin", snap.origin};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::Preferences & preferences)
    {
        node >> FileParse::Child{"Settings", preferences.settings};
        node >> FileParse::Child{"Underlay", preferences.underlay};
        node >> FileParse::Child{"Snap", preferences.snap};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::Preferences & preferences)
    {
        node << FileParse::Child{"Settings", preferences.settings};
        node << FileParse::Child{"Underlay", preferences.underlay};
        node << FileParse::Child{"Snap", preferences.snap};

        return node;
    }
}   // namespace ThermFile