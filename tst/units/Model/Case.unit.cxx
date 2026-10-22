#include <gtest/gtest.h>

#include "Model/Case.hxx"
#include "Materials/FromValues.hxx"

using namespace ThermFile::Model;
using TimeSeriesLibrary::SeriesRole;

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
          .schedule = uniformSchedule(3600.0, 24U),
        };
    }

    std::vector<double> hours(const std::size_t count)
    {
        std::vector<double> values(count);
        for(std::size_t index = 0U; index < count; ++index)
        {
            values[index] = static_cast<double>(index);
        }
        return values;
    }

    Convective & convectiveEnd(ModelCase & modelCase)
    {
        return std::get<Convective>(modelCase.segments[2].kind);
    }

    std::vector<SeriesRole> rolesOf(const Boundary & boundary)
    {
        std::vector<SeriesRole> roles;
        for(const auto & input : inputs(boundary))
        {
            roles.push_back(input.role);
        }
        return roles;
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

TEST(Model, ANonPositiveStepIsAnIssue)
{
    auto modelCase{strip()};
    modelCase.schedule = uniformSchedule(0.0, 3U);
    EXPECT_EQ(issues(modelCase), (std::vector<std::string>{"the schedule's step 1 must be positive"}));
    modelCase.schedule.stepLengths = {3600.0, -1.0, 3600.0};
    EXPECT_EQ(issues(modelCase), (std::vector<std::string>{"the schedule's step 2 must be positive"}));
    modelCase.schedule = {};
    EXPECT_EQ(issues(modelCase), (std::vector<std::string>{"the schedule needs at least one step"}));
}

TEST(Model, IssuesAccumulate)
{
    auto modelCase{strip()};
    modelCase.regions[0].material.Name.clear();
    modelCase.segments[0].region = 9U;
    modelCase.schedule.stepLengths.clear();
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

    EXPECT_EQ(Convective{}.humidity, Input{0.5});
    EXPECT_EQ(Convective{}.model, BCLibrary::ConvectionModel::Fixed_Convection_Coefficient);
    EXPECT_TRUE(std::holds_alternative<Adiabatic>(Segment{}.kind));
    EXPECT_EQ(Schedule{}.start, (Start{.month = 1U, .day = 1U, .hour = 0U, .minute = 0U}));
}

TEST(Model, CasesCompareByValue)
{
    EXPECT_EQ(strip(), strip());
    auto changed{strip()};
    changed.initial.humidity = 0.6;
    EXPECT_NE(changed, strip());
}

// ------------------------------------------------------------------------------------------
// Inputs: constants and series
// ------------------------------------------------------------------------------------------

TEST(Model, AnInputIsAConstantOrASeries)
{
    EXPECT_TRUE(isConstant(Input{20.0}));
    EXPECT_FALSE(seriesLength(Input{20.0}).has_value());
    EXPECT_FALSE(isConstant(Input{hours(24U)}));
    EXPECT_EQ(seriesLength(Input{hours(24U)}), std::optional{std::size_t{24U}});
}

TEST(Model, InputsListEveryRoleOnce)
{
    const Convective everything{.airTemperature = -5.0,
                                .filmCoefficient = 8.0,
                                .humidity = 0.9,
                                .windSpeed = 3.0,
                                .windDirection = 180.0,
                                .flux = 50.0,
                                .radiation = FixedRadiation{.temperature = -10.0, .coefficient = 4.5}};
    EXPECT_EQ(rolesOf(everything),
              (std::vector<SeriesRole>{SeriesRole::RelativeHumidity,
                                       SeriesRole::AirTemperature,
                                       SeriesRole::ConvectiveCoefficient,
                                       SeriesRole::WindSpeed,
                                       SeriesRole::WindDirection,
                                       SeriesRole::RadiantTemperature,
                                       SeriesRole::RadiativeCoefficient,
                                       SeriesRole::HeatFlux}));
    EXPECT_EQ(rolesOf(Convective{.airTemperature = -5.0, .filmCoefficient = 8.0}),
              (std::vector<SeriesRole>{SeriesRole::RelativeHumidity, SeriesRole::AirTemperature, SeriesRole::ConvectiveCoefficient}));
    EXPECT_EQ(rolesOf(Prescribed{.temperature = 20.0, .humidity = 0.8}),
              (std::vector<SeriesRole>{SeriesRole::PrescribedTemperature, SeriesRole::PrescribedHumidity}));
    EXPECT_EQ(rolesOf(Prescribed{.temperature = 20.0}), (std::vector<SeriesRole>{SeriesRole::PrescribedTemperature}));
    EXPECT_TRUE(inputs(Adiabatic{}).empty());
    EXPECT_EQ(inputs(everything)[1].value, Input{-5.0});
}

TEST(Model, SeriesInputMustMatchTheSchedule)
{
    auto modelCase{strip()};
    convectiveEnd(modelCase).airTemperature = hours(10U);
    const auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 1U);
    EXPECT_EQ(found[0], "segment 2 from (0.1, 0) to (0.1, 0.01): AirTemperature has 10 values, the schedule has 24 steps");

    convectiveEnd(modelCase).airTemperature = hours(24U);
    EXPECT_TRUE(issues(modelCase).empty());
}

TEST(Model, EachMismatchedSeriesIsOneIssue)
{
    auto modelCase{strip()};
    convectiveEnd(modelCase).airTemperature = hours(10U);
    convectiveEnd(modelCase).humidity = hours(5U);
    std::get<Prescribed>(modelCase.segments[0].kind).temperature = hours(24U);   // this one fits
    EXPECT_EQ(issues(modelCase).size(), 2U);
}

