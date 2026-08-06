#include <filesystem>
#include <fstream>
#include <map>

#include <gtest/gtest.h>

#include <fileParse/FileFormat.hxx>

#include "BoundaryConditions/DB.hxx"
#include "EnvironmentData/DB.hxx"
#include "Materials/DB.hxx"
#include "THMZ/Model/DB.hxx"
#include "THMZ/Model/Serializers.hxx"
#include "THMZ/ZipModule/ZipModule.hxx"

#include "THMXHelper.hxx"
#include "TestExamples.hxx"

using EnvironmentDataLibrary::Channel;
using EnvironmentDataLibrary::ChannelRole;
using EnvironmentDataLibrary::EnvironmentData;

namespace
{
    constexpr auto tolerance{1e-6};

    std::filesystem::path scratchPath(const std::string & name)
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

TEST(TestJSONFormat, EntryNameForFormat)
{
    EXPECT_EQ(ThermZip::entryNameForFormat("Model.xml", FileParse::FileFormat::XML), "Model.xml");
    EXPECT_EQ(ThermZip::entryNameForFormat("Model.xml", FileParse::FileFormat::JSON), "Model.json");
    EXPECT_EQ(ThermZip::entryNameForFormat("transient results/Geometry.xml", FileParse::FileFormat::JSON),
              "transient results/Geometry.json");

    const auto candidates{ThermZip::entryNameCandidates("Model.xml")};
    ASSERT_EQ(candidates.size(), 2U);
    EXPECT_EQ(candidates[0], "Model.json");
    EXPECT_EQ(candidates[1], "Model.xml");
}

TEST(TestJSONFormat, ModelJSONStringRoundTrip)
{
    const ThermFile::ThermModel model{Helper::THMXFile1::testObject()};

    const auto jsonContent{ThermFile::saveToString(model, FileParse::FileFormat::JSON)};
    ASSERT_FALSE(jsonContent.empty());
    EXPECT_EQ(jsonContent.front(), '{');

    const auto loaded{ThermFile::loadThermModelFromString(jsonContent, FileParse::FileFormat::JSON)};
    ASSERT_TRUE(loaded.has_value());

    Helper::expect_near(model, loaded.value(), tolerance);
}

TEST(TestJSONFormat, ModelJSONFileRoundTrip)
{
    const ThermFile::ThermModel model{Helper::THMXFile1::testObject()};

    const auto fileName{scratchPath("json_format_model.json")};
    std::filesystem::remove(fileName);
    ThermFile::saveToFile(model, fileName.string(), FileParse::FileFormat::JSON);

    const auto loaded{ThermFile::loadThermModelFromFile(fileName.string())};
    ASSERT_TRUE(loaded.has_value());

    Helper::expect_near(model, loaded.value(), tolerance);

    std::filesystem::remove(fileName);
}

TEST(TestJSONFormat, ModelJSONZipRoundTripAndCounterpartRemoval)
{
    const ThermFile::ThermModel model{Helper::THMXFile1::testObject()};

    const auto zipPath{scratchPath("json_format_model.thmz")};
    createArchive(zipPath);

    EXPECT_EQ(ThermFile::saveToZipFile(model, zipPath.string(), FileParse::FileFormat::JSON), 1);

    const auto entriesAfterJson{ThermZip::unzipFiles(zipPath.string())};
    EXPECT_TRUE(entriesAfterJson.contains("Model.json"));
    EXPECT_FALSE(entriesAfterJson.contains("Model.xml"));

    const auto loaded{ThermFile::loadThermModelFromZipFile(zipPath.string())};
    ASSERT_TRUE(loaded.has_value());
    Helper::expect_near(model, loaded.value(), tolerance);

    // Switching back to XML must drop the JSON entry again.
    EXPECT_EQ(ThermFile::saveToZipFile(model, zipPath.string(), FileParse::FileFormat::XML), 1);
    const auto entriesAfterXml{ThermZip::unzipFiles(zipPath.string())};
    EXPECT_TRUE(entriesAfterXml.contains("Model.xml"));
    EXPECT_FALSE(entriesAfterXml.contains("Model.json"));

    const auto reloaded{ThermFile::loadThermModelFromZipFile(zipPath.string())};
    ASSERT_TRUE(reloaded.has_value());
    Helper::expect_near(model, reloaded.value(), tolerance);

    std::filesystem::remove(zipPath);
}

TEST(TestJSONFormat, BoundaryConditionsJSONZipRoundTrip)
{
    const auto zipPath{scratchPath("json_format_bc.thmz")};
    createArchive(zipPath);

    BCLibrary::DB source;
    BCLibrary::BoundaryCondition record;
    record.UUID = "11111111-2222-3333-4444-555555555555";
    record.Name = "Exterior";
    source.add(record);

    EXPECT_EQ(source.saveToZipFile(zipPath.string(), FileParse::FileFormat::JSON), 1);

    const auto entries{ThermZip::unzipFiles(zipPath.string())};
    EXPECT_TRUE(entries.contains("BoundaryConditions.json"));
    EXPECT_FALSE(entries.contains("BoundaryConditions.xml"));

    BCLibrary::DB loaded;
    loaded.loadFromZipFile(zipPath.string());

    const auto found{loaded.getByUUID("11111111-2222-3333-4444-555555555555")};
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->Name, "Exterior");

    std::filesystem::remove(zipPath);
}

TEST(TestJSONFormat, EnvironmentDatasetsJSONZipRoundTrip)
{
    const auto zipPath{scratchPath("json_format_env.thmz")};
    createArchive(zipPath);

    const std::vector<EnvironmentData> datasets{
      makeDataset("aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee", "Golden CO")};

    EXPECT_EQ(EnvironmentDataLibrary::saveDatasetsToZipFile(datasets, zipPath.string(), FileParse::FileFormat::JSON),
              1);

    const auto entries{ThermZip::unzipFiles(zipPath.string())};
    EXPECT_TRUE(entries.contains("environment data/aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee.json"));
    EXPECT_FALSE(entries.contains("environment data/aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee.xml"));

    const auto loaded{EnvironmentDataLibrary::loadDatasetsFromZipFile(zipPath.string())};
    ASSERT_EQ(loaded.size(), 1U);
    EXPECT_EQ(loaded[0].Name, "Golden CO");
    ASSERT_EQ(loaded[0].channels.size(), 1U);
    EXPECT_NEAR(loaded[0].channels[0].values[2], 3.0, 1e-9);

    std::filesystem::remove(zipPath);
}

TEST(TestJSONFormat, SeededJSONLibraryFileLoads)
{
    const auto fileName{scratchPath("json_format_materials.json")};
    std::filesystem::remove(fileName);

    // Constructor seeds a missing .json library file with JSON content and loads it back empty.
    MaterialsLibrary::DB database{fileName.string()};
    EXPECT_TRUE(database.getMaterials().empty());

    {
        std::ifstream seeded(fileName);
        std::string firstChar;
        seeded >> firstChar;
        EXPECT_EQ(firstChar.front(), '{');
    }

    std::filesystem::remove(fileName);
}
