#include <gtest/gtest.h>

#include "THMXHelper.hxx"

#include "THMZ/Model/THMX.hxx"
#include "PreferencesHelper.hxx"
#include "PropertiesHelper.hxx"
#include "THMXGeometryHelper.hxx"
#include "THMXSteadyStateResultsHelper.hxx"


namespace Helper
{
    void expect_near(const ThermFile::ThermModel & expected, const ThermFile::ThermModel & result, double tolerance)
    {
        ASSERT_EQ(expected.calculationReady, result.calculationReady);
        expect_near(expected.properties, result.properties, tolerance);
        expect_near(expected.preferences, result.preferences, tolerance);
        ASSERT_EQ(expected.polygons.size(), result.polygons.size());
        for(size_t i = 0; i < expected.polygons.size(); ++i)
        {
            expect_near(expected.polygons[i], result.polygons[i], tolerance);
        }

        ASSERT_EQ(expected.cavities.size(), result.cavities.size());
        for(size_t i = 0; i < expected.cavities.size(); ++i)
        {
            expect_near(expected.cavities[i], result.cavities[i], tolerance);
        }

        ASSERT_EQ(expected.boundaryConditions.size(), result.boundaryConditions.size());
        for(size_t i = 0; i < expected.boundaryConditions.size(); ++i)
        {
            expect_near(expected.boundaryConditions[i], result.boundaryConditions[i], tolerance);
        }
    }
}
