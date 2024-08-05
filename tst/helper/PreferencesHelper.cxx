#include <gtest/gtest.h>

#include "PreferencesHelper.hxx"

#include "MockNodeAdapter.hxx"

namespace Helper
{
    void expect_near(const ThermFile::Underlay & expected, const ThermFile::Underlay & actual, double tolerance)
    {
        EXPECT_EQ(expected.name, actual.name);
        EXPECT_EQ(expected.isUnderlayOn, actual.isUnderlayOn);
        EXPECT_EQ(expected.unitSystem, actual.unitSystem);
        EXPECT_NEAR(expected.origin.x, actual.origin.x, tolerance);
        EXPECT_NEAR(expected.origin.y, actual.origin.y, tolerance);
        EXPECT_EQ(expected.underlayType, actual.underlayType);
        EXPECT_NEAR(expected.horizontalScale, actual.horizontalScale, tolerance);
        EXPECT_NEAR(expected.verticalScale, actual.verticalScale, tolerance);
    }

    void expect_near(const ThermFile::Settings & expected, const ThermFile::Settings & actual, double tolerance)
    {
        EXPECT_NEAR(expected.origin.x, actual.origin.x, tolerance);
        EXPECT_NEAR(expected.origin.y, actual.origin.y, tolerance);
        EXPECT_NEAR(expected.scale, actual.scale, tolerance);
        EXPECT_EQ(expected.conductivityUnits, actual.conductivityUnits);
        EXPECT_EQ(expected.uValueBasis, actual.uValueBasis);
        EXPECT_EQ(expected.condensationResistanceModel, actual.condensationResistanceModel);
    }

    void expect_near(const ThermFile::Preferences & expected, const ThermFile::Preferences & actual, double tolerance)
    {
        expect_near(expected.underlay, actual.underlay, tolerance);
        expect_near(expected.settings, actual.settings, tolerance);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //// Helper functions to fill the data
    //////////////////////////////////////////////////////////////////////////////////////////

    MockUnderlay::MockUnderlay(std::string name,
                               std::string isUnderlayOn,
                               std::string unitSystem,
                               MockPointNode origin,
                               std::string underlayType,
                               std::string horizontalScale,
                               std::string verticalScale) :
        name{std::move(name)},
        isUnderlayOn{std::move(isUnderlayOn)},
        unitSystem{std::move(unitSystem)},
        origin{std::move(origin)},
        underlayType{std::move(underlayType)},
        horizontalScale{std::move(horizontalScale)},
        verticalScale{std::move(verticalScale)}
    {}

    MockNode generateUnderlayNode(const MockUnderlay & underlay)
    {
        MockNode node{"Underlay"};
        addChildNode(node, "Name", underlay.name);
        addChildNode(node, "IsUnderlayOn", underlay.isUnderlayOn);
        addChildNode(node, "UnitSystem", underlay.unitSystem);
        addChildNode(node, "Origin", generatePointNode(underlay.origin));
        addChildNode(node, "UnderlayType", underlay.underlayType);
        addChildNode(node, "HorizontalScale", underlay.horizontalScale);
        addChildNode(node, "VerticalScale", underlay.verticalScale);

        return node;
    }

    MockSettings::MockSettings(MockPointNode origin,
                                               std::string scale,
                                               std::string conductivityUnits,
                                               std::string uValueBasis,
                                               std::string condensationResistanceModel) :
        origin{std::move(origin)},
        scale{std::move(scale)},
        conductivityUnits{std::move(conductivityUnits)},
        uValueBasis{std::move(uValueBasis)},
        condensationResistanceModel{std::move(condensationResistanceModel)}
    {}

    MockNode generateSettingsNode(const MockSettings & modelPreferences)
    {
        MockNode node{"Settings"};
        addChildNode(node, "Origin", generatePointNode(modelPreferences.origin));
        addChildNode(node, "Scale", modelPreferences.scale);
        addChildNode(node, "ConductivityUnits", modelPreferences.conductivityUnits);
        addChildNode(node, "UValueBasis", modelPreferences.uValueBasis);
        addChildNode(node, "CondensationResistanceModel", modelPreferences.condensationResistanceModel);

        return node;
    }

    MockSnap::MockSnap(std::string snapToGrid,
                       std::string smartSnap,
                       std::string snapToUnderlay,
                       std::string snapToVertical,
                       std::string snapToHorizontal,
                       std::string snapToAngle,
                       std::string vhLock,
                       std::string drawGrid,
                       std::string snapAngle,
                       std::string gridWidth,
                       std::string gridHeight,
                       MockPointNode origin) :
        snapToGrid{std::move(snapToGrid)},
        smartSnap{std::move(smartSnap)},
        snapToUnderlay{std::move(snapToUnderlay)},
        snapToVertical{std::move(snapToVertical)},
        snapToHorizontal{std::move(snapToHorizontal)},
        snapToAngle{std::move(snapToAngle)},
        verticalAndHorizontalLock{std::move(vhLock)},
        drawGrid{std::move(drawGrid)},
        snapAngle{std::move(snapAngle)},
        gridWidth{std::move(gridWidth)},
        gridHeight{std::move(gridHeight)},
        origin{std::move(origin)}
    {}

    MockNode generateSnapNode(const MockSnap & snap)
    {
        MockNode node{"Snap"};
        addChildNode(node, "SnapToGrid", snap.snapToGrid);
        addChildNode(node, "SmartSnap", snap.smartSnap);
        addChildNode(node, "SnapToUnderlay", snap.snapToUnderlay);
        addChildNode(node, "SnapToVertical", snap.snapToVertical);
        addChildNode(node, "SnapToHorizontal", snap.snapToHorizontal);
        addChildNode(node, "SnapToAngle", snap.snapToAngle);
        addChildNode(node, "VerticalAndHorizontalLock", snap.verticalAndHorizontalLock);
        addChildNode(node, "DrawGrid", snap.drawGrid);
        addChildNode(node, "SnapAngle", snap.snapAngle);
        addChildNode(node, "GridWidth", snap.gridWidth);
        addChildNode(node, "GridHeight", snap.gridHeight);
        addChildNode(node, "Origin", generatePointNode(snap.origin));

        return node;
    }

    MockPreferences::MockPreferences(MockUnderlay underlay, MockSettings settings, MockSnap snap) :
        underlay{std::move(underlay)}, settings{std::move(settings)}, snap{std::move(snap)}
    {}

    MockNode generatePreferencesNode(const MockPreferences & preferences)
    {
        MockNode node{"Preferences"};
        addChildNode(node, "Settings", generateSettingsNode(preferences.settings));
        addChildNode(node, "Underlay", generateUnderlayNode(preferences.underlay));
        addChildNode(node, "Snap", generateSnapNode(preferences.snap));

        return node;
    }
}   // namespace Helper