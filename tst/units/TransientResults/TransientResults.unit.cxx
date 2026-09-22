#include <gtest/gtest.h>

#include <filesystem>
#include <map>
#include <set>
#include <string>

#include "GeometryForResults/DB.hxx"
#include "THMZ/ZipModule/ZipModule.hxx"
#include "TransientResults/TransientResults.hxx"

using namespace ThermFile::TransientResults;

namespace
{
    constexpr double tolerance{1e-12};

    //! A one-element isothermal strip solved by HygroTherm for 24 hourly steps: 22 nodes,
    //! 10 elements, constant 20 C / 0.7 / 9.6 kg/m3 throughout, every CSV present, rows
    //! numbered from the initial-conditions frame 0.
    std::string fixturePath()
    {
        std::filesystem::path path{TEST_DATA_DIR};
        path /= "products";
        path /= "transient-isothermal-strip.thmz";
        return path.string();
    }

    std::string geometryEntry()
    {
        return ThermZip::entryNameForFormat(ThermZip::getGeometryFileName(), FileParse::FileFormat::XML);
    }

    //! A unit square of four nodes and one element, as the results geometry stores it.
    std::string squareGeometry()
    {
        GeometryLibrary::InputGeometryDataRecord geometry;
        geometry.version = "1";
        geometry.add(1U, "Stucco", "0x808080");
        geometry.addNode(1U, 0.0, 0.0);
        geometry.addNode(2U, 1.0, 0.0);
        geometry.addNode(3U, 1.0, 1.0);
        geometry.addNode(4U, 0.0, 1.0);
        geometry.addElement(1U, 1U, 2U, 3U, 4U, 1U);
        return geometry.saveToString();
    }

    //! Two frames numbered from 1, as archives written before the initial frame existed.
    std::map<std::string, std::string> olderEntries()
    {
        return {{geometryEntry(), squareGeometry()},
                {ThermZip::getTemperatureFileName(),
                 "timestep #,error tolerance,1,2,3,4\r\n1,1e-6,10,11,12,13\r\n2,2e-6,20,21,22,23\r\n"},
                {ThermZip::getHeatFluxFileName(), "1-x,1-y,2-x,2-y,3-x,3-y,4-x,4-y\n0,0,1,0,2,0,3,0\n0,0,1,1,2,2,3,3\n"}};
    }
}   // namespace

TEST(TransientResults, FixtureReadsCompletely)
{
    const auto results{readFromZipFile(fixturePath())};
    ASSERT_TRUE(results.has_value()) << results.error();
    const auto & read{results.value()};

    EXPECT_EQ(read.nodeCount(), 22U);
    EXPECT_EQ(read.geometry.elements.size(), 10U);
    EXPECT_EQ(read.frameCount(), 25U);
    EXPECT_TRUE(read.firstFrameIsInitial);
    EXPECT_EQ(read.frames.front(), 0U);
    EXPECT_EQ(read.frames.back(), 24U);

    EXPECT_TRUE(read.has(Quantity::Temperature));
    EXPECT_TRUE(read.has(Quantity::Humidity));
    EXPECT_TRUE(read.has(Quantity::WaterContent));
    EXPECT_FALSE(read.has(Quantity::IceContent));

    for(std::size_t frame = 0U; frame < read.frameCount(); ++frame)
    {
        for(std::size_t column = 0U; column < read.nodeCount(); ++column)
        {
            EXPECT_NEAR(read.nodal.at(Quantity::Temperature).values[frame][column], 20.0, tolerance);
            EXPECT_NEAR(read.nodal.at(Quantity::Humidity).values[frame][column], 0.7, tolerance);
            EXPECT_NEAR(read.nodal.at(Quantity::WaterContent).values[frame][column], 9.6, tolerance);
        }
    }
    EXPECT_EQ(read.nodal.at(Quantity::Temperature).solveError.size(), 25U);
    EXPECT_EQ(read.heatFlux.size(), 25U);
    EXPECT_EQ(read.heatFlux.front().size(), 22U);
    EXPECT_EQ(read.waterFlux.size(), 25U);
}

TEST(TransientResults, NodesAreInColumnOrderWithPositions)
{
    const auto results{readFromZipFile(fixturePath())};
    ASSERT_TRUE(results.has_value());
    const auto & nodes{results.value().nodes};
    for(std::size_t column = 0U; column < nodes.size(); ++column)
    {
        EXPECT_EQ(nodes[column].id, column + 1U);
    }
    // A strip: two rows of eleven nodes, so eleven distinct x positions.
    std::set<double> xPositions;
    for(const auto & node : nodes)
    {
        xPositions.insert(node.x);
    }
    EXPECT_EQ(xPositions.size(), 11U);
}

