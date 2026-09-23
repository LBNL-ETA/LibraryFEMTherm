#include <gtest/gtest.h>

#include "Model/Case.hxx"

using namespace ThermFile::Model;

namespace
{
    Material stucco()
    {
        return Material{.name = "Stucco",
                        .diffusionResistanceFactor = 25.0,
                        .sorptionCurve = {{0.0, 0.0}, {0.8, 30.0}, {1.0, 180.0}},
                        .density = 1800.0,
                        .heatCapacity = 850.0,
                        .thermalConductivity = 0.8,
                        .porosity = 0.3};
    }

    //! A strip of one material, sealed top and bottom, a condition on each end.
    ModelCase strip()
    {
        return ModelCase{
          .id = "strip",
          .regions = {{.material = stucco(), .points = {{0.0, 0.0}, {0.1, 0.0}, {0.1, 0.01}, {0.0, 0.01}}}},
          .segments = {{.kind = Prescribed{.temperature = 20.0, .humidity = 0.8},
                        .start = {0.0, 0.01},
                        .end = {0.0, 0.0}},
                       {.kind = Adiabatic{}, .start = {0.0, 0.0}, .end = {0.1, 0.0}},
                       {.kind = Convective{.airTemperature = -5.0, .filmCoefficient = 8.0, .humidity = 0.9},
                        .start = {0.1, 0.0},
                        .end = {0.1, 0.01}},
                       {.kind = Adiabatic{}, .start = {0.1, 0.01}, .end = {0.0, 0.01}}},
          .initial = {.temperature = 20.0, .humidity = 0.5},
          .schedule = {.dtime = 3600.0, .nSteps = 24U},
        };
    }
}   // namespace

TEST(Model, WellFormedCaseHasNoIssues)
{
    EXPECT_TRUE(issues(strip()).empty());
}

TEST(Model, ScheduleDuration)
{
    EXPECT_NEAR(strip().schedule.duration(), 86400.0, 1e-9);
}

TEST(Model, NoRegionsIsAnIssue)
{
    auto modelCase{strip()};
    modelCase.regions.clear();
    modelCase.segments.clear();
    const auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 1U);
    EXPECT_EQ(found[0], "a model needs at least one region");
}

TEST(Model, TwoPointRegionIsAnIssue)
{
    auto modelCase{strip()};
    modelCase.regions[0].points = {{0.0, 0.0}, {0.1, 0.0}};
    const auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 1U);
    EXPECT_EQ(found[0], "region 0 has 2 points; a region needs at least three");
}

TEST(Model, SegmentOnMissingRegionIsAnIssue)
{
    auto modelCase{strip()};
    modelCase.segments[1].region = 3U;
    const auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 1U);
    EXPECT_EQ(found[0], "segment 1 borders region 3, which does not exist");
}

TEST(Model, SegmentOffTheOutlineIsAnIssue)
{
    auto modelCase{strip()};
    modelCase.segments[1].end = {0.1, 0.002};   // lifted off the bottom edge
    const auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 1U);
    EXPECT_EQ(found[0], "segment 1 from (0, 0) to (0.1, 0.002) does not lie along an edge of any region");
}

TEST(Model, NamedRegionIsCheckedAsNamed)
{
    auto modelCase{strip()};
    modelCase.segments[1].region = 0U;
    modelCase.segments[1].end = {0.1, 0.002};
    const auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 1U);
    EXPECT_EQ(found[0], "segment 1 from (0, 0) to (0.1, 0.002) does not lie along an edge of region 0");
}

TEST(Model, UnnamedSegmentOnTheOutlineIsAttachedToItsRegion)
{
    const auto modelCase{strip()};
    for(std::size_t index = 0U; index < modelCase.segments.size(); ++index)
    {
        EXPECT_FALSE(modelCase.segments[index].region.has_value());
        EXPECT_EQ(segmentRegion(modelCase, index), std::optional{std::size_t{0U}});
    }
    EXPECT_FALSE(segmentRegion(modelCase, modelCase.segments.size()).has_value());
}

TEST(Model, SharedEdgeMustNameItsRegion)
{
    auto modelCase{strip()};
    modelCase.regions.push_back({.material = stucco(), .points = {{0.1, 0.0}, {0.2, 0.0}, {0.2, 0.01}, {0.1, 0.01}}});
    // The convective segment now sits on the edge both rectangles share.
    auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 1U);
    EXPECT_EQ(found[0],
              "segment 2 from (0.1, 0) to (0.1, 0.01) lies on an edge shared by regions 0 and 1; state which with region");
    EXPECT_FALSE(segmentRegion(modelCase, 2U).has_value());

    modelCase.segments[2].region = 1U;
    EXPECT_TRUE(issues(modelCase).empty());
    EXPECT_EQ(segmentRegion(modelCase, 2U), std::optional{std::size_t{1U}});
}

