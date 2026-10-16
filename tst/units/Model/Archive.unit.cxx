#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <set>
#include <string>
#include <vector>

#include "Model/Archive.hxx"
#include "Materials/FromValues.hxx"
#include "THMZ/Model/DB.hxx"
#include "THMZ/ZipModule/ZipModule.hxx"
#include "TimeSeriesData/ContentHash.hxx"
#include "TimeSeriesData/DB.hxx"

using namespace ThermFile::Model;
using namespace ThermFile::Build;
using TimeSeriesLibrary::SeriesRole;

namespace
{
    constexpr double tolerance{1e-12};

    MaterialsLibrary::Material stucco()
    {
        return MaterialsLibrary::fromValues("Stucco", 25.0, {{0.0, 0.0}, {0.8, 30.0}, {1.0, 180.0}}, {}, {}, 1800.0, 850.0, 0.8, 0.0, 0.0, 0.3);
    }

    MaterialsLibrary::Material cottaerSandstone()
    {
        return MaterialsLibrary::fromValues("Cottaer Sandstone", 15.0, {{0.0, 0.0}, {0.8, 27.0}, {1.0, 180.0}}, {}, {}, 2050.0, 850.0, 1.8, 0.0, 0.0, 0.22);
    }

    MaterialsLibrary::Material linearSorption()
    {
        return MaterialsLibrary::fromValues("linear-sorption", 10.0, {{0.0, 0.0}, {1.0, 100.0}}, {}, {}, 1000.0, 1000.0, 1.0);
    }

    //! A general 2D model: two rectangles of different materials, six segments.
    ModelCase twoRegionCase()
    {
        return ModelCase{
          .id = "two_regions",
          .regions = {{.material = stucco(), .points = {{0.0, 0.05}, {0.0, 0.0}, {0.02, 0.0}, {0.02, 0.05}}},
                      {.material = cottaerSandstone(),
                       .points = {{0.02, 0.05}, {0.02, 0.0}, {0.10, 0.0}, {0.10, 0.05}}}},
          .segments = {{.kind = Adiabatic{}, .start = {0.0, 0.0}, .end = {0.02, 0.0}, .region = 0U},
                       {.kind = Adiabatic{}, .start = {0.02, 0.0}, .end = {0.10, 0.0}, .region = 1U},
                       {.kind = Convective{.airTemperature = 20.0, .filmCoefficient = 8.0, .humidity = 0.5},
                        .start = {0.10, 0.0},
                        .end = {0.10, 0.05},
                        .region = 1U},
                       {.kind = Adiabatic{}, .start = {0.10, 0.05}, .end = {0.02, 0.05}, .region = 1U},
                       {.kind = Adiabatic{}, .start = {0.02, 0.05}, .end = {0.0, 0.05}, .region = 0U},
                       {.kind = Prescribed{.temperature = -5.0, .humidity = 0.8},
                        .start = {0.0, 0.05},
                        .end = {0.0, 0.0},
                        .region = 0U}},
          .initial = {.temperature = 20.0, .humidity = 0.5},
          .schedule = {.dtime = 3600.0, .nSteps = 24U},
        };
    }

    //! A sealed strip with a temperature gradient imposed end to end: one rectangle, four
    //! segments, prescribed temperature on the two short ends.
    ModelCase sealedStrip()
    {
        return ModelCase{
          .id = "sealed_strip_gradient",
          .regions = {{.material = linearSorption(),
                       .points = {{0.0, 0.0}, {0.1, 0.0}, {0.1, 0.005}, {0.0, 0.005}}}},
          .segments = {{.kind = Adiabatic{}, .start = {0.0, 0.0}, .end = {0.1, 0.0}},
                       {.kind = Prescribed{.temperature = 20.0}, .start = {0.1, 0.0}, .end = {0.1, 0.005}},
                       {.kind = Adiabatic{}, .start = {0.1, 0.005}, .end = {0.0, 0.005}},
                       {.kind = Prescribed{.temperature = 40.0}, .start = {0.0, 0.005}, .end = {0.0, 0.0}}},
          .initial = {.temperature = 30.0, .humidity = 0.4},
          .schedule = {.dtime = 36000.0, .nSteps = 300U},
          .physics = {.liquidTransport = false, .heatOfEvaporation = false},
        };
    }

