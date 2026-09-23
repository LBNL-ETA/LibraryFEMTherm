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
#include "TimeSeriesData/DB.hxx"

using namespace ThermFile::Model;
using namespace ThermFile::Build;

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
    EXPECT_EQ(records.getByName("Prescribed temperature")->Color, kindColor(Prescribed{}));
    EXPECT_EQ(records.getByName("Adiabatic")->Color, "0x000000");
}

TEST(Build, UnstatedFacesAreWrittenAdiabatic)
{
    auto stated{sealedStrip()};
    stated.segments = {stated.segments[1], stated.segments[3]};   // the two prescribed ends
    const auto built{model(stated)};
    ASSERT_EQ(built.boundaryConditions.size(), 4U);
    EXPECT_EQ(built.boundaryConditions[0].name, "Prescribed temperature");
    EXPECT_EQ(built.boundaryConditions[2].name, "Adiabatic");
    EXPECT_EQ(built.boundaryConditions[3].name, "Adiabatic");
    auto names{libraries(stated).boundaryConditions.getNames()};
    std::ranges::sort(names);
    EXPECT_EQ(names, (std::vector<std::string>{"Adiabatic", "Prescribed temperature"}));
    EXPECT_EQ(boundaryDatasets(stated).size(), 2U);
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
    EXPECT_EQ(built.boundaryConditions[2].name, "Convective exchange");
    EXPECT_EQ(built.boundaryConditions[5].name, "Prescribed temperature and humidity");
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
    EXPECT_EQ(keys, (std::vector<std::size_t>{2U, 5U}));

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

TEST(Build, ConvectiveRecordReadsThreeRoles)
{
    const auto record{
      boundaryRecord(Convective{.airTemperature = 20.0, .filmCoefficient = 8.0, .humidity = 0.5}, "probe")};
    EXPECT_EQ(record.ProjectName.value(), "probe");
    EXPECT_EQ(BCLibrary::requiredRoles(record),
              (std::vector<TimeSeriesLibrary::SeriesRole>{TimeSeriesLibrary::SeriesRole::RelativeHumidity,
                                                          TimeSeriesLibrary::SeriesRole::AirTemperature,
                                                          TimeSeriesLibrary::SeriesRole::ConvectiveCoefficient}));
}

TEST(Build, RecordUuidsAreTheSameInEveryFile)
{
    EXPECT_EQ(boundaryRecord(Convective{}, "a").UUID, boundaryRecord(Convective{}, "b").UUID);
    EXPECT_NE(boundaryRecord(Adiabatic{}, "a").UUID, boundaryRecord(Convective{}, "a").UUID);
    EXPECT_EQ(recordUuid("Convective exchange"), boundaryRecord(Convective{}, "a").UUID);
}

TEST(Build, AdiabaticIsThermsBuiltInRecord)
{
    const auto record{boundaryRecord(Adiabatic{}, "a")};
    EXPECT_EQ(record.UUID, std::string{adiabaticRecordUuid});
    EXPECT_EQ(record.Name, "Adiabatic");
    EXPECT_TRUE(record.Protected);
    EXPECT_FALSE(record.ProjectName.has_value());
    EXPECT_TRUE(std::holds_alternative<BCLibrary::NoExchange>(record.data));
    EXPECT_EQ(recordUuid("Adiabatic"), std::string{adiabaticRecordUuid});
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
    ASSERT_EQ(libs.datasets.size(), 2U);
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

TEST(Build, AnInvalidCaseIsRefusedWithItsIssues)
{
    auto modelCase{sealedStrip()};
    modelCase.regions[0].material.Name.clear();
    modelCase.schedule.nSteps = 0U;

    const auto entries{archiveEntries(modelCase)};
    ASSERT_FALSE(entries.has_value());
    EXPECT_NE(entries.error().find("material with no name"), std::string::npos);
    EXPECT_NE(entries.error().find("at least one step"), std::string::npos);

    const auto written{archive(modelCase, "should-not-exist.thmz")};
    EXPECT_FALSE(written.has_value());
    EXPECT_FALSE(std::filesystem::exists("should-not-exist.thmz"));
}

TEST(Build, WrittenArchiveRoundTrips)
{
    const auto path{(std::filesystem::temp_directory_path() / "authoring-sealed-strip.thmz").string()};
    const auto written{archive(sealedStrip(), path)};
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
    EXPECT_EQ(names, (std::vector<std::string>{"Adiabatic", "Prescribed temperature"}));

    EXPECT_EQ(TimeSeriesLibrary::loadDatasetsFromZipFile(path).size(), 2U);

    MaterialsLibrary::DB materials;
    const std::vector<std::string> wanted{"Materials.xml"};
    materials.loadFromString(ThermZip::unzipFiles(path, wanted).at("Materials.xml"));
    EXPECT_EQ(materials.getNames(), (std::vector<std::string>{"linear-sorption"}));

    // Overwrites in place rather than failing on an existing file.
    EXPECT_TRUE(archive(sealedStrip(), path).has_value());
    std::filesystem::remove(path);
}