TEST(TransientResults, EntriesAndZipAgree)
{
    const auto entries{ThermZip::unzipFiles(fixturePath())};
    EXPECT_TRUE(hasTransientResults(entries));
    const auto fromEntries{read(entries)};
    const auto fromZip{readFromZipFile(fixturePath())};
    ASSERT_TRUE(fromEntries.has_value() && fromZip.has_value());
    EXPECT_EQ(fromEntries.value().frames, fromZip.value().frames);
    EXPECT_EQ(fromEntries.value().nodal.at(Quantity::Humidity).values, fromZip.value().nodal.at(Quantity::Humidity).values);
}

TEST(TransientResults, ASteadyArchiveHasNone)
{
    std::filesystem::path path{TEST_DATA_DIR};
    path /= "products";
    path /= "sample-sill.thmz";
    EXPECT_FALSE(hasTransientResults(ThermZip::unzipFiles(path.string())));
    const auto results{readFromZipFile(path.string())};
    EXPECT_FALSE(results.has_value());
}

TEST(TransientResults, OlderNumberingStartsAtOne)
{
    const auto results{read(olderEntries())};
    ASSERT_TRUE(results.has_value()) << results.error();
    EXPECT_FALSE(results.value().firstFrameIsInitial);
    EXPECT_EQ(results.value().frames, (std::vector<std::size_t>{1U, 2U}));
    const auto & temperature{results.value().nodal.at(Quantity::Temperature)};
    EXPECT_NEAR(temperature.values[1][3], 23.0, tolerance);
    EXPECT_NEAR(temperature.solveError[1], 2e-6, tolerance);
    EXPECT_FALSE(results.value().has(Quantity::Humidity));
    ASSERT_EQ(results.value().heatFlux.size(), 2U);
    EXPECT_EQ(results.value().heatFlux[1][3], (FluxVector{3.0, 3.0}));
    EXPECT_TRUE(results.value().waterFlux.empty());
}

TEST(TransientResults, EntryNamesFollowTheFormat)
{
    EXPECT_EQ(entryName(Quantity::Temperature), "transient results/Temperatures.csv");
    EXPECT_EQ(entryName(Quantity::Humidity), "transient results/Humidities.csv");
    EXPECT_EQ(entryName(Quantity::WaterContent), "transient results/WaterContent.csv");
    EXPECT_EQ(entryName(Quantity::IceContent), "transient results/IceContent.csv");
}

TEST(TransientResults, MissingGeometryIsRefused)
{
    auto entries{olderEntries()};
    entries.erase(geometryEntry());
    EXPECT_FALSE(hasTransientResults(entries));
    const auto results{read(entries)};
    ASSERT_FALSE(results.has_value());
    EXPECT_NE(results.error().find("Geometry"), std::string::npos);
}

TEST(TransientResults, NoNodalQuantityIsRefused)
{
    std::map<std::string, std::string> entries{{geometryEntry(), squareGeometry()}};
    EXPECT_FALSE(hasTransientResults(entries));
    const auto results{read(entries)};
    ASSERT_FALSE(results.has_value());
    EXPECT_NE(results.error().find("no nodal quantities"), std::string::npos);
}

TEST(TransientResults, ColumnCountMustMatchTheGeometry)
{
    auto entries{olderEntries()};
    entries[ThermZip::getTemperatureFileName()] = "timestep #,error tolerance,1,2,3\n1,0,10,11,12\n";
    const auto results{read(entries)};
    ASSERT_FALSE(results.has_value());
    EXPECT_NE(results.error().find("3 node columns"), std::string::npos);

    entries = olderEntries();
    entries[ThermZip::getHeatFluxFileName()] = "1-x,1-y\n0,0\n";
    EXPECT_FALSE(read(entries).has_value());
}

TEST(TransientResults, QuantitiesMustAgreeOnFrames)
{
    auto entries{olderEntries()};
    entries[ThermZip::getHumidityFileName()] = "timestep #,error tolerance,1,2,3,4\n0,0,1,1,1,1\n1,0,1,1,1,1\n";
    const auto results{read(entries)};
    ASSERT_FALSE(results.has_value());
    EXPECT_NE(results.error().find("frame numbering"), std::string::npos);
}

TEST(TransientResults, GeometryIdsMustBeContiguous)
{
    GeometryLibrary::InputGeometryDataRecord geometry;
    geometry.version = "1";
    geometry.add(1U, "Stucco", "0x808080");
    geometry.addNode(1U, 0.0, 0.0);
    geometry.addNode(2U, 1.0, 0.0);
    geometry.addNode(5U, 1.0, 1.0);   // a gap
    geometry.addElement(1U, 1U, 2U, 5U, 5U, 1U);
    std::map<std::string, std::string> entries{
      {geometryEntry(), geometry.saveToString()},
      {ThermZip::getTemperatureFileName(), "timestep #,error tolerance,1,2,3\n0,0,1,2,3\n"}};
    const auto results{read(entries)};
    ASSERT_FALSE(results.has_value());
    EXPECT_NE(results.error().find("without gaps"), std::string::npos);
}