    //! A steady-state L: a low block beside a tall one, a cold face on the low block's
    //! outer side and a warm face on the tall block's. Every other face is left to the
    //! writer, which makes it adiabatic. No clock: a steady-state case does not need one.
    ModelCase steadyL()
    {
        return ModelCase{
          .id = "steady_l",
          .regions = {{.material = stucco(), .points = {{0.0, 0.0}, {0.09, 0.0}, {0.09, 0.06}, {0.0, 0.06}}},
                      {.material = cottaerSandstone(), .points = {{0.09, 0.0}, {0.17, 0.0}, {0.17, 0.09}, {0.09, 0.09}}}},
          .segments = {{.kind = Convective{.airTemperature = -18.0, .filmCoefficient = 26.0, .humidity = 0.5},
                        .start = {0.0, 0.06},
                        .end = {0.0, 0.0},
                        .color = "0x0080C0"},
                       {.kind = Convective{.airTemperature = 21.0, .filmCoefficient = 8.0, .humidity = 0.5},
                        .start = {0.17, 0.0},
                        .end = {0.17, 0.09}}},
          .physics = {.moisture = false},
          .mode = ThermFile::CalculationMode::cmSteadyState,
          .meshControl = {.errorEnergyNorm = 8.0, .maximumIterations = 4U},
        };
    }

    //! A wall of one material between two faces that read the same weather series: the
    //! room side at a constant temperature, the outside following `outdoor` with a fixed
    //! film coefficient, and a second outside face reading the same series.
    ModelCase weatherWall(const std::vector<double> & outdoor)
    {
        const Convective outside{.airTemperature = outdoor, .filmCoefficient = 25.0, .humidity = 0.8};
        return ModelCase{
          .id = "weather_wall",
          .regions = {{.material = stucco(), .points = {{0.0, 0.0}, {0.3, 0.0}, {0.3, 0.3}, {0.0, 0.3}}}},
          .segments = {{.kind = outside, .start = {0.0, 0.3}, .end = {0.0, 0.0}, .name = "outside"},
                       {.kind = Convective{.airTemperature = 20.0, .filmCoefficient = 8.0, .humidity = 0.5},
                        .start = {0.3, 0.0},
                        .end = {0.3, 0.3},
                        .name = "room side"},
                       {.kind = outside, .start = {0.3, 0.3}, .end = {0.0, 0.3}, .name = "top"}},
          .initial = {.temperature = 20.0, .humidity = 0.5},
          .schedule = {.dtime = 3600.0, .nSteps = static_cast<std::size_t>(outdoor.size())},
        };
    }

    std::vector<double> ramp(const std::size_t count)
    {
        std::vector<double> values(count);
        for(std::size_t index = 0U; index < count; ++index)
        {
            values[index] = -5.0 + static_cast<double>(index) * 0.5;
        }
        return values;
    }

    std::set<double> offsets(const std::vector<ThermFile::Point> & points, const double origin, const bool xAxis)
    {
        std::set<double> found;
        for(const auto & point : points)
        {
            found.insert((xAxis ? point.x : point.y) - origin);
        }
        return found;
    }

    std::vector<std::string> sortedNames(const BCLibrary::DB & records)
    {
        auto names{records.getNames()};
        std::ranges::sort(names);
        return names;
    }

    const BCLibrary::Convection & convectionOf(const BCLibrary::BoundaryCondition & record)
    {
        return std::get<BCLibrary::SurfaceExchange>(record.data).convection.value();
    }
}   // namespace

TEST(Build, SegmentsAttachToTheRegionFoundWhenNoneIsNamed)
{
    auto unnamed{twoRegionCase()};
    for(auto & segment : unnamed.segments)
    {
        segment.region = std::nullopt;
    }
    ASSERT_TRUE(issues(unnamed).empty());
    const auto named{model(twoRegionCase())};
    const auto found{model(unnamed)};
    ASSERT_EQ(found.boundaryConditions.size(), named.boundaryConditions.size());
    for(std::size_t index = 0U; index < named.boundaryConditions.size(); ++index)
    {
        EXPECT_EQ(found.boundaryConditions[index].neighborPolygonUUID, named.boundaryConditions[index].neighborPolygonUUID);
    }
}