TEST(Model, SegmentBridgingTwoEdgesIsAnIssue)
{
    auto modelCase{strip()};
    // Both endpoints are corners of the region, but on different edges: a chord.
    modelCase.segments[1] = {.kind = Adiabatic{}, .start = {0.0, 0.0}, .end = {0.1, 0.01}};
    EXPECT_EQ(issues(modelCase).size(), 1U);
}

TEST(Model, ASpanOfOneEdgeIsFine)
{
    auto modelCase{strip()};
    modelCase.segments[1] = {.kind = Adiabatic{}, .start = {0.02, 0.0}, .end = {0.07, 0.0}};
    EXPECT_TRUE(issues(modelCase).empty());

    // Arithmetic on stated coordinates lands within tolerance.
    modelCase.segments[1] = {.kind = Adiabatic{}, .start = {0.1 - 0.07, 0.0}, .end = {0.3 * 0.1 + 0.04, 0.0}};
    EXPECT_TRUE(issues(modelCase).empty());
}

TEST(Model, OutlineIsNotCheckedForARegionAlreadyReported)
{
    auto modelCase{strip()};
    modelCase.regions[0].points = {{0.0, 0.0}, {0.1, 0.0}};   // two points: reported once
    EXPECT_EQ(issues(modelCase).size(), 1U);
}

TEST(Model, NamelessMaterialIsAnIssue)
{
    auto modelCase{strip()};
    modelCase.regions[0].material.name.clear();
    const auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 1U);
    EXPECT_EQ(found[0], "region 0 has a material with no name");
}

TEST(Model, OneNameWithTwoDefinitionsIsAnIssue)
{
    auto modelCase{strip()};
    auto denser{stucco()};
    denser.density = 1900.0;
    modelCase.regions.push_back({.material = denser, .points = {{0.1, 0.0}, {0.2, 0.0}, {0.2, 0.01}, {0.1, 0.01}}});
    modelCase.segments[2].region = 0U;   // the shared edge, attributed
    const auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 1U);
    EXPECT_EQ(found[0],
              "regions 0 and 1 both use a material named 'Stucco' with different properties; the library keeps one "
              "record per name");

    modelCase.regions[1].material = stucco();
    EXPECT_TRUE(issues(modelCase).empty());
}

TEST(Model, BadScheduleIsTwoIssues)
{
    auto modelCase{strip()};
    modelCase.schedule = {.dtime = 0.0, .nSteps = 0U};
    const auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 2U);
    EXPECT_EQ(found[0], "the time step must be positive");
    EXPECT_EQ(found[1], "the schedule needs at least one step");
}

TEST(Model, IssuesAccumulate)
{
    auto modelCase{strip()};
    modelCase.regions[0].material.name.clear();
    modelCase.segments[0].region = 9U;
    modelCase.schedule.nSteps = 0U;
    EXPECT_EQ(issues(modelCase).size(), 3U);
}

TEST(Model, MaterialLookup)
{
    const auto modelCase{strip()};
    ASSERT_TRUE(regionMaterial(modelCase, 0U).has_value());
    EXPECT_EQ(regionMaterial(modelCase, 0U)->name, "Stucco");
    EXPECT_NEAR(regionMaterial(modelCase, 0U)->density, 1800.0, 1e-9);
    EXPECT_FALSE(regionMaterial(modelCase, 1U).has_value());
}

TEST(Model, RecordNamesAreFixedPerKind)
{
    EXPECT_EQ(recordName(Adiabatic{}), "Adiabatic");
    EXPECT_EQ(recordName(Prescribed{.temperature = 20.0}), "Prescribed temperature");
    EXPECT_EQ(recordName(Prescribed{.temperature = 20.0, .humidity = 0.5}), "Prescribed temperature and humidity");
    EXPECT_EQ(recordName(Convective{}), "Convective exchange");
}

TEST(Model, DefaultsMatchTheEngineSettings)
{
    const Physics physics;
    EXPECT_TRUE(physics.thermal && physics.moisture && physics.liquidTransport && physics.heatOfEvaporation
                && physics.capillaryConduction && physics.vaporDiffusionConduction && physics.latentHeatOfFusion
                && physics.conductivityDependent);

    const Numerics numerics;
    EXPECT_NEAR(numerics.convergenceTolerance, 1e-5, 1e-15);
    EXPECT_NEAR(numerics.relaxation, 1.0, 1e-15);
    EXPECT_EQ(numerics.maxIterations, 25);

    EXPECT_NEAR(Convective{}.humidity, 0.5, 1e-15);
    EXPECT_TRUE(std::holds_alternative<Adiabatic>(Segment{}.kind));
}

TEST(Model, CasesCompareByValue)
{
    EXPECT_EQ(strip(), strip());
    auto changed{strip()};
    changed.initial.humidity = 0.6;
    EXPECT_NE(changed, strip());
}
