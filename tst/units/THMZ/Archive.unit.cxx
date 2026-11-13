#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "THMZ/Archive/Archive.hxx"
#include "THMZ/ZipModule/ZipModule.hxx"

// A THERM archive as one value: read() reads the list of entries, each part is read from
// the file when asked for. The failure rule the tests hold: a file that cannot be read
// fails at read(); a part the archive does not hold is empty; nothing else is a failure.
namespace
{
    std::string product(const std::string & name)
    {
        std::filesystem::path path{TEST_DATA_DIR};
        path /= "products";
        path /= name;
        return path.string();
    }
}   // namespace

TEST(Archive, AMissingFileFailsAtRead)
{
    const auto archive{ThermArchive::read(product("no-such-archive.thmz"))};

    ASSERT_FALSE(archive.has_value());
    EXPECT_NE(archive.error().find("No such archive"), std::string::npos) << archive.error();
}

TEST(Archive, AFileThatIsNotAnArchiveFailsAtRead)
{
    const auto path{std::filesystem::temp_directory_path() / "not-an-archive.thmz"};
    {
        std::ofstream file{path};
        file << "this is text, not a zip";
    }

    const auto archive{ThermArchive::read(path.string())};
    std::filesystem::remove(path);

    ASSERT_FALSE(archive.has_value());
    EXPECT_NE(archive.error().find("Not a readable archive"), std::string::npos) << archive.error();
}

TEST(Archive, ReadListsTheEntriesWithoutTheirContent)
{
    const auto archive{ThermArchive::read(product("sample-sill.thmz"))};

    ASSERT_TRUE(archive.has_value()) << archive.error();
    EXPECT_EQ(archive->fileName, product("sample-sill.thmz"));
    EXPECT_FALSE(archive->entryNames.empty());
    EXPECT_TRUE(ThermArchive::hasEntry(archive.value(), ThermZip::ModelFileName));
    EXPECT_TRUE(ThermArchive::hasEntry(archive.value(), ThermZip::MeshName));
    EXPECT_FALSE(ThermArchive::hasEntry(archive.value(), "NoSuchEntry"));
}

TEST(Archive, ASteadyArchiveGivesItsPartsAndNoTransientResults)
{
    const auto archive{ThermArchive::read(product("sample-sill.thmz"))};
    ASSERT_TRUE(archive.has_value()) << archive.error();

    const auto model{ThermArchive::model(archive.value())};
    ASSERT_TRUE(model.has_value()) << model.error();
    ASSERT_TRUE(model->has_value());
    EXPECT_FALSE(model->value().polygons.empty());

    const auto mesh{ThermArchive::mesh(archive.value())};
    ASSERT_TRUE(mesh.has_value()) << mesh.error();
    EXPECT_TRUE(mesh->has_value());

    const auto materials{ThermArchive::materials(archive.value())};
    ASSERT_TRUE(materials.has_value()) << materials.error();
    ASSERT_TRUE(materials->has_value());
    EXPECT_FALSE(materials->value().getNames().empty());

    const auto results{ThermArchive::steadyStateResults(archive.value())};
    ASSERT_TRUE(results.has_value()) << results.error();
    EXPECT_TRUE(results->has_value());

    const auto meshResults{ThermArchive::steadyStateMeshResults(archive.value())};
    ASSERT_TRUE(meshResults.has_value()) << meshResults.error();
    EXPECT_TRUE(meshResults->has_value());

    const auto field{ThermArchive::steadyStateField(archive.value(), ThermFile::RunType::UFactor)};
    ASSERT_TRUE(field.has_value()) << field.error();
    ASSERT_TRUE(field->has_value());
    EXPECT_FALSE(field->value().temperature.empty());

    // Not in a steady archive: empty, and not a failure.
    const auto transient{ThermArchive::transientResults(archive.value())};
    ASSERT_TRUE(transient.has_value()) << transient.error();
    EXPECT_FALSE(transient->has_value());

    const auto datasets{ThermArchive::datasets(archive.value())};
    ASSERT_TRUE(datasets.has_value()) << datasets.error();
    EXPECT_TRUE(datasets->empty());
}

TEST(Archive, AnArchiveWithoutResultsHasEmptyResults)
{
    const auto archive{ThermArchive::read(product("sample-sill-no-results.thmz"))};
    ASSERT_TRUE(archive.has_value()) << archive.error();

    const auto model{ThermArchive::model(archive.value())};
    ASSERT_TRUE(model.has_value()) << model.error();
    EXPECT_TRUE(model->has_value());

    const auto results{ThermArchive::steadyStateResults(archive.value())};
    ASSERT_TRUE(results.has_value()) << results.error();
    EXPECT_FALSE(results->has_value());

    const auto field{ThermArchive::steadyStateField(archive.value(), ThermFile::RunType::UFactor)};
    ASSERT_TRUE(field.has_value()) << field.error();
    EXPECT_FALSE(field->has_value());
}

TEST(Archive, ATransientArchiveGivesItsTransientResults)
{
    const auto archive{ThermArchive::read(product("transient-isothermal-strip.thmz"))};
    ASSERT_TRUE(archive.has_value()) << archive.error();

    const auto transient{ThermArchive::transientResults(archive.value())};
    ASSERT_TRUE(transient.has_value()) << transient.error();
    ASSERT_TRUE(transient->has_value());
    EXPECT_GT(transient->value().nodeCount(), 0U);
}

TEST(Archive, AnEntryIsGivenAsTextOrNotAtAll)
{
    const auto archive{ThermArchive::read(product("sample-sill.thmz"))};
    ASSERT_TRUE(archive.has_value()) << archive.error();

    const auto present{ThermArchive::entry(archive.value(), archive->entryNames.front())};
    ASSERT_TRUE(present.has_value()) << present.error();
    ASSERT_TRUE(present->has_value());
    EXPECT_FALSE(present->value().empty());

    const auto absent{ThermArchive::entry(archive.value(), "NoSuchEntry.xml")};
    ASSERT_TRUE(absent.has_value()) << absent.error();
    EXPECT_FALSE(absent->has_value());
}

TEST(Archive, AFileRemovedAfterReadFailsWhenAPartIsAskedFor)
{
    const auto path{std::filesystem::temp_directory_path() / "archive-that-goes-away.thmz"};
    std::filesystem::copy_file(product("sample-sill.thmz"), path, std::filesystem::copy_options::overwrite_existing);
    const auto archive{ThermArchive::read(path.string())};
    ASSERT_TRUE(archive.has_value()) << archive.error();
    std::filesystem::remove(path);

    // The archive value refers to a file. Its list said the model is there, the file is not.
    const auto model{ThermArchive::model(archive.value())};
    EXPECT_FALSE(model.has_value());
}