TEST(Build, SegmentsAndRecordsCarryTheirColours)
{
    auto stated{sealedStrip()};
    stated.segments = {stated.segments[1], stated.segments[3]};
    stated.segments[0].color = "0xE07A1F";
    const auto built{model(stated)};
    ASSERT_EQ(built.boundaryConditions.size(), 4U);
    EXPECT_EQ(built.boundaryConditions[0].color, "0xE07A1F");
    EXPECT_EQ(built.boundaryConditions[1].color, kindColor(Prescribed{}));
    EXPECT_EQ(built.boundaryConditions[2].color, "0x000000");
    const auto records{libraries(stated).boundaryConditions};
    EXPECT_EQ(records.getByName("segment 1")->Color, "0xE07A1F");
    EXPECT_EQ(records.getByName("segment 2")->Color, kindColor(Prescribed{}));
    EXPECT_EQ(records.getByName("Adiabatic")->Color, "0x000000");
}

TEST(Build, UnstatedFacesAreWrittenAdiabatic)
{
    auto stated{sealedStrip()};
    stated.segments = {stated.segments[1], stated.segments[3]};   // the two prescribed ends
    const auto built{model(stated)};
    ASSERT_EQ(built.boundaryConditions.size(), 4U);
    EXPECT_EQ(built.boundaryConditions[0].name, "segment 1");
    EXPECT_EQ(built.boundaryConditions[2].name, "Adiabatic");
    EXPECT_EQ(built.boundaryConditions[3].name, "Adiabatic");
    EXPECT_EQ(sortedNames(libraries(stated).boundaryConditions),
              (std::vector<std::string>{"Adiabatic", "segment 1", "segment 2"}));
    // Constants everywhere: only the clock.
    EXPECT_EQ(boundaryDatasets(stated).size(), 1U);
}

TEST(Build, RegionsBecomePolygonsInMillimetres)
{
    const auto built{model(twoRegionCase())};
    ASSERT_EQ(built.polygons.size(), 2U);
    EXPECT_EQ(built.polygons[0].materialName, "Stucco");
    EXPECT_EQ(built.polygons[1].materialName, "Cottaer Sandstone");
    EXPECT_EQ(built.polygons[1].polygonType, ThermFile::PolygonType::Material);

    const auto & origin{built.preferences.settings.origin};
    EXPECT_NEAR(origin.x, drawingOriginMm.x, tolerance);
    EXPECT_NEAR(origin.y, drawingOriginMm.y, tolerance);
    EXPECT_EQ(offsets(built.polygons[1].points, origin.x, true), (std::set<double>{20.0, 100.0}));
    EXPECT_EQ(offsets(built.polygons[1].points, origin.y, false), (std::set<double>{0.0, 50.0}));
}

TEST(Build, SegmentsLinkRecordsRegionsAndDatasets)
{
    const auto modelCase{twoRegionCase()};
    const auto datasets{boundaryDatasets(modelCase)};
    const auto built{model(modelCase)};
    const auto libs{libraries(modelCase)};

    ASSERT_EQ(built.boundaryConditions.size(), 6U);
    EXPECT_EQ(built.boundaryConditions[2].name, "segment 3");
    EXPECT_EQ(built.boundaryConditions[5].name, "segment 6");
    for(const auto & boundary : built.boundaryConditions)
    {
        EXPECT_TRUE(libs.boundaryConditions.getByUUID(boundary.bcUUID.value()).has_value());
        EXPECT_EQ(boundary.status, 1);
        EXPECT_TRUE(boundary.isBlocking);
        EXPECT_EQ(boundary.surfaceType, ThermFile::SurfaceType::BoundaryCondition);
    }
    EXPECT_EQ(built.boundaryConditions[2].neighborPolygonUUID, built.polygons[1].uuid);
    EXPECT_EQ(built.boundaryConditions[5].neighborPolygonUUID, built.polygons[0].uuid);

    std::vector<std::size_t> keys;
    for(const auto & [index, data] : datasets)
    {
        keys.push_back(index);
    }
    EXPECT_EQ(keys, (std::vector<std::size_t>{0U}));   // the clock, on the first segment

    std::set<std::string> bound;
    for(const auto & boundary : built.boundaryConditions)
    {
        if(boundary.timeSeriesUUID.has_value())
        {
            bound.insert(boundary.timeSeriesUUID.value());
        }
    }
    std::set<std::string> stored;
    for(const auto & data : libs.datasets)
    {
        stored.insert(data.UUID);
    }
    EXPECT_EQ(bound, stored);
    EXPECT_EQ(libs.materials.getNames(), (std::vector<std::string>{"Stucco", "Cottaer Sandstone"}));
}

