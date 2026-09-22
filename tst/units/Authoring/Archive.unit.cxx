#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <set>
#include <string>
#include <vector>

#include "Authoring/Archive.hxx"
#include "Authoring/Materials.hxx"
#include "THMZ/Model/DB.hxx"
#include "THMZ/ZipModule/ZipModule.hxx"
#include "TimeSeriesData/DB.hxx"

using namespace ThermFile::Authoring;

namespace
{
    constexpr double tolerance{1e-12};

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

    Material cottaerSandstone()
    {
        return Material{.name = "Cottaer Sandstone",
                        .diffusionResistanceFactor = 15.0,
                        .sorptionCurve = {{0.0, 0.0}, {0.8, 27.0}, {1.0, 180.0}},
                        .density = 2050.0,
                        .heatCapacity = 850.0,
                        .thermalConductivity = 1.8,
                        .porosity = 0.22};
    }

    Material linearSorption()
    {
        return Material{.name = "linear-sorption",
                        .diffusionResistanceFactor = 10.0,
                        .sorptionCurve = {{0.0, 0.0}, {1.0, 100.0}},
                        .density = 1000.0,
                        .heatCapacity = 1000.0,
                        .thermalConductivity = 1.0};
    }

    //! A general 2D model: two rectangles of different materials, six segments.
    ModelCase twoRegionCase()
    {
        return ModelCase{
          .id = "two_regions",
          .regions = {{.material = "stucco", .points = {{0.0, 0.05}, {0.0, 0.0}, {0.02, 0.0}, {0.02, 0.05}}},
                      {.material = "cottaer_sandstone",
                       .points = {{0.02, 0.05}, {0.02, 0.0}, {0.10, 0.0}, {0.10, 0.05}}}},
          .segments = {{.kind = Sealed{}, .start = {0.0, 0.0}, .end = {0.02, 0.0}, .region = 0U},
                       {.kind = Sealed{}, .start = {0.02, 0.0}, .end = {0.10, 0.0}, .region = 1U},
                       {.kind = Convective{.airTemperature = 20.0, .filmCoefficient = 8.0, .humidity = 0.5},
                        .start = {0.10, 0.0},
                        .end = {0.10, 0.05},
                        .region = 1U},
                       {.kind = Sealed{}, .start = {0.10, 0.05}, .end = {0.02, 0.05}, .region = 1U},
                       {.kind = Sealed{}, .start = {0.02, 0.05}, .end = {0.0, 0.05}, .region = 0U},
                       {.kind = Prescribed{.temperature = -5.0, .humidity = 0.8},
                        .start = {0.0, 0.05},
                        .end = {0.0, 0.0},
                        .region = 0U}},
          .initial = {.temperature = 20.0, .humidity = 0.5},
          .schedule = {.dtime = 3600.0, .nSteps = 24U},
          .materials = {{"stucco", stucco()}, {"cottaer_sandstone", cottaerSandstone()}},
        };
    }

