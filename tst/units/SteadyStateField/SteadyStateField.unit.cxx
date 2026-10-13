#include <cmath>
#include <filesystem>
#include <map>
#include <string>

#include <gtest/gtest.h>

#include "SteadyStateField/SteadyStateField.hxx"
#include "THMZ/Mesh/DB.hxx"
#include "THMZ/SteadyStateMeshResults/DB.hxx"

namespace
{
    using ThermFile::RunType;
    namespace Field = ThermFile::SteadyStateField;

    [[nodiscard]] std::string archive(const std::string & name)
    {
        std::filesystem::path path{TEST_DATA_DIR};
        path /= "products";
        path /= name;
        return path.string();
    }

    //! Three nodes labelled 1..3 at x = 0, 1, 2, and the results listed in the REVERSE order
    //! with temperatures 10 x label: a join by list position would put 30 at x = 0.
    [[nodiscard]] std::map<std::string, std::string> reversedEntries()
    {
        ThermFile::Mesh::Mesh mesh;
        mesh.cases.push_back({.modelType = RunType::UFactor,
                              .nodes = {{.index = 1U, .x = 0.0, .y = 0.0},
                                        {.index = 2U, .x = 1.0, .y = 0.0},
                                        {.index = 3U, .x = 2.0, .y = 0.0}},
                              .elements = {{.index = 1U, .node1 = 0U, .node2 = 1U, .node3 = 2U, .node4 = 2U}}});
        ThermFile::MeshResults results;
        results.cases.push_back({.resultsType = RunType::UFactor,
                                 .nodes = {{.index = 3U, .temperature = 30.0, .xFlux = 3.0, .yFlux = 4.0},
                                           {.index = 2U, .temperature = 20.0},
                                           {.index = 1U, .temperature = 10.0, .humidity = 0.5}}});
        return {{"Mesh.xml", ThermFile::Mesh::saveToString(mesh)},
                {"SteadyStateMeshResults.xml", ThermFile::saveToString(results)}};
    }
}   // namespace

TEST(SteadyStateField, JoinsTheResultsOnTheNodeLabelNotTheListPosition)
{
    const auto field{Field::read(reversedEntries(), RunType::UFactor)};
    ASSERT_TRUE(field.has_value()) << field.error();

    ASSERT_EQ(field->nodes.size(), 3U);
    EXPECT_DOUBLE_EQ(field->temperature[0], 10.0);
    EXPECT_DOUBLE_EQ(field->temperature[1], 20.0);
    EXPECT_DOUBLE_EQ(field->temperature[2], 30.0);
    EXPECT_DOUBLE_EQ(field->humidity[0], 0.5);
    EXPECT_TRUE(std::isnan(field->humidity[1]));
    EXPECT_DOUBLE_EQ(Field::fluxMagnitude(field.value())[2], 5.0);
    EXPECT_EQ(field->elements.size(), 1U);
}

TEST(SteadyStateField, AMissingRunNamesTheRunsThereAre)
{
    const auto field{Field::read(reversedEntries(), RunType::CondensationResistance)};
    ASSERT_FALSE(field.has_value());
    EXPECT_NE(field.error().find("Condensation Resistance"), std::string::npos) << field.error();
    EXPECT_NE(field.error().find("U-factor"), std::string::npos) << field.error();
}

TEST(SteadyStateField, ReadsBothRunsOfTheSill)
{
    for(const auto runType : {RunType::UFactor, RunType::CondensationResistance})
    {
        const auto field{Field::readFromZipFile(archive("sample-sill.thmz"), runType)};
        ASSERT_TRUE(field.has_value()) << field.error();
        ASSERT_FALSE(field->nodes.empty());
        EXPECT_EQ(field->temperature.size(), field->nodes.size());
        EXPECT_EQ(field->runType, runType);
        for(const auto & element : field->elements)
        {
            EXPECT_LT(element.node1, field->nodes.size());
            EXPECT_LT(element.node4, field->nodes.size());
        }
        for(const auto temperature : field->temperature)
        {
            EXPECT_FALSE(std::isnan(temperature));
        }
    }
}

TEST(SteadyStateField, TheSillsUFactorRunHasItsStoredMesh)
{
    const auto field{Field::readFromZipFile(archive("sample-sill.thmz"), RunType::UFactor)};
    ASSERT_TRUE(field.has_value()) << field.error();
    EXPECT_EQ(field->nodes.size(), 425U);
    EXPECT_EQ(field->elements.size(), 417U);
}

TEST(SteadyStateField, AnArchiveWithoutResultsSaysSo)
{
    const auto field{Field::readFromZipFile(archive("sample-sill-no-results.thmz"), RunType::UFactor)};
    ASSERT_FALSE(field.has_value());
    EXPECT_NE(field.error().find("nodal results"), std::string::npos) << field.error();
}

TEST(SteadyStateField, AMissingArchiveSaysSo)
{
    const auto field{Field::readFromZipFile(archive("no-such-archive.thmz"), RunType::UFactor)};
    ASSERT_FALSE(field.has_value());
    EXPECT_NE(field.error().find("No such archive"), std::string::npos) << field.error();
}