TEST(Build, AllAdiabaticModelStillHasAClock)
{
    auto modelCase{twoRegionCase()};
    for(auto & segment : modelCase.segments)
    {
        segment.kind = Adiabatic{};
    }
    const auto datasets{boundaryDatasets(modelCase)};
    ASSERT_EQ(datasets.size(), 1U);
    ASSERT_TRUE(datasets.contains(0U));
    EXPECT_EQ(TimeSeriesLibrary::steps(datasets.at(0U)), 24U);
    EXPECT_EQ(datasets.at(0U).Name, "clock");
}

TEST(Build, AdiabaticIsThermsBuiltInRecord)
{
    const auto records{segmentRecords(sealedStrip())};
    const auto adiabatic{std::ranges::find_if(records, [](const auto & record) { return record.Name == "Adiabatic"; })};
    ASSERT_NE(adiabatic, records.end());
    EXPECT_EQ(adiabatic->UUID, std::string{adiabaticRecordUuid});
    EXPECT_TRUE(adiabatic->Protected);
    EXPECT_FALSE(adiabatic->ProjectName.has_value());
    EXPECT_TRUE(std::holds_alternative<BCLibrary::NoExchange>(adiabatic->data));
    EXPECT_EQ(std::ranges::count_if(records, [](const auto & record) { return record.Name == "Adiabatic"; }), 1);
}

TEST(Build, StripIsOneRectangleWithFourSegments)
{
    const auto built{model(sealedStrip())};
    ASSERT_EQ(built.polygons.size(), 1U);
    ASSERT_EQ(built.boundaryConditions.size(), 4U);
    const auto & origin{built.preferences.settings.origin};
    EXPECT_EQ(offsets(built.polygons[0].points, origin.x, true), (std::set<double>{0.0, 100.0}));
    EXPECT_EQ(offsets(built.polygons[0].points, origin.y, false), (std::set<double>{0.0, 5.0}));
    EXPECT_EQ(built.polygons[0].materialName, "linear-sorption");
    EXPECT_EQ(built.properties.general.fileName, "sealed_strip_gradient");
    EXPECT_EQ(built.properties.general.title, "sealed_strip_gradient");
    EXPECT_TRUE(built.calculationReady);
}

TEST(Build, CalculationOptionsFollowTheCase)
{
    const auto & options{model(sealedStrip()).properties.calculationOptions};
    EXPECT_EQ(options.simulationEngine, ThermFile::SimulationEngine::HygroThermFEM);
    EXPECT_EQ(options.calculationMode, ThermFile::CalculationMode::cmTransient);
    EXPECT_TRUE(options.simulateThermal);
    EXPECT_TRUE(options.simulateMoisture);
    EXPECT_TRUE(options.modelingOptions.excludeWaterLiquidTransportation);
    EXPECT_TRUE(options.modelingOptions.excludeHeatOfEvaporation);
    EXPECT_FALSE(options.modelingOptions.excludeCapillaryConduction);
    EXPECT_FALSE(options.modelingOptions.excludeThermalConductivityTemperatureDependent);
    EXPECT_EQ(options.transientCalculationMethodology, ThermFile::TransientCalculationMethodology::icConstantEverywhere);
    EXPECT_NEAR(options.constantInitialConditionsTransient.temperature, 30.0, tolerance);
    EXPECT_NEAR(options.constantInitialConditionsTransient.humidity, 0.4, tolerance);
    EXPECT_NEAR(options.engineParameters.convergenceTolerance, 1e-5, tolerance);
    EXPECT_NEAR(options.engineParameters.relaxationParameter, 1.0, tolerance);
    EXPECT_EQ(options.heatTransferModelingOptions.maximumIterations, 25U);
    EXPECT_FALSE(options.meshControl.runErrorEstimator);
}

TEST(Build, DatasetsSetTheClock)
{
    const auto libs{libraries(sealedStrip())};
    ASSERT_EQ(libs.datasets.size(), 1U);   // constants everywhere: the clock alone
    const auto schedule{scheduleOf(libs.datasets)};
    EXPECT_EQ(schedule.nSteps, 300U);
    EXPECT_NEAR(schedule.dtime, 36000.0, tolerance);
    EXPECT_TRUE(TimeSeriesLibrary::alignmentIssues(libs.datasets).empty());
    for(const auto & data : libs.datasets)
    {
        EXPECT_EQ(data.ProjectName.value(), "sealed_strip_gradient");
    }
}