TEST(Model, ASchedulWithNoStepsReportsItselfNotEverySeries)
{
    auto modelCase{strip()};
    modelCase.schedule.stepLengths.clear();
    convectiveEnd(modelCase).airTemperature = hours(10U);
    EXPECT_EQ(issues(modelCase), (std::vector<std::string>{"the schedule needs at least one step"}));
}

TEST(Model, TheConvectionModelMustBeGivenItsInputs)
{
    auto modelCase{strip()};
    convectiveEnd(modelCase).filmCoefficient = std::nullopt;
    auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 1U);
    EXPECT_EQ(found[0], "segment 2 from (0.1, 0) to (0.1, 0.01) needs a film coefficient for its convection model");

    convectiveEnd(modelCase).model = BCLibrary::ConvectionModel::ASHRAE_NFRC_Outside;
    found = issues(modelCase);
    ASSERT_EQ(found.size(), 1U);
    EXPECT_NE(found[0].find("needs the wind speed"), std::string::npos);

    convectiveEnd(modelCase).windSpeed = 3.0;
    EXPECT_TRUE(issues(modelCase).empty());

    convectiveEnd(modelCase).model = BCLibrary::ConvectionModel::Kimura;
    found = issues(modelCase);
    ASSERT_EQ(found.size(), 1U);
    EXPECT_NE(found[0].find("needs the wind direction"), std::string::npos);

    convectiveEnd(modelCase).windDirection = hours(24U);
    EXPECT_TRUE(issues(modelCase).empty());

    // The inside correlations read no wind at all.
    convectiveEnd(modelCase) = Convective{.airTemperature = 20.0, .model = BCLibrary::ConvectionModel::TARP};
    EXPECT_TRUE(issues(modelCase).empty());
}

TEST(Model, TransientAllowsFixedRadiationFromSeries)
{
    auto modelCase{strip()};
    convectiveEnd(modelCase).radiation = FixedRadiation{.temperature = hours(24U), .coefficient = 4.5};
    EXPECT_TRUE(issues(modelCase).empty());

    convectiveEnd(modelCase).radiation = FixedRadiation{.temperature = hours(12U), .coefficient = 4.5};
    const auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 1U);
    EXPECT_NE(found[0].find("RadiantTemperature has 12 values"), std::string::npos);
}

TEST(Model, TransientRefusesWhatTheEngineDoesNotRunThroughAClock)
{
    auto modelCase{strip()};
    convectiveEnd(modelCase).radiation = BCLibrary::AutomaticEnclosure{};
    auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 1U);
    EXPECT_NE(found[0].find("radiates into an enclosure"), std::string::npos);

    convectiveEnd(modelCase).radiation = BCLibrary::BlackBodyRadiation{
      .temperature = BCLibrary::FromTimeSeries{SeriesRole::RadiantTemperature}};
    found = issues(modelCase);
    ASSERT_EQ(found.size(), 1U);
    EXPECT_NE(found[0].find("black body read from a series"), std::string::npos);

    convectiveEnd(modelCase).radiation = BCLibrary::BlackBodyRadiation{.temperature = BCLibrary::Constant{-10.0}};
    EXPECT_TRUE(issues(modelCase).empty());
}

TEST(Model, SteadyStateRefusesAnySeriesInput)
{
    auto modelCase{strip()};
    modelCase.mode = ThermFile::CalculationMode::cmSteadyState;
    modelCase.segments[0].kind = Convective{.airTemperature = 20.0, .filmCoefficient = 8.0};
    EXPECT_TRUE(issues(modelCase).empty());

    convectiveEnd(modelCase).airTemperature = hours(24U);
    const auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 1U);
    EXPECT_EQ(found[0],
              "segment 2 from (0.1, 0) to (0.1, 0.01) reads AirTemperature from a series, which a steady-state case "
              "has no datasets for; state it as a constant");
}

// ------------------------------------------------------------------------------------------
// The schedule and the dataset axis
// ------------------------------------------------------------------------------------------

TEST(Model, ScheduleForDatasetCopiesTheAxis)
{
    const TimeSeriesLibrary::TimeSeriesData dataset{
      .axis = {.month = 3U, .day = 2U, .hour = 6U, .minute = 30U, .stepSeconds = 1800.0},
      .series = {{.role = SeriesRole::AirTemperature, .values = hours(48U)}}};
    const auto schedule{scheduleFor(dataset)};
    EXPECT_EQ(schedule, uniformSchedule(1800.0, 48U, {.month = 3U, .day = 2U, .hour = 6U, .minute = 30U}));
    EXPECT_EQ(nSteps(schedule), 48U);
    EXPECT_EQ(uniformStep(schedule), std::optional{1800.0});
    EXPECT_EQ(axisOf(schedule), dataset.axis);
}

TEST(Model, AnUnevenScheduleStatesEveryStep)
{
    const TimeSeriesLibrary::TimeSeriesData dataset{
      .axis = {.month = 3U, .day = 2U, .stepSeconds = 900.0, .stepLengths = {900.0, 1800.0}},
      .series = {{.role = SeriesRole::AirTemperature, .values = hours(2U)}}};
    const auto schedule{scheduleFor(dataset)};
    EXPECT_EQ(schedule, (Schedule{.start = {.month = 3U, .day = 2U}, .stepLengths = {900.0, 1800.0}}));
    EXPECT_EQ(uniformStep(schedule), std::nullopt);
    EXPECT_NEAR(schedule.duration(), 2700.0, 1e-9);
    EXPECT_EQ(axisOf(schedule), dataset.axis);
    // Stated evenly, the axis is the plain even one.
    EXPECT_EQ(axisOf(Schedule{.stepLengths = {3600.0, 3600.0}}), TimeSeriesLibrary::TimeAxis{});
}
