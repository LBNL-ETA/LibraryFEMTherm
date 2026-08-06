// Real-archive regression guard: the bundled Mullion jamb archives carry the SAME model
// in XML and JSON form. Loading both through the full path (single extraction, model +
// migrations, libraries, mesh, results) must yield identical structure -- this pins the
// dual-format promise on real-world data. Step timings are printed as a side product
// for quick eyeballing; nothing asserts on time.
#include <chrono>
#include <filesystem>
#include <iostream>

#include <gtest/gtest.h>

#include "BoundaryConditions/DB.hxx"
#include "TimeSeriesData/DB.hxx"
#include "THMZ/Mesh/DB.hxx"
#include "THMZ/Model/DB.hxx"
#include "THMZ/SteadyStateMeshResults/DB.hxx"
#include "THMZ/SteadyStateResults/DB.hxx"
#include "THMZ/ZipModule/ZipModule.hxx"

namespace
{
    std::string productPath(const std::string & name)
    {
        std::filesystem::path path{TEST_DATA_DIR};
        path /= "products";
        path /= name;
        return path.string();
    }

    class Stopwatch
    {
    public:
        Stopwatch() : start_{std::chrono::steady_clock::now()}
        {}

        void lap(const std::string & label)
        {
            const auto now{std::chrono::steady_clock::now()};
            const auto elapsed{std::chrono::duration<double, std::milli>(now - start_).count()};
            std::cout << "  " << label << ": " << elapsed << " ms\n";
            start_ = now;
        }

    private:
        std::chrono::steady_clock::time_point start_;
    };

    struct LoadedArchive
    {
        ThermFile::ThermModel model;
        ThermFile::Mesh::Mesh mesh;
        ThermFile::MeshResults meshResults;
        ThermFile::SteadyStateResults results;
        std::size_t bcRecordCount{0U};
    };

    LoadedArchive loadWholeArchive(const std::string & archive)
    {
        Stopwatch watch;

        const auto entries{ThermZip::unzipFiles(archive)};
        watch.lap("unzip whole archive into memory   ");

        auto model{ThermFile::loadThermModelFromEntries(entries)};
        EXPECT_TRUE(model.has_value());
        watch.lap("model + migrations from entries   ");

        BCLibrary::DB unified;
        unified.loadFromEntries(entries);
        watch.lap("BC library from entries           ");

        auto mesh{ThermFile::Mesh::loadMeshFromString(ThermZip::findEntry(entries, ThermZip::MeshName))};
        EXPECT_TRUE(mesh.has_value());
        watch.lap("mesh parse from memory            ");

        auto meshResults{ThermFile::loadSteadyStateMeshResultsFromString(
          ThermZip::findEntry(entries, ThermZip::SteadyStateMeshResultsName))};
        EXPECT_TRUE(meshResults.has_value());
        watch.lap("mesh results parse from memory    ");

        auto results{ThermFile::loadSteadyStateResultsFromString(
          ThermZip::findEntry(entries, ThermZip::SteadyStateResultsName))};
        EXPECT_TRUE(results.has_value());
        watch.lap("steady state results from memory  ");

        return LoadedArchive{std::move(model).value_or(ThermFile::ThermModel{}),
                             std::move(mesh).value_or(ThermFile::Mesh::Mesh{}),
                             std::move(meshResults).value_or(ThermFile::MeshResults{}),
                             std::move(results).value_or(ThermFile::SteadyStateResults{}),
                             unified.getBoundaryConditions().size()};
    }
}   // namespace

TEST(TestRealArchiveFormatParity, MullionJambXMLvsJSON)
{
    const auto xmlArchive{productPath("mullion-jamb-xml.thmz")};
    const auto jsonArchive{productPath("mullion-jamb-json.thmz")};
    if(!std::filesystem::exists(xmlArchive) || !std::filesystem::exists(jsonArchive))
    {
        GTEST_SKIP() << "parity archives not present";
    }

    std::cout << "XML variant:\n";
    const auto xmlLoaded{loadWholeArchive(xmlArchive)};
    std::cout << "JSON variant:\n";
    const auto jsonLoaded{loadWholeArchive(jsonArchive)};

    // Model structure
    EXPECT_EQ(xmlLoaded.model.polygons.size(), jsonLoaded.model.polygons.size());
    EXPECT_EQ(xmlLoaded.model.boundaryConditions.size(), jsonLoaded.model.boundaryConditions.size());
    EXPECT_EQ(xmlLoaded.model.properties.calculationOptions.simulationEngine,
              jsonLoaded.model.properties.calculationOptions.simulationEngine);

    // Mesh structure
    ASSERT_EQ(xmlLoaded.mesh.cases.size(), jsonLoaded.mesh.cases.size());
    ASSERT_FALSE(xmlLoaded.mesh.cases.empty());
    EXPECT_EQ(xmlLoaded.mesh.cases.front().nodes.size(), jsonLoaded.mesh.cases.front().nodes.size());

    // Results
    EXPECT_EQ(xmlLoaded.meshResults.cases.size(), jsonLoaded.meshResults.cases.size());
    EXPECT_EQ(xmlLoaded.results.cases.size(), jsonLoaded.results.cases.size());

    // The embedded unified BC libraries are NOT compared: the XML archive predates the
    // BC consolidation entries (legacy-only generation), while the JSON one was written
    // by the current build and embeds them. The models still agree because the legacy
    // migration fills the segment bindings at load -- which the counts above verify.
    EXPECT_EQ(xmlLoaded.bcRecordCount, 0U);
    EXPECT_GT(jsonLoaded.bcRecordCount, 0U);

    // Sanity: this is a real model, not an accidentally empty pair.
    EXPECT_GT(xmlLoaded.model.polygons.size(), 0U);
    EXPECT_GT(xmlLoaded.mesh.cases.front().nodes.size(), 0U);
}