TEST(Build, EntriesCarryEveryLibrary)
{
    const auto entries{thmzEntries(sealedStrip())};
    ASSERT_TRUE(entries.has_value());
    for(const auto * name : {"Model.xml", "Materials.xml", "BoundaryConditions.xml", "Gases.xml", "Mesh.xml"})
    {
        EXPECT_TRUE(entries->contains(name)) << name;
    }
    const auto seriesEntries{std::ranges::count_if(
      entries.value(), [](const auto & entry) { return entry.first.starts_with("time series/"); })};
    EXPECT_EQ(seriesEntries, 1);
}

TEST(Build, AnInvalidCaseIsRefusedWithItsIssues)
{
    auto modelCase{sealedStrip()};
    modelCase.regions[0].material.Name.clear();
    modelCase.schedule.nSteps = 0U;

    const auto entries{thmzEntries(modelCase)};
    ASSERT_FALSE(entries.has_value());
    EXPECT_NE(entries.error().find("material with no name"), std::string::npos);
    EXPECT_NE(entries.error().find("at least one step"), std::string::npos);

    const auto written{thmz(modelCase, "should-not-exist.thmz")};
    EXPECT_FALSE(written.has_value());
    EXPECT_FALSE(std::filesystem::exists("should-not-exist.thmz"));
}

TEST(Build, WrittenArchiveRoundTrips)
{
    const auto path{(std::filesystem::temp_directory_path() / "authoring-sealed-strip.thmz").string()};
    const auto written{thmz(sealedStrip(), path)};
    ASSERT_TRUE(written.has_value()) << written.error();
    EXPECT_EQ(written.value(), path);
    ASSERT_TRUE(std::filesystem::exists(path));

    const auto model{ThermFile::loadThermModelFromZipFile(path)};
    ASSERT_TRUE(model.has_value());
    EXPECT_EQ(model->polygons.size(), 1U);
    EXPECT_EQ(model->boundaryConditions.size(), 4U);

    BCLibrary::DB records;
    records.loadFromZipFile(path);
    EXPECT_EQ(sortedNames(records), (std::vector<std::string>{"Adiabatic", "segment 2", "segment 4"}));

    EXPECT_EQ(TimeSeriesLibrary::loadDatasetsFromZipFile(path).size(), 1U);

    MaterialsLibrary::DB materials;
    const std::vector<std::string> wanted{"Materials.xml"};
    materials.loadFromString(ThermZip::unzipFiles(path, wanted).at("Materials.xml"));
    EXPECT_EQ(materials.getNames(), (std::vector<std::string>{"linear-sorption"}));

    // Overwrites in place rather than failing on an existing file.
    EXPECT_TRUE(thmz(sealedStrip(), path).has_value());
    std::filesystem::remove(path);
}

TEST(Build, SteadyStateCaseNeedsNoClock)
{
    EXPECT_TRUE(issues(steadyL()).empty());
    EXPECT_TRUE(isSteadyState(steadyL()));
    EXPECT_TRUE(libraries(steadyL()).datasets.empty());
}

TEST(Build, SteadyStateOptionsCarryTheMeshControl)
{
    const auto modelCase{steadyL()};
    const auto & options{model(modelCase).properties.calculationOptions};
    EXPECT_EQ(options.calculationMode, ThermFile::CalculationMode::cmSteadyState);
    EXPECT_EQ(options.meshControl, modelCase.meshControl);
    EXPECT_TRUE(options.meshControl.runErrorEstimator);
    EXPECT_NEAR(options.meshControl.errorEnergyNorm, 8.0, tolerance);
    EXPECT_EQ(options.meshControl.maximumIterations, 4U);
    EXPECT_FALSE(options.simulateMoisture);
}

TEST(Build, SteadyStateSegmentsHoldTheirConstantsOnRecordsOfTheirOwn)
{
    const auto modelCase{steadyL()};
    auto libs{libraries(modelCase)};
    EXPECT_EQ(sortedNames(libs.boundaryConditions), (std::vector<std::string>{"Adiabatic", "segment 1", "segment 2"}));

    const auto cold{libs.boundaryConditions.getByName("segment 1")};
    ASSERT_TRUE(cold.has_value());
    EXPECT_EQ(cold->Color, "0x0080C0");
    EXPECT_EQ(convectionOf(cold.value()).airTemperature, BCLibrary::Source{BCLibrary::Constant{-18.0}});
    EXPECT_EQ(convectionOf(cold.value()).filmCoefficient, BCLibrary::Source{BCLibrary::Constant{26.0}});

    const auto written{model(modelCase)};
    const auto & first{written.boundaryConditions.front()};
    EXPECT_EQ(first.bcUUID, cold->UUID);
    EXPECT_FALSE(first.timeSeriesUUID.has_value());
}

TEST(Build, SteadyStateEntriesCarryTheUnifiedLibraryOnlyAndNoSeries)
{
    const auto entries{thmzEntries(steadyL())};
    ASSERT_TRUE(entries.has_value()) << entries.error();
    for(const auto * name : {"Model.xml", "BoundaryConditions.xml"})
    {
        EXPECT_TRUE(entries->contains(name)) << name;
    }
    EXPECT_FALSE(entries->contains("SteadyStateBC.xml"));
    EXPECT_FALSE(std::ranges::any_of(entries.value(),
                                     [](const auto & entry) { return entry.first.starts_with("time series/"); }));
}

TEST(Build, SteadyStateRefusesAPrescribedSegment)
{
    auto modelCase{sealedStrip()};
    modelCase.mode = ThermFile::CalculationMode::cmSteadyState;
    const auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 2U);
    EXPECT_NE(found.front().find("is prescribed"), std::string::npos);
}

TEST(Build, SteadyStateRadiationAndTagsAreWritten)
{
    auto modelCase{steadyL()};
    const BCLibrary::BlackBodyRadiation blackBody{.temperature = BCLibrary::Constant{-18.0},
                                                  .emissivity = BCLibrary::Constant{1.0}};
    std::get<Convective>(modelCase.segments[0].kind).radiation = blackBody;
    modelCase.segments[1].uFactorTag = "Room";
    ASSERT_TRUE(issues(modelCase).empty());

    auto libs{libraries(modelCase)};
    const auto cold{libs.boundaryConditions.getByName("segment 1")};
    ASSERT_TRUE(cold.has_value());
    EXPECT_EQ(std::get<BCLibrary::SurfaceExchange>(cold->data).radiation, BCLibrary::RadiationOptions{blackBody});

    const auto written{model(modelCase)};
    EXPECT_EQ(written.boundaryConditions[0].fluxTag, "");
    EXPECT_EQ(written.boundaryConditions[1].fluxTag, "Room");

    // The solver reads the stored emissivity, so the boundary carries the material's, as
    // THERM saves it, with the flag that says where it came from.
    const auto & emission{written.boundaryConditions[0].thermalEmissionProperties};
    EXPECT_NEAR(emission.emissivity, 0.9, tolerance);
    EXPECT_TRUE(emission.useGlobalEmissivity);

    const auto entries{thmzEntries(modelCase)};
    ASSERT_TRUE(entries.has_value()) << entries.error();
}

TEST(Build, SteadyStateRefusesRadiationReadFromADataset)
{
    auto modelCase{steadyL()};
    std::get<Convective>(modelCase.segments[0].kind).radiation = BCLibrary::BlackBodyRadiation{
      .temperature = BCLibrary::FromTimeSeries{SeriesRole::RadiantTemperature}};
    const auto found{issues(modelCase)};
    ASSERT_EQ(found.size(), 1U);
    EXPECT_NE(found.front().find("read from a dataset"), std::string::npos);
}

TEST(Build, AModeThatIsNeitherIsAnIssue)
{
    auto modelCase{sealedStrip()};
    modelCase.mode = ThermFile::CalculationMode::Unknown;
    EXPECT_EQ(issues(modelCase), (std::vector<std::string>{"the mode must be transient or steady state"}));
}

// ------------------------------------------------------------------------------------------
// Lowering: constants stay on the record, series go to a dataset
// ------------------------------------------------------------------------------------------

