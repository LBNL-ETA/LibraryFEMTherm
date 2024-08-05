#pragma once

#include "THMZ/Preferences/Preferences.hxx"

#include "THMXGeometryHelper.hxx"

namespace Helper
{
    struct MockNode;

    void expect_near(const ThermFile::Underlay & expected, const ThermFile::Underlay & actual, double tolerance);

    void expect_near(const ThermFile::Settings & expected, const ThermFile::Settings & actual, double tolerance);

    void expect_near(const ThermFile::Preferences & expected, const ThermFile::Preferences & actual, double tolerance);

    //////////////////////////////////////////////////////////////////////////////////////////
    //// Helper functions to fill the data
    //////////////////////////////////////////////////////////////////////////////////////////

    struct MockUnderlay
    {
        std::string name;
        std::string isUnderlayOn;
        std::string unitSystem;
        MockPointNode origin;
        std::string underlayType;
        std::string horizontalScale;
        std::string verticalScale;

        MockUnderlay(std::string name,
                     std::string isUnderlayOn,
                     std::string unitSystem,
                     MockPointNode origin,
                     std::string underlayType,
                     std::string horizontalScale,
                     std::string verticalScale);
    };

    MockNode generateUnderlayNode(const MockUnderlay & underlay);

    struct MockSettings
    {
        MockPointNode origin;
        std::string scale;
        std::string conductivityUnits;
        std::string uValueBasis;
        std::string condensationResistanceModel;

        MockSettings(MockPointNode origin,
                             std::string scale,
                             std::string conductivityUnits,
                             std::string uValueBasis,
                             std::string condensationResistanceModel);
    };

    MockNode generateSettingsNode(const MockSettings & modelPreferences);

    struct MockSnap
    {
        std::string snapToGrid;
        std::string smartSnap;
        std::string snapToUnderlay;
        std::string snapToVertical;
        std::string snapToHorizontal;
        std::string snapToAngle;
        std::string verticalAndHorizontalLock;
        std::string drawGrid;
        std::string snapAngle;
        std::string gridWidth;
        std::string gridHeight;
        MockPointNode origin;

        MockSnap(std::string snapToGrid,
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
                 MockPointNode origin);
    };

    MockNode generateSnapNode(const MockSnap & snap);

    struct MockPreferences
    {
        MockUnderlay underlay;
        MockSettings settings;
        MockSnap snap;

        MockPreferences(MockUnderlay underlay, MockSettings settings, MockSnap snap);
    };

    MockNode generatePreferencesNode(const MockPreferences & preferences);

}   // namespace Helper