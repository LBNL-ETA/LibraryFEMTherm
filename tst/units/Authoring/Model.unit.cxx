#include <gtest/gtest.h>

#include "Authoring/Model.hxx"

using namespace ThermFile::Authoring;

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
          .regions = {{.material = "stucco", .points = {{0.0, 0.0}, {0.1, 0.0}, {0.1, 0.01}, {0.0, 0.01}}}},
          .segments = {{.kind = Prescribed{.temperature = 20.0, .humidity = 0.8},
                        .start = {0.0, 0.01},
                        .end = {0.0, 0.0}},
                       {.kind = Sealed{}, .start = {0.0, 0.0}, .end = {0.1, 0.0}},
                       {.kind = Convective{.airTemperature = -5.0, .filmCoefficient = 8.0, .humidity = 0.9},
                        .start = {0.1, 0.0},
                        .end = {0.1, 0.01}},
                       {.kind = Sealed{}, .start = {0.1, 0.01}, .end = {0.0, 0.01}}},
          .initial = {.temperature = 20.0, .humidity = 0.5},
          .schedule = {.dtime = 3600.0, .nSteps = 24U},
          .materials = {{"stucco", stucco()}},
        };
    }
}   // namespace

TEST(AuthoringModel, WellFormedCaseHasNoIssues)
{
    EXPECT_TRUE(issues(strip()).empty());
}

TEST(AuthoringModel, ScheduleDuration)
{
    EXPECT_NEAR(strip().schedule.duration(), 86400.0, 1e-9);
}

TEST(AuthoringModel, NoRegionsIsAnIssue)
{
    auto modelCase{strip()};
    modelCase.regions.clear();
    modelCase.segments.clear();
    const auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 1U);
    EXPECT_EQ(found[0], "a model needs at least one region");
}

TEST(AuthoringModel, TwoPointRegionIsAnIssue)
{
    auto modelCase{strip()};
    modelCase.regions[0].points = {{0.0, 0.0}, {0.1, 0.0}};
    const auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 1U);
    EXPECT_EQ(found[0], "region 0 has 2 points; a region needs at least three");
}

TEST(AuthoringModel, SegmentOnMissingRegionIsAnIssue)
{
    auto modelCase{strip()};
    modelCase.segments[1].region = 3U;
    const auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 1U);
    EXPECT_EQ(found[0], "segment 1 borders region 3, which does not exist");
}

TEST(AuthoringModel, SegmentOffTheOutlineIsAnIssue)
{
    auto modelCase{strip()};
    modelCase.segments[1].end = {0.1, 0.002};   // lifted off the bottom edge
    const auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 1U);
    EXPECT_EQ(found[0], "segment 1 from (0, 0) to (0.1, 0.002) does not lie along an edge of region 0");
}

TEST(AuthoringModel, SegmentBridgingTwoEdgesIsAnIssue)
{
    auto modelCase{strip()};
    // Both endpoints are corners of the region, but on different edges: a chord.
    modelCase.segments[1] = {.kind = Sealed{}, .start = {0.0, 0.0}, .end = {0.1, 0.01}};
    EXPECT_EQ(issues(modelCase).size(), 1U);
}

TEST(AuthoringModel, ASpanOfOneEdgeIsFine)
{
    auto modelCase{strip()};
    modelCase.segments[1] = {.kind = Sealed{}, .start = {0.02, 0.0}, .end = {0.07, 0.0}};
    EXPECT_TRUE(issues(modelCase).empty());

    // Arithmetic on stated coordinates lands within tolerance.
    modelCase.segments[1] = {.kind = Sealed{}, .start = {0.1 - 0.07, 0.0}, .end = {0.3 * 0.1 + 0.04, 0.0}};
    EXPECT_TRUE(issues(modelCase).empty());
}

TEST(AuthoringModel, OutlineIsNotCheckedForARegionAlreadyReported)
{
    auto modelCase{strip()};
    modelCase.regions[0].points = {{0.0, 0.0}, {0.1, 0.0}};   // two points: reported once
    EXPECT_EQ(issues(modelCase).size(), 1U);
}

TEST(AuthoringModel, MaterialNotCarriedIsAnIssue)
{
    auto modelCase{strip()};
    modelCase.regions[0].material = "brick";
    const auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 1U);
    EXPECT_EQ(found[0], "region 0 names material 'brick', which the case does not carry");
}

TEST(AuthoringModel, BadScheduleIsTwoIssues)
{
    auto modelCase{strip()};
    modelCase.schedule = {.dtime = 0.0, .nSteps = 0U};
    const auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 2U);
    EXPECT_EQ(found[0], "the time step must be positive");
    EXPECT_EQ(found[1], "the schedule needs at least one step");
}

TEST(AuthoringModel, IssuesAccumulate)
{
    auto modelCase{strip()};
    modelCase.regions[0].material = "brick";
    modelCase.segments[0].region = 9U;
    modelCase.schedule.nSteps = 0U;
    EXPECT_EQ(issues(modelCase).size(), 3U);
}

TEST(AuthoringModel, MaterialLookup)
{
    const auto modelCase{strip()};
    ASSERT_TRUE(material(modelCase, "stucco").has_value());
    EXPECT_EQ(material(modelCase, "stucco")->name, "Stucco");
    EXPECT_FALSE(material(modelCase, "brick").has_value());

    ASSERT_TRUE(regionMaterial(modelCase, 0U).has_value());
    EXPECT_NEAR(regionMaterial(modelCase, 0U)->density, 1800.0, 1e-9);
    EXPECT_FALSE(regionMaterial(modelCase, 1U).has_value());
}

TEST(AuthoringModel, RecordNamesAreFixedPerKind)
{
    EXPECT_EQ(recordName(Sealed{}), "Sealed");
    EXPECT_EQ(recordName(Prescribed{.temperature = 20.0}), "Prescribed temperature");
    EXPECT_EQ(recordName(Prescribed{.temperature = 20.0, .humidity = 0.5}), "Prescribed temperature and humidity");
    EXPECT_EQ(recordName(Convective{}), "Convective exchange");
}

TEST(AuthoringModel, DefaultsMatchTheEngineSettings)
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
    EXPECT_TRUE(std::holds_alternative<Sealed>(Segment{}.kind));
}

TEST(AuthoringModel, CasesCompareByValue)
{
    EXPECT_EQ(strip(), strip());
    auto changed{strip()};
    changed.initial.humidity = 0.6;
    EXPECT_NE(changed, strip());
}
