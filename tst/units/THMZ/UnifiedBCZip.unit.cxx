#include <filesystem>
#include <map>

#include <gtest/gtest.h>

#include "BoundaryConditions/DB.hxx"
#include "EnvironmentData/DB.hxx"
#include "THMZ/ZipModule/ZipModule.hxx"

using EnvironmentDataLibrary::Channel;
using EnvironmentDataLibrary::ChannelRole;
using EnvironmentDataLibrary::EnvironmentData;

namespace
{
    std::filesystem::path scratchZip(const std::string & name)
    {
        return std::filesystem::temp_directory_path() / name;
    }

    //! miniz needs to create the archive itself, so start each test from a fresh zip
    //! that already contains a model entry, as every real THMZ does.
    void createArchive(const std::filesystem::path & zipPath)
    {
        std::filesystem::remove(zipPath);
        const std::map<std::string, std::string> seed{{"Model.xml", "<ThermModel/>"}};
        ThermZip::zipFiles(seed, zipPath.string());
    }

    EnvironmentData makeDataset(const std::string & uuid, const std::string & name)
    {
        EnvironmentData data;
        data.UUID = uuid;
        data.Name = name;
        data.channels = {Channel{ChannelRole::AirTemperature, {1.0, 2.0, 3.0}}};
        return data;
    }
}   // namespace

TEST(TestUnifiedBCZip, BoundaryConditionsRoundTrip)
{
    const auto zipPath{scratchZip("unified_bc_roundtrip.thmz")};
    createArchive(zipPath);

    BCLibrary::DB source;
    BCLibrary::BoundaryCondition record;
    record.UUID = "11111111-2222-3333-4444-555555555555";
    record.Name = "Exterior";
    source.add(record);

    EXPECT_EQ(source.saveToZipFile(zipPath.string()), 1);

    BCLibrary::DB loaded;
    loaded.loadFromZipFile(zipPath.string());

    const auto found{loaded.getByUUID("11111111-2222-3333-4444-555555555555")};
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->Name, "Exterior");

    std::filesystem::remove(zipPath);
}

TEST(TestUnifiedBCZip, BoundaryConditionsMissingEntryLeavesDBUntouched)
{
    const auto zipPath{scratchZip("unified_bc_missing_entry.thmz")};
    createArchive(zipPath);

    BCLibrary::DB loaded;
    loaded.loadFromZipFile(zipPath.string());
    EXPECT_TRUE(loaded.getNames().empty());

    loaded.loadFromZipFile("nonexistent-archive.thmz");
    EXPECT_TRUE(loaded.getNames().empty());

    std::filesystem::remove(zipPath);
}

TEST(TestUnifiedBCZip, EnvironmentDatasetsRoundTrip)
{
    const auto zipPath{scratchZip("unified_env_roundtrip.thmz")};
    createArchive(zipPath);

    const std::vector<EnvironmentData> datasets{
      makeDataset("aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee", "Golden CO"),
      makeDataset("ffffffff-0000-1111-2222-333333333333", "Reference year")};

    EXPECT_EQ(EnvironmentDataLibrary::saveDatasetsToZipFile(datasets, zipPath.string()), 2);

    const auto entries{ThermZip::unzipFiles(zipPath.string())};
    EXPECT_TRUE(entries.contains("environment data/aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee.xml"));
    EXPECT_TRUE(entries.contains("environment data/ffffffff-0000-1111-2222-333333333333.xml"));

    const auto loaded{EnvironmentDataLibrary::loadDatasetsFromZipFile(zipPath.string())};
    ASSERT_EQ(loaded.size(), 2U);

    const auto golden{std::ranges::find_if(
      loaded, [](const EnvironmentData & data) { return data.Name == "Golden CO"; })};
    ASSERT_NE(golden, loaded.end());
    ASSERT_EQ(golden->channels.size(), 1U);
    EXPECT_EQ(golden->channels[0].role, ChannelRole::AirTemperature);
    EXPECT_NEAR(golden->channels[0].values[2], 3.0, 1e-9);

    std::filesystem::remove(zipPath);
}

TEST(TestUnifiedBCZip, EnvironmentDatasetsAbsentYieldEmpty)
{
    const auto zipPath{scratchZip("unified_env_absent.thmz")};
    createArchive(zipPath);

    EXPECT_TRUE(EnvironmentDataLibrary::loadDatasetsFromZipFile(zipPath.string()).empty());
    EXPECT_TRUE(EnvironmentDataLibrary::loadDatasetsFromZipFile("nonexistent-archive.thmz").empty());

    std::filesystem::remove(zipPath);
}
