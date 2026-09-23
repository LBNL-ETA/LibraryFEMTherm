#include <gtest/gtest.h>

#include "Model/Case.hxx"
#include "Materials/FromValues.hxx"

using namespace ThermFile::Model;

namespace
{
    MaterialsLibrary::Material stucco()
    {
        return MaterialsLibrary::fromValues("Stucco", 25.0, {{0.0, 0.0}, {0.8, 30.0}, {1.0, 180.0}}, {}, {}, 1800.0, 850.0, 0.8, 0.0, 0.0, 0.3);
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

TEST(Model, ANamedSegmentIsNamedInItsIssue)
{
    auto modelCase{strip()};
    modelCase.segments[1].name = "bottom";
    modelCase.segments[1].end = {0.1, 0.002};
    const auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 1U);
    EXPECT_EQ(found[0], "segment 1 'bottom' from (0, 0) to (0.1, 0.002) does not lie along an edge of any region");
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
    modelCase.regions[0].material.Name.clear();
    const auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 1U);
    EXPECT_EQ(found[0], "region 0 has a material with no name");
}

TEST(Model, OneNameWithTwoRecordsIsAnIssue)
{
    auto modelCase{strip()};
    auto another{stucco()};
    another.UUID = "another-record";
    modelCase.regions.push_back({.material = another, .points = {{0.1, 0.0}, {0.2, 0.0}, {0.2, 0.01}, {0.1, 0.01}}});
    modelCase.segments[2].region = 0U;   // the shared edge, attributed
    const auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 1U);
    EXPECT_EQ(found[0],
              "regions 0 and 1 carry two different materials both named 'Stucco'; the library keeps one record per "
              "name");

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
    modelCase.regions[0].material.Name.clear();
    modelCase.segments[0].region = 9U;
    modelCase.schedule.nSteps = 0U;
    EXPECT_EQ(issues(modelCase).size(), 3U);
}

TEST(Model, MaterialLookup)
{
    const auto modelCase{strip()};
    ASSERT_TRUE(regionMaterial(modelCase, 0U).has_value());
    EXPECT_EQ(regionMaterial(modelCase, 0U)->Name, "Stucco");
    EXPECT_NEAR(regionMaterial(modelCase, 0U)->data.hygroThermal->BulkDensity.value(), 1800.0, 1e-9);
    EXPECT_FALSE(regionMaterial(modelCase, 1U).has_value());
}

TEST(Model, OutlineGapsAreEmptyWhenEveryFaceIsStated)
{
    EXPECT_TRUE(outlineGaps(strip()).empty());
    EXPECT_EQ(completed(strip()), strip());
}

TEST(Model, UnstatedFacesBecomeAdiabaticSegments)
{
    auto modelCase{strip()};
    modelCase.segments = {modelCase.segments[0], modelCase.segments[2]};   // the two ends only
    const auto gaps{outlineGaps(modelCase)};
    ASSERT_EQ(gaps.size(), 2U);
    EXPECT_TRUE(std::holds_alternative<Adiabatic>(gaps[0].kind));
    EXPECT_NEAR(gaps[0].start.x, 0.0, 1e-12);   // the bottom, in the edge's drawing direction
    EXPECT_NEAR(gaps[0].end.x, 0.1, 1e-12);
    EXPECT_NEAR(gaps[0].end.y, 0.0, 1e-12);
    EXPECT_NEAR(gaps[1].start.x, 0.1, 1e-12);   // the top
    EXPECT_NEAR(gaps[1].end.x, 0.0, 1e-12);
    EXPECT_NEAR(gaps[1].start.y, 0.01, 1e-12);
    EXPECT_EQ(gaps[0].region, std::optional{std::size_t{0U}});

    const auto full{completed(modelCase)};
    EXPECT_EQ(full.segments.size(), 4U);
    EXPECT_EQ(full.segments[0], modelCase.segments[0]);
    EXPECT_TRUE(outlineGaps(full).empty());
}

TEST(Model, AStatedSpanLeavesTheRestOfItsEdgeToFill)
{
    auto modelCase{strip()};
    modelCase.segments[1] = {.kind = Prescribed{.temperature = 5.0}, .start = {0.02, 0.0}, .end = {0.07, 0.0}};
    const auto gaps{outlineGaps(modelCase)};
    ASSERT_EQ(gaps.size(), 2U);
    EXPECT_NEAR(gaps[0].start.x, 0.0, 1e-12);
    EXPECT_NEAR(gaps[0].end.x, 0.02, 1e-12);
    EXPECT_NEAR(gaps[1].start.x, 0.07, 1e-12);
    EXPECT_NEAR(gaps[1].end.x, 0.1, 1e-12);
}

TEST(Model, ASharedEdgeIsNotOutline)
{
    auto modelCase{strip()};
    modelCase.regions.push_back({.material = stucco(), .points = {{0.1, 0.0}, {0.2, 0.0}, {0.2, 0.01}, {0.1, 0.01}}});
    modelCase.segments = {modelCase.segments[0]};   // only the left end stated
    const auto gaps{outlineGaps(modelCase)};
    // First rectangle: bottom, top. Second: bottom, right end, top. Nothing at x = 0.1.
    ASSERT_EQ(gaps.size(), 5U);
    for(const auto & gap : gaps)
    {
        EXPECT_FALSE(std::abs(gap.start.x - 0.1) < 1e-12 && std::abs(gap.end.x - 0.1) < 1e-12);
    }
}

TEST(Model, ANeighbourCoveringPartOfAnEdgeLeavesTheRestOutline)
{
    auto modelCase{strip()};
    modelCase.segments.clear();
    // A block 0.02 wide sitting on the middle of the strip's top edge.
    modelCase.regions.push_back({.material = stucco(), .points = {{0.04, 0.01}, {0.06, 0.01}, {0.06, 0.02}, {0.04, 0.02}}});
    // Strip: bottom, right, two pieces of the top, left. Block: right, top, left.
    EXPECT_EQ(outlineGaps(modelCase).size(), 8U);
}

TEST(Model, ColoursComeFromTheKindUnlessStated)
{
    EXPECT_EQ(kindColor(Adiabatic{}), "0x000000");
    EXPECT_NE(kindColor(Prescribed{}), kindColor(Convective{}));
    Segment segment{.kind = Convective{}, .start = {0.0, 0.0}, .end = {0.1, 0.0}};
    EXPECT_EQ(segmentColor(segment), kindColor(Convective{}));
    segment.color = "0xE07A1F";
    EXPECT_EQ(segmentColor(segment), "0xE07A1F");
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