TEST(Build, ConstantsStayOnTheRecordAndBindNoDataset)
{
    const auto modelCase{twoRegionCase()};
    const auto bindings{segmentBindings(modelCase)};
    ASSERT_EQ(bindings.size(), 6U);
    const auto records{libraries(modelCase).boundaryConditions};
    const auto convective{records.getByUUID(bindings[2].recordUuid)};
    ASSERT_TRUE(convective.has_value());
    EXPECT_EQ(convectionOf(convective.value()).airTemperature, BCLibrary::Source{BCLibrary::Constant{20.0}});
    EXPECT_EQ(convectionOf(convective.value()).filmCoefficient, BCLibrary::Source{BCLibrary::Constant{8.0}});
    EXPECT_TRUE(BCLibrary::isSteadyCapable(convective.value()));
    EXPECT_FALSE(bindings[2].datasetUuid.has_value());
    EXPECT_FALSE(bindings[5].datasetUuid.has_value());
    // The clock rides on the first segment.
    EXPECT_TRUE(bindings[0].datasetUuid.has_value());
}

TEST(Build, SeriesBecomeRolesInADataset)
{
    auto modelCase{twoRegionCase()};
    const auto outdoor{ramp(24U)};
    std::get<Convective>(modelCase.segments[2].kind).airTemperature = outdoor;
    ASSERT_TRUE(issues(modelCase).empty());

    const auto bindings{segmentBindings(modelCase)};
    const auto libs{libraries(modelCase)};
    const auto record{libs.boundaryConditions.getByUUID(bindings[2].recordUuid)};
    ASSERT_TRUE(record.has_value());
    EXPECT_EQ(convectionOf(record.value()).airTemperature,
              BCLibrary::Source{BCLibrary::FromTimeSeries{SeriesRole::AirTemperature}});
    EXPECT_EQ(convectionOf(record.value()).filmCoefficient, BCLibrary::Source{BCLibrary::Constant{8.0}});
    EXPECT_EQ(BCLibrary::requiredRoles(record.value()), (std::vector<SeriesRole>{SeriesRole::AirTemperature}));

    ASSERT_EQ(libs.datasets.size(), 1U);   // the series dataset sets the clock; no clock dataset beside it
    const auto & data{libs.datasets.front()};
    EXPECT_EQ(bindings[2].datasetUuid, std::optional{data.UUID});
    EXPECT_EQ(data.UUID, TimeSeriesLibrary::contentUuid(data));
    EXPECT_EQ(data.Name, "segment 3 series");
    EXPECT_EQ(data.ProjectName, std::optional<std::string>{"two_regions"});
    EXPECT_EQ(data.Source, std::optional<std::string>{"Generated"});
    EXPECT_NEAR(data.axis.stepSeconds, 3600.0, tolerance);
    EXPECT_EQ(TimeSeriesLibrary::providedRoles(data), (std::vector<SeriesRole>{SeriesRole::AirTemperature}));
    EXPECT_EQ(TimeSeriesLibrary::valuesForRole(data, SeriesRole::AirTemperature), std::optional{outdoor});
    EXPECT_FALSE(bindings[0].datasetUuid.has_value());   // no clock dataset when a segment reads series
}

TEST(Build, PrescribedSeriesLowerToTheirRoles)
{
    const auto lowered{lower(Prescribed{.temperature = ramp(24U), .humidity = 0.8})};
    const auto & state{std::get<BCLibrary::PrescribedState>(lowered.data)};
    EXPECT_EQ(state.temperature, std::optional{BCLibrary::Source{BCLibrary::FromTimeSeries{SeriesRole::PrescribedTemperature}}});
    EXPECT_EQ(state.relativeHumidity, std::optional{BCLibrary::Source{BCLibrary::Constant{0.8}}});
    ASSERT_EQ(lowered.series.size(), 1U);
    EXPECT_EQ(lowered.series[0].role, SeriesRole::PrescribedTemperature);
    EXPECT_EQ(lowered.series[0].values, ramp(24U));
    EXPECT_TRUE(std::holds_alternative<BCLibrary::NoExchange>(lower(Adiabatic{}).data));
}

TEST(Build, ConvectiveRecordReadsThreeRolesWhenEveryInputIsASeries)
{
    const auto lowered{lower(Convective{.airTemperature = ramp(4U), .filmCoefficient = ramp(4U), .humidity = ramp(4U)})};
    const BCLibrary::BoundaryCondition record{.data = lowered.data};
    EXPECT_EQ(BCLibrary::requiredRoles(record),
              (std::vector<SeriesRole>{SeriesRole::RelativeHumidity, SeriesRole::AirTemperature, SeriesRole::ConvectiveCoefficient}));
    EXPECT_EQ(lowered.series.size(), 3U);
}