    //! A sealed strip with a temperature gradient imposed end to end: one rectangle, four
    //! segments, prescribed temperature on the two short ends.
    ModelCase sealedStrip()
    {
        return ModelCase{
          .id = "sealed_strip_gradient",
          .regions = {{.material = "linear-sorption",
                       .points = {{0.0, 0.0}, {0.1, 0.0}, {0.1, 0.005}, {0.0, 0.005}}}},
          .segments = {{.kind = Sealed{}, .start = {0.0, 0.0}, .end = {0.1, 0.0}},
                       {.kind = Prescribed{.temperature = 20.0}, .start = {0.1, 0.0}, .end = {0.1, 0.005}},
                       {.kind = Sealed{}, .start = {0.1, 0.005}, .end = {0.0, 0.005}},
                       {.kind = Prescribed{.temperature = 40.0}, .start = {0.0, 0.005}, .end = {0.0, 0.0}}},
          .initial = {.temperature = 30.0, .humidity = 0.4},
          .schedule = {.dtime = 36000.0, .nSteps = 300U},
          .physics = {.liquidTransport = false, .heatOfEvaporation = false},
          .materials = {{"linear-sorption", linearSorption()}},
        };
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
}   // namespace

TEST(AuthoringArchive, RegionsBecomePolygonsInMillimetres)
{
    const auto model{buildModel(twoRegionCase())};
    ASSERT_EQ(model.polygons.size(), 2U);
    EXPECT_EQ(model.polygons[0].materialName, "Stucco");
    EXPECT_EQ(model.polygons[1].materialName, "Cottaer Sandstone");
    EXPECT_EQ(model.polygons[1].polygonType, ThermFile::PolygonType::Material);

    const auto & origin{model.preferences.settings.origin};
    EXPECT_NEAR(origin.x, drawingOriginMm.x, tolerance);
    EXPECT_NEAR(origin.y, drawingOriginMm.y, tolerance);
    EXPECT_EQ(offsets(model.polygons[1].points, origin.x, true), (std::set<double>{20.0, 100.0}));
    EXPECT_EQ(offsets(model.polygons[1].points, origin.y, false), (std::set<double>{0.0, 50.0}));
}

TEST(AuthoringArchive, SegmentsLinkRecordsRegionsAndDatasets)
{
    const auto modelCase{twoRegionCase()};
    const auto datasets{boundaryDatasets(modelCase)};
    const auto model{buildModel(modelCase)};
    const auto libraries{buildLibraries(modelCase)};

    ASSERT_EQ(model.boundaryConditions.size(), 6U);
    EXPECT_EQ(model.boundaryConditions[2].name, "Convective exchange");
    EXPECT_EQ(model.boundaryConditions[5].name, "Prescribed temperature and humidity");
    for(const auto & boundary : model.boundaryConditions)
    {
        EXPECT_TRUE(libraries.boundaryConditions.getByUUID(boundary.bcUUID.value()).has_value());
        EXPECT_EQ(boundary.status, 1);
        EXPECT_TRUE(boundary.isBlocking);
        EXPECT_EQ(boundary.surfaceType, ThermFile::SurfaceType::BoundaryCondition);
    }
    EXPECT_EQ(model.boundaryConditions[2].neighborPolygonUUID, model.polygons[1].uuid);
    EXPECT_EQ(model.boundaryConditions[5].neighborPolygonUUID, model.polygons[0].uuid);

    std::vector<std::size_t> keys;
    for(const auto & [index, data] : datasets)
    {
        keys.push_back(index);
    }
    EXPECT_EQ(keys, (std::vector<std::size_t>{2U, 5U}));

    std::set<std::string> bound;
    for(const auto & boundary : model.boundaryConditions)
    {
        if(boundary.timeSeriesUUID.has_value())
        {
            bound.insert(boundary.timeSeriesUUID.value());
        }
    }
    std::set<std::string> stored;
    for(const auto & data : libraries.datasets)
    {
        stored.insert(data.UUID);
    }
    EXPECT_EQ(bound, stored);
    EXPECT_EQ(libraries.materials.getNames(), (std::vector<std::string>{"Stucco", "Cottaer Sandstone"}));
}

TEST(AuthoringArchive, FullySealedModelStillHasAClock)
{
    auto modelCase{twoRegionCase()};
    for(auto & segment : modelCase.segments)
    {
        segment.kind = Sealed{};
    }
    const auto datasets{boundaryDatasets(modelCase)};
    ASSERT_EQ(datasets.size(), 1U);
    ASSERT_TRUE(datasets.contains(0U));
    EXPECT_EQ(TimeSeriesLibrary::steps(datasets.at(0U)), 24U);
    EXPECT_EQ(datasets.at(0U).Name, "clock");
}

TEST(AuthoringArchive, ConvectiveRecordReadsThreeRoles)
{
    const auto record{
      boundaryRecord(Convective{.airTemperature = 20.0, .filmCoefficient = 8.0, .humidity = 0.5}, "probe")};
    EXPECT_EQ(record.ProjectName.value(), "probe");
    EXPECT_EQ(BCLibrary::requiredRoles(record),
              (std::vector<TimeSeriesLibrary::SeriesRole>{TimeSeriesLibrary::SeriesRole::RelativeHumidity,
                                                          TimeSeriesLibrary::SeriesRole::AirTemperature,
                                                          TimeSeriesLibrary::SeriesRole::ConvectiveCoefficient}));
}

TEST(AuthoringArchive, RecordUuidsAreTheSameInEveryFile)
{
    EXPECT_EQ(boundaryRecord(Sealed{}, "a").UUID, boundaryRecord(Sealed{}, "b").UUID);
    EXPECT_NE(boundaryRecord(Sealed{}, "a").UUID, boundaryRecord(Convective{}, "a").UUID);
    EXPECT_EQ(recordUuid("Sealed"), boundaryRecord(Sealed{}, "a").UUID);
}

TEST(AuthoringArchive, StripIsOneRectangleWithFourSegments)
{
    const auto model{buildModel(sealedStrip())};
    ASSERT_EQ(model.polygons.size(), 1U);
    ASSERT_EQ(model.boundaryConditions.size(), 4U);
    const auto & origin{model.preferences.settings.origin};
    EXPECT_EQ(offsets(model.polygons[0].points, origin.x, true), (std::set<double>{0.0, 100.0}));
    EXPECT_EQ(offsets(model.polygons[0].points, origin.y, false), (std::set<double>{0.0, 5.0}));
    EXPECT_EQ(model.polygons[0].materialName, "linear-sorption");
    EXPECT_EQ(model.properties.general.fileName, "sealed_strip_gradient");
    EXPECT_EQ(model.properties.general.title, "sealed_strip_gradient");
    EXPECT_TRUE(model.calculationReady);
}

TEST(AuthoringArchive, CalculationOptionsFollowTheCase)
{
    const auto & options{buildModel(sealedStrip()).properties.calculationOptions};
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

TEST(AuthoringArchive, DatasetsSetTheClock)
{
    const auto libraries{buildLibraries(sealedStrip())};
    ASSERT_EQ(libraries.datasets.size(), 2U);
    const auto schedule{scheduleOf(libraries.datasets)};
    EXPECT_EQ(schedule.nSteps, 300U);
    EXPECT_NEAR(schedule.dtime, 36000.0, tolerance);
    EXPECT_TRUE(TimeSeriesLibrary::alignmentIssues(libraries.datasets).empty());
    for(const auto & data : libraries.datasets)
    {
        EXPECT_EQ(data.ProjectName.value(), "sealed_strip_gradient");
    }
}

TEST(AuthoringArchive, EntriesCarryEveryLibrary)
{
    const auto entries{archiveEntries(sealedStrip())};
    ASSERT_TRUE(entries.has_value());
    for(const auto * name : {"Model.xml", "Materials.xml", "BoundaryConditions.xml", "Gases.xml", "Mesh.xml"})
    {
        EXPECT_TRUE(entries->contains(name)) << name;
    }
    const auto seriesEntries{std::ranges::count_if(
      entries.value(), [](const auto & entry) { return entry.first.starts_with("time series/"); })};
    EXPECT_EQ(seriesEntries, 2);
}

TEST(AuthoringArchive, AnInvalidCaseIsRefusedWithItsIssues)
{
    auto modelCase{sealedStrip()};
    modelCase.regions[0].material = "brick";
    modelCase.schedule.nSteps = 0U;

    const auto entries{archiveEntries(modelCase)};
    ASSERT_FALSE(entries.has_value());
    EXPECT_NE(entries.error().find("names material 'brick'"), std::string::npos);
    EXPECT_NE(entries.error().find("at least one step"), std::string::npos);

    const auto written{writeArchive(modelCase, "should-not-exist.thmz")};
    EXPECT_FALSE(written.has_value());
    EXPECT_FALSE(std::filesystem::exists("should-not-exist.thmz"));
}

TEST(AuthoringArchive, WrittenArchiveRoundTrips)
{
    const auto path{(std::filesystem::temp_directory_path() / "authoring-sealed-strip.thmz").string()};
    const auto written{writeArchive(sealedStrip(), path)};
    ASSERT_TRUE(written.has_value()) << written.error();
    EXPECT_EQ(written.value(), path);
    ASSERT_TRUE(std::filesystem::exists(path));

    const auto model{ThermFile::loadThermModelFromZipFile(path)};
    ASSERT_TRUE(model.has_value());
    EXPECT_EQ(model->polygons.size(), 1U);
    EXPECT_EQ(model->boundaryConditions.size(), 4U);

    BCLibrary::DB records;
    records.loadFromZipFile(path);
    auto names{records.getNames()};
    std::ranges::sort(names);
    EXPECT_EQ(names, (std::vector<std::string>{"Prescribed temperature", "Sealed"}));

    EXPECT_EQ(TimeSeriesLibrary::loadDatasetsFromZipFile(path).size(), 2U);

    MaterialsLibrary::DB materials;
    const std::vector<std::string> wanted{"Materials.xml"};
    materials.loadFromString(ThermZip::unzipFiles(path, wanted).at("Materials.xml"));
    EXPECT_EQ(materials.getNames(), (std::vector<std::string>{"linear-sorption"}));

    // Overwrites in place rather than failing on an existing file.
    EXPECT_TRUE(writeArchive(sealedStrip(), path).has_value());
    std::filesystem::remove(path);
}
