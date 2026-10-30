#include <filesystem>
#include <fstream>
#include <map>

#include <gtest/gtest.h>

#include <fileParse/FileFormat.hxx>

#include "BoundaryConditions/DB.hxx"
#include "TimeSeriesData/DB.hxx"
#include "Materials/DB.hxx"
#include "THMZ/Model/DB.hxx"
#include "THMZ/Model/Serializers.hxx"
#include "THMZ/ZipModule/ZipModule.hxx"

#include "THMXHelper.hxx"
#include "TestExamples.hxx"

using TimeSeriesLibrary::Series;
using TimeSeriesLibrary::SeriesRole;
using TimeSeriesLibrary::TimeSeriesData;

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

    TimeSeriesData makeDataset(const std::string & uuid, const std::string & name)
    {
        TimeSeriesData data;
        data.UUID = uuid;
        data.Name = name;
        data.series = {Series{SeriesRole::AirTemperature, {1.0, 2.0, 3.0}}};
        return data;
    }
}   // namespace

TEST(TestJSONFormat, EntryNameForFormat)
{
    EXPECT_EQ(ThermZip::entryNameForFormat("Model", FileParse::FileFormat::XML), "Model.xml");
    EXPECT_EQ(ThermZip::entryNameForFormat("Model", FileParse::FileFormat::JSON), "Model.json");
    EXPECT_EQ(ThermZip::entryNameForFormat("transient results/Geometry", FileParse::FileFormat::JSON),
              "transient results/Geometry.json");

    const auto candidates{ThermZip::entryNameCandidates("Model")};
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

namespace
{
    //! What the zip loader hands back for the pre-consolidation example: a face naming a
    //! transient type record is bound to it on load (Migration::BCConsolidation), the rest
    //! is as written.
    ThermFile::ThermModel withTransientBindings(ThermFile::ThermModel model)
    {
        for(auto & face : model.boundaryConditions)
        {
            if(!face.bcUUID.has_value() && face.transientRecordData.has_value())
            {
                face.bcUUID = face.transientRecordData->typeRecordUUID;
            }
        }
        return model;
    }
}   // namespace

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
    Helper::expect_near(withTransientBindings(model), loaded.value(), tolerance);

    // Switching back to XML must drop the JSON entry again.
    EXPECT_EQ(ThermFile::saveToZipFile(model, zipPath.string(), FileParse::FileFormat::XML), 1);
    const auto entriesAfterXml{ThermZip::unzipFiles(zipPath.string())};
    EXPECT_TRUE(entriesAfterXml.contains("Model.xml"));
    EXPECT_FALSE(entriesAfterXml.contains("Model.json"));

    const auto reloaded{ThermFile::loadThermModelFromZipFile(zipPath.string())};
    ASSERT_TRUE(reloaded.has_value());
    Helper::expect_near(withTransientBindings(model), reloaded.value(), tolerance);

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

TEST(TestJSONFormat, TimeSeriesDatasetsJSONZipRoundTrip)
{
    const auto zipPath{scratchPath("json_format_env.thmz")};
    createArchive(zipPath);

    const std::vector<TimeSeriesData> datasets{
      makeDataset("aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee", "Golden CO")};

    EXPECT_EQ(TimeSeriesLibrary::saveDatasetsToZipFile(datasets, zipPath.string(), FileParse::FileFormat::JSON),
              1);

    const auto entries{ThermZip::unzipFiles(zipPath.string())};
    EXPECT_TRUE(entries.contains("time series/aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee.json"));
    EXPECT_FALSE(entries.contains("time series/aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee.xml"));

    const auto loaded{TimeSeriesLibrary::loadDatasetsFromZipFile(zipPath.string())};
    ASSERT_EQ(loaded.size(), 1U);
    EXPECT_EQ(loaded[0].Name, "Golden CO");
    ASSERT_EQ(loaded[0].series.size(), 1U);
    EXPECT_NEAR(loaded[0].series[0].values[2], 3.0, 1e-9);

    std::filesystem::remove(zipPath);
}

TEST(TestJSONFormat, AMissingJSONLibraryIsEmptyUntilSaved)
{
    const auto fileName{scratchPath("json_format_materials.json")};
    std::filesystem::remove(fileName);

    // A missing library file opens as an empty library and writes nothing...
    MaterialsLibrary::DB database{fileName.string()};
    EXPECT_TRUE(database.getMaterials().empty());
    EXPECT_FALSE(std::filesystem::exists(fileName));

    // ...until it is saved, in the format asked for, and then it loads back empty.
    EXPECT_EQ(database.saveToFile(FileParse::FileFormat::JSON), 0);
    {
        std::ifstream saved(fileName);
        std::string firstChar;
        saved >> firstChar;
        EXPECT_EQ(firstChar.front(), '{');
    }
    EXPECT_TRUE(MaterialsLibrary::DB{fileName.string()}.getMaterials().empty());

    std::filesystem::remove(fileName);
}