TEST(Build, SegmentsReadingTheSameSeriesShareOneDataset)
{
    const auto modelCase{weatherWall(ramp(48U))};
    ASSERT_TRUE(issues(modelCase).empty());
    const auto libs{libraries(modelCase)};
    ASSERT_EQ(libs.datasets.size(), 1U);
    const auto bindings{segmentBindings(modelCase)};
    EXPECT_EQ(bindings[0].datasetUuid, std::optional{libs.datasets.front().UUID});
    EXPECT_EQ(bindings[2].datasetUuid, bindings[0].datasetUuid);
    EXPECT_FALSE(bindings[1].datasetUuid.has_value());
    // The same values a month apart are different data, so a shifted clock gives a new dataset.
    auto shifted{modelCase};
    shifted.schedule.start.month = 2U;
    EXPECT_NE(libraries(shifted).datasets.front().UUID, libs.datasets.front().UUID);
}

TEST(Build, IdenticalSegmentsShareOneRecordNamedAfterTheFirst)
{
    const auto modelCase{weatherWall(ramp(48U))};
    const auto records{segmentRecords(modelCase)};
    std::vector<std::string> names;
    for(const auto & record : records)
    {
        names.push_back(record.Name);
    }
    EXPECT_EQ(names, (std::vector<std::string>{"outside", "room side", "Adiabatic"}));
    const auto bindings{segmentBindings(modelCase)};
    EXPECT_EQ(bindings[0].recordUuid, bindings[2].recordUuid);
    EXPECT_NE(bindings[0].recordUuid, bindings[1].recordUuid);
    EXPECT_EQ(records[0].ProjectName, std::optional<std::string>{"weather_wall"});
    EXPECT_FALSE(records[0].Protected);
}

TEST(Build, ANameTakenByDifferentContentGetsASuffix)
{
    auto modelCase{weatherWall(ramp(48U))};
    modelCase.segments[2].name = "outside";
    std::get<Convective>(modelCase.segments[2].kind).filmCoefficient = 10.0;
    const auto records{segmentRecords(modelCase)};
    std::vector<std::string> names;
    for(const auto & record : records)
    {
        names.push_back(record.Name);
    }
    EXPECT_EQ(names, (std::vector<std::string>{"outside", "room side", "outside (2)", "Adiabatic"}));
    // Different records, but the same series: still one dataset.
    EXPECT_EQ(libraries(modelCase).datasets.size(), 1U);
}

TEST(Build, DatasetAxisFollowsTheScheduleStart)
{
    auto modelCase{weatherWall(ramp(48U))};
    modelCase.schedule.start = {.month = 2U, .day = 10U, .hour = 6U, .minute = 0U};
    const auto & axis{libraries(modelCase).datasets.front().axis};
    EXPECT_EQ(axis, (TimeSeriesLibrary::TimeAxis{.month = 2U, .day = 10U, .hour = 6U, .minute = 0U, .stepSeconds = 3600.0}));
    EXPECT_EQ(scheduleOf(libraries(modelCase).datasets), modelCase.schedule);
}

TEST(Build, TransientCaseWritesFixedRadiation)
{
    auto modelCase{twoRegionCase()};
    std::get<Convective>(modelCase.segments[2].kind).radiation = FixedRadiation{.temperature = ramp(24U), .coefficient = 4.0};
    ASSERT_TRUE(issues(modelCase).empty());
    const auto bindings{segmentBindings(modelCase)};
    const auto record{libraries(modelCase).boundaryConditions.getByUUID(bindings[2].recordUuid)};
    ASSERT_TRUE(record.has_value());
    const auto & radiation{std::get<BCLibrary::SurfaceExchange>(record->data).radiation};
    const BCLibrary::FixedCoefficientRadiation expected{
      .temperature = BCLibrary::FromTimeSeries{SeriesRole::RadiantTemperature}, .coefficient = BCLibrary::Constant{4.0}};
    EXPECT_EQ(radiation, std::optional{BCLibrary::RadiationOptions{expected}});
    EXPECT_EQ(BCLibrary::requiredRoles(record.value()), (std::vector<SeriesRole>{SeriesRole::RadiantTemperature}));
}
