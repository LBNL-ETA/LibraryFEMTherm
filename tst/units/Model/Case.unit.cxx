#include <gtest/gtest.h>

#include <lbnl/algorithm.hxx>

#include "Model/Case.hxx"
#include "Materials/FromValues.hxx"

using namespace ThermFile::Model;
using TimeSeriesLibrary::SeriesRole;
using TimeSeriesLibrary::TimeSeriesData;

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
                       {.kind = Convective{.airTemperature = -5.0,
                                           .filmCoefficient = 8.0,
                                           .humidity = 0.9},
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

    //! A dataset named "weather" of `count` hourly steps from 1 January, one series per role.
    TimeSeriesData providing(const std::vector<SeriesRole> & roles, const std::size_t count = 24U)
    {
        TimeSeriesData data{.UUID = "weather-uuid", .Name = "weather"};
        for(const auto role : roles)
        {
            data.series.push_back({.role = role, .values = hours(count)});
        }
        return data;
    }

    Convective & convectiveEnd(ModelCase & modelCase)
    {
        return std::get<Convective>(modelCase.segments[2].kind);
    }

    std::vector<SeriesRole> rolesOf(const Boundary & boundary)
    {
        return BCLibrary::requiredRoles(BCLibrary::BoundaryCondition{.data = recordData(boundary)});
    }

    //! The case has exactly the one issue, and LibraryFEMTherm words it as given.
    void expectOneIssue(const ModelCase & modelCase, const Issue & expected, const std::string & wording)
    {
        const auto found{issues(modelCase)};
        ASSERT_EQ(found.size(), 1U);
        EXPECT_EQ(found[0], expected);
        EXPECT_EQ(describe(modelCase, found[0]), wording);
    }

    std::vector<IssueKind> kindsOf(const std::vector<Issue> & found)
    {
        return lbnl::transform_to_vector(found, [](const Issue & issue) { return issue.kind; });
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
    expectOneIssue(modelCase, Issue{.kind = IssueKind::NoRegions}, "a model needs at least one region");
}

TEST(Model, TwoPointRegionIsAnIssue)
{
    auto modelCase{strip()};
    modelCase.regions[0].points = {{0.0, 0.0}, {0.1, 0.0}};
    expectOneIssue(modelCase,
                   Issue{.kind = IssueKind::TooFewPoints, .region = 0U},
                   "region 0 has 2 points; a region needs at least three");
}

TEST(Model, SegmentOnMissingRegionIsAnIssue)
{
    auto modelCase{strip()};
    modelCase.segments[1].region = 3U;
    expectOneIssue(modelCase,
                   Issue{.kind = IssueKind::RegionMissing, .region = 3U, .segment = 1U},
                   "segment 1 borders region 3, which does not exist");
}

TEST(Model, SegmentOffTheOutlineIsAnIssue)
{
    auto modelCase{strip()};
    modelCase.segments[1].end = {0.1, 0.002};   // lifted off the bottom edge
    expectOneIssue(modelCase,
                   Issue{.kind = IssueKind::OffEveryRegion, .segment = 1U},
                   "segment 1 from (0, 0) to (0.1, 0.002) does not lie along an edge of any region");
}

TEST(Model, ANamedSegmentIsNamedInItsIssue)
{
    auto modelCase{strip()};
    modelCase.segments[1].name = "bottom";
    modelCase.segments[1].end = {0.1, 0.002};
    expectOneIssue(modelCase,
                   Issue{.kind = IssueKind::OffEveryRegion, .segment = 1U},
                   "segment 1 'bottom' from (0, 0) to (0.1, 0.002) does not lie along an edge of any region");
}

TEST(Model, NamedRegionIsCheckedAsNamed)
{
    auto modelCase{strip()};
    modelCase.segments[1].region = 0U;
    modelCase.segments[1].end = {0.1, 0.002};
    expectOneIssue(modelCase,
                   Issue{.kind = IssueKind::OffRegion, .region = 0U, .segment = 1U},
                   "segment 1 from (0, 0) to (0.1, 0.002) does not lie along an edge of region 0");
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
    expectOneIssue(
      modelCase,
      Issue{.kind = IssueKind::SharedEdge, .region = 0U, .otherRegion = 1U, .segment = 2U},
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
    expectOneIssue(modelCase,
                   Issue{.kind = IssueKind::UnnamedMaterial, .region = 0U},
                   "region 0 has a material with no name");
}

TEST(Model, OneNameWithTwoRecordsIsAnIssue)
{
    auto modelCase{strip()};
    auto another{stucco()};
    another.UUID = "another-record";
    modelCase.regions.push_back({.material = another, .points = {{0.1, 0.0}, {0.2, 0.0}, {0.2, 0.01}, {0.1, 0.01}}});
    modelCase.segments[2].region = 0U;   // the shared edge, attributed
    expectOneIssue(modelCase,
                   Issue{.kind = IssueKind::MaterialNameClash, .region = 0U, .otherRegion = 1U},
                   "regions 0 and 1 carry two different materials both named 'Stucco'; the library keeps one "
                   "record per name");

    modelCase.regions[1].material = stucco();
    EXPECT_TRUE(issues(modelCase).empty());
}

TEST(Model, ANonPositiveStepIsAnIssue)
{
    auto modelCase{strip()};
    modelCase.schedule = uniformSchedule(0.0, 3U);
    expectOneIssue(modelCase,
                   Issue{.kind = IssueKind::NonPositiveStep, .step = 0U},
                   "the schedule's step 1 must be positive");
    modelCase.schedule.stepLengths = {3600.0, -1.0, 3600.0};
    expectOneIssue(modelCase,
                   Issue{.kind = IssueKind::NonPositiveStep, .step = 1U},
                   "the schedule's step 2 must be positive");
    modelCase.schedule = {};
    expectOneIssue(modelCase, Issue{.kind = IssueKind::NoSteps}, "the schedule needs at least one step");
}

TEST(Model, IssuesAccumulate)
{
    auto modelCase{strip()};
    modelCase.regions[0].material.Name.clear();
    modelCase.segments[0].region = 9U;
    modelCase.schedule.stepLengths.clear();
    EXPECT_EQ(kindsOf(issues(modelCase)),
              (std::vector<IssueKind>{IssueKind::UnnamedMaterial, IssueKind::RegionMissing, IssueKind::NoSteps}));
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

    // What a caller gets for every field it leaves out.
    const Convective convective;
    EXPECT_EQ(convective.humidity, BCLibrary::Source{0.5});
    EXPECT_EQ(convective.model, BCLibrary::ConvectionModel::Fixed_Convection_Coefficient);

    const Segment segment;
    EXPECT_TRUE(std::holds_alternative<Adiabatic>(segment.kind));
    EXPECT_FALSE(segment.dataset.has_value());

    const Schedule schedule;
    EXPECT_EQ(schedule.start, (Start{.month = 1U, .day = 1U, .hour = 1U, .minute = 0U}));
}

TEST(Model, CasesCompareByValue)
{
    EXPECT_EQ(strip(), strip());
    auto changed{strip()};
    changed.initial.humidity = 0.6;
    EXPECT_NE(changed, strip());
}

// ------------------------------------------------------------------------------------------
// Inputs: constants and roles, and the dataset a segment binds
// ------------------------------------------------------------------------------------------

TEST(Model, RecordDataCarriesTheInputsAsStated)
{
    const Convective everything{
      .airTemperature = SeriesRole::AirTemperature,
      .filmCoefficient = SeriesRole::ConvectiveCoefficient,
      .humidity = SeriesRole::RelativeHumidity,
      .windSpeed = SeriesRole::WindSpeed,
      .windDirection = SeriesRole::WindDirection,
      .flux = SeriesRole::HeatFlux,
      .radiation = BCLibrary::FixedCoefficientRadiation{.temperature = SeriesRole::RadiantTemperature,
                                                        .coefficient = SeriesRole::RadiativeCoefficient}};
    EXPECT_EQ(rolesOf(everything),
              (std::vector<SeriesRole>{SeriesRole::RelativeHumidity,
                                       SeriesRole::AirTemperature,
                                       SeriesRole::ConvectiveCoefficient,
                                       SeriesRole::WindSpeed,
                                       SeriesRole::WindDirection,
                                       SeriesRole::RadiantTemperature,
                                       SeriesRole::RadiativeCoefficient,
                                       SeriesRole::HeatFlux}));
    const auto exchange{std::get<BCLibrary::SurfaceExchange>(recordData(everything))};
    EXPECT_EQ(exchange.convection->airTemperature, std::optional{BCLibrary::Source{SeriesRole::AirTemperature}});

    // Constants read nothing; the kinds map to THERM's record shapes.
    EXPECT_TRUE(rolesOf(Convective{.airTemperature = -5.0, .filmCoefficient = 8.0}).empty());
    EXPECT_EQ(rolesOf(Prescribed{.temperature = SeriesRole::PrescribedTemperature, .humidity = 0.8}),
              (std::vector<SeriesRole>{SeriesRole::PrescribedTemperature}));
    EXPECT_TRUE(std::holds_alternative<BCLibrary::PrescribedState>(recordData(Prescribed{})));
    EXPECT_TRUE(std::holds_alternative<BCLibrary::NoExchange>(recordData(Adiabatic{})));
}

TEST(Model, ASegmentReadingARoleNeedsADataset)
{
    auto modelCase{strip()};
    convectiveEnd(modelCase).airTemperature = SeriesRole::AirTemperature;
    expectOneIssue(modelCase,
                   Issue{.kind = IssueKind::RoleWithoutDataset, .segment = 2U, .roles = {SeriesRole::AirTemperature}},
                   "segment 2 from (0.1, 0) to (0.1, 0.01) reads AirTemperature from a dataset but binds none");

    modelCase.segments[2].dataset = providing({SeriesRole::AirTemperature});
    EXPECT_TRUE(issues(modelCase).empty());
}

TEST(Model, TheDatasetMustProvideEveryRoleRead)
{
    auto modelCase{strip()};
    convectiveEnd(modelCase).airTemperature = SeriesRole::AirTemperature;
    convectiveEnd(modelCase).humidity = SeriesRole::RelativeHumidity;
    modelCase.segments[2].dataset = providing({SeriesRole::AirTemperature});
    expectOneIssue(modelCase,
                   Issue{.kind = IssueKind::RoleNotProvided, .segment = 2U, .roles = {SeriesRole::RelativeHumidity}},
                   "segment 2 from (0.1, 0) to (0.1, 0.01): its dataset 'weather' provides no RelativeHumidity");
}

TEST(Model, AStatedScheduleMustAgreeWithTheDatasets)
{
    auto modelCase{strip()};
    convectiveEnd(modelCase).airTemperature = SeriesRole::AirTemperature;
    modelCase.segments[2].dataset = providing({SeriesRole::AirTemperature}, 10U);
    expectOneIssue(modelCase,
                   Issue{.kind = IssueKind::ScheduleDisagrees, .segment = 2U},
                   "segment 2 from (0.1, 0) to (0.1, 0.01): the stated schedule differs from the axis of its dataset "
                   "'weather' (1 Jan 00:00, step 1 h, 10 rows); leave the schedule out to run on the datasets' clock");

    // Left unstated, the schedule is the datasets'.
    modelCase.schedule = Schedule{};
    EXPECT_TRUE(issues(modelCase).empty());
}

TEST(Model, DatasetsAreNotComparedWithOneAnother)
{
    // Two datasets that do not align are for TimeSeriesLibrary::alignmentIssues, and for
    // HygroTherm before it solves; the case states what it binds.
    auto modelCase{strip()};
    modelCase.schedule = Schedule{};
    std::get<Prescribed>(modelCase.segments[0].kind).temperature = SeriesRole::PrescribedTemperature;
    modelCase.segments[0].dataset = providing({SeriesRole::PrescribedTemperature}, 24U);
    convectiveEnd(modelCase).airTemperature = SeriesRole::AirTemperature;
    modelCase.segments[2].dataset = providing({SeriesRole::AirTemperature}, 10U);
    EXPECT_TRUE(issues(modelCase).empty());
}

TEST(Model, ASchedulWithNoStepsIsReportedWhenNoDatasetIsBound)
{
    auto modelCase{strip()};
    modelCase.schedule.stepLengths.clear();
    EXPECT_EQ(issues(modelCase), (std::vector<Issue>{Issue{.kind = IssueKind::NoSteps}}));
}

TEST(Model, TheConvectionModelMustBeGivenItsInputs)
{
    auto modelCase{strip()};
    convectiveEnd(modelCase).filmCoefficient = std::nullopt;
    expectOneIssue(
      modelCase,
      Issue{.kind = IssueKind::ConvectionInputMissing, .segment = 2U, .input = ConvectionInput::FilmCoefficient},
      "segment 2 from (0.1, 0) to (0.1, 0.01) needs a film coefficient for its convection model");

    convectiveEnd(modelCase).model = BCLibrary::ConvectionModel::ASHRAE_NFRC_Outside;
    expectOneIssue(modelCase,
                   Issue{.kind = IssueKind::ConvectionInputMissing, .segment = 2U, .input = ConvectionInput::WindSpeed},
                   "segment 2 from (0.1, 0) to (0.1, 0.01) needs the wind speed for its convection model");

    convectiveEnd(modelCase).windSpeed = 3.0;
    EXPECT_TRUE(issues(modelCase).empty());

    convectiveEnd(modelCase).model = BCLibrary::ConvectionModel::Kimura;
    expectOneIssue(
      modelCase,
      Issue{.kind = IssueKind::ConvectionInputMissing, .segment = 2U, .input = ConvectionInput::WindDirection},
      "segment 2 from (0.1, 0) to (0.1, 0.01) needs the wind direction for its convection model");

    convectiveEnd(modelCase).windDirection = SeriesRole::WindDirection;
    modelCase.segments[2].dataset = providing({SeriesRole::WindDirection});
    EXPECT_TRUE(issues(modelCase).empty());

    // The inside correlations read no wind at all.
    convectiveEnd(modelCase) = Convective{.airTemperature = 20.0, .model = BCLibrary::ConvectionModel::TARP};
    EXPECT_TRUE(issues(modelCase).empty());
}

TEST(Model, TransientAllowsFixedRadiationFromADataset)
{
    auto modelCase{strip()};
    convectiveEnd(modelCase).radiation = BCLibrary::FixedCoefficientRadiation{
      .temperature = SeriesRole::RadiantTemperature, .coefficient = 4.5};
    modelCase.segments[2].dataset = providing({SeriesRole::RadiantTemperature});
    EXPECT_TRUE(issues(modelCase).empty());

    modelCase.segments[2].dataset = providing({SeriesRole::RelativeHumidity});
    EXPECT_EQ(issues(modelCase),
              (std::vector<Issue>{
                Issue{.kind = IssueKind::RoleNotProvided, .segment = 2U, .roles = {SeriesRole::RadiantTemperature}}}));
}

TEST(Model, TransientRefusesWhatTheEngineDoesNotRunThroughAClock)
{
    auto modelCase{strip()};
    modelCase.segments[2].dataset = providing({SeriesRole::RadiantTemperature});
    convectiveEnd(modelCase).radiation = BCLibrary::AutomaticEnclosure{};
    expectOneIssue(modelCase,
                   Issue{.kind = IssueKind::EnclosureInTransient, .segment = 2U},
                   "segment 2 from (0.1, 0) to (0.1, 0.01) radiates into an enclosure, which a transient run does "
                   "not model; state fixed radiation or a black body");

    convectiveEnd(modelCase).radiation = BCLibrary::BlackBodyRadiation{
      .temperature = SeriesRole::RadiantTemperature};
    expectOneIssue(modelCase,
                   Issue{.kind = IssueKind::BlackBodyFromDataset, .segment = 2U},
                   "segment 2 from (0.1, 0) to (0.1, 0.01) radiates as a black body read from a series, which a "
                   "transient run holds constant; state constants, or fixed-coefficient radiation to read a dataset");

    convectiveEnd(modelCase).radiation = BCLibrary::BlackBodyRadiation{.temperature = -10.0};
    EXPECT_TRUE(issues(modelCase).empty());
}

TEST(Model, SteadyStateRefusesAnyRoleAndAnyDataset)
{
    auto modelCase{strip()};
    modelCase.mode = ThermFile::CalculationMode::cmSteadyState;
    modelCase.segments[0].kind = Convective{.airTemperature = 20.0, .filmCoefficient = 8.0};
    EXPECT_TRUE(issues(modelCase).empty());

    convectiveEnd(modelCase).airTemperature = SeriesRole::AirTemperature;
    expectOneIssue(modelCase,
                   Issue{.kind = IssueKind::RoleInSteadyState, .segment = 2U, .roles = {SeriesRole::AirTemperature}},
                   "segment 2 from (0.1, 0) to (0.1, 0.01) reads AirTemperature from a dataset, which a steady-state "
                   "case has none of; state it as a constant");

    convectiveEnd(modelCase).airTemperature = -5.0;
    modelCase.segments[2].dataset = providing({SeriesRole::AirTemperature});
    expectOneIssue(modelCase,
                   Issue{.kind = IssueKind::DatasetInSteadyState, .segment = 2U},
                   "segment 2 from (0.1, 0) to (0.1, 0.01) binds a dataset, which a steady-state case does not write");
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
      .axis = {.month = 3U, .day = 2U, .stepSeconds = std::vector<double>{900.0, 1800.0}},
      .series = {{.role = SeriesRole::AirTemperature, .values = hours(2U)}}};
    const auto schedule{scheduleFor(dataset)};
    EXPECT_EQ(schedule, (Schedule{.start = {.month = 3U, .day = 2U}, .stepLengths = {900.0, 1800.0}}));
    EXPECT_EQ(uniformStep(schedule), std::nullopt);
    EXPECT_NEAR(schedule.duration(), 2700.0, 1e-9);
    EXPECT_EQ(axisOf(schedule), dataset.axis);
    // Stated evenly, the axis is the plain even one.
    EXPECT_EQ(axisOf(Schedule{.stepLengths = {3600.0, 3600.0}}), TimeSeriesLibrary::TimeAxis{});
}
