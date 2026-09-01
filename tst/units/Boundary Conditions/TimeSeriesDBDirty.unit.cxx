#include <filesystem>

#include <gtest/gtest.h>

#include "TimeSeriesData/DB.hxx"

using TimeSeriesLibrary::Series;
using TimeSeriesLibrary::SeriesRole;
using TimeSeriesLibrary::TimeSeriesData;

namespace
{
    TimeSeriesData makeSample()
    {
        TimeSeriesData data;
        data.UUID = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee";
        data.Name = "Golden CO";
        data.series = {Series{SeriesRole::AirTemperature, {21.3, 21.1, 20.8}}};
        return data;
    }

    std::filesystem::path scratchLibrary(const std::string & name)
    {
        return std::filesystem::temp_directory_path() / name;
    }
}   // namespace

TEST(TestTimeSeriesDBDirty, FreshAndLoadedDBIsClean)
{
    TimeSeriesLibrary::DB database;
    EXPECT_FALSE(database.isDirty());

    TimeSeriesLibrary::DB source;
    source.add(makeSample());

    TimeSeriesLibrary::DB loaded;
    loaded.loadFromString(source.saveToString());
    EXPECT_FALSE(loaded.isDirty());
}

TEST(TestTimeSeriesDBDirty, SaveIfDirtyWritesOnceAndClears)
{
    const auto libraryPath{scratchLibrary("time_series_dirty.xml")};
    std::filesystem::remove(libraryPath);

    TimeSeriesLibrary::DB database(libraryPath.string());
    EXPECT_FALSE(database.isDirty());

    database.add(makeSample());
    EXPECT_TRUE(database.isDirty());

    EXPECT_EQ(database.saveIfDirty(), 0);
    EXPECT_FALSE(database.isDirty());

    const auto savedTime{std::filesystem::last_write_time(libraryPath)};
    EXPECT_EQ(database.saveIfDirty(), 0);
    EXPECT_FALSE(database.isDirty());
    EXPECT_EQ(std::filesystem::last_write_time(libraryPath), savedTime);

    std::filesystem::remove(libraryPath);
}

TEST(TestTimeSeriesDBDirty, UpdateWithIdenticalRecordStaysClean)
{
    TimeSeriesLibrary::DB source;
    source.add(makeSample());

    TimeSeriesLibrary::DB database;
    database.loadFromString(source.saveToString());

    database.update(makeSample());
    EXPECT_FALSE(database.isDirty());

    auto renamed{makeSample()};
    renamed.Name = "Renamed";
    database.update(renamed);
    EXPECT_TRUE(database.isDirty());
}

TEST(TestTimeSeriesDBDirty, DeleteMarksDirtyOnlyWhenRecordsShrink)
{
    TimeSeriesLibrary::DB source;
    source.add(makeSample());

    TimeSeriesLibrary::DB database;
    database.loadFromString(source.saveToString());

    database.deleteWithUUID("no-such-uuid");
    EXPECT_FALSE(database.isDirty());

    database.deleteRecordsWithProjectName("no such project");
    EXPECT_FALSE(database.isDirty());

    database.deleteTemporaryRecords();
    EXPECT_FALSE(database.isDirty());

    database.deleteWithUUID(makeSample().UUID);
    EXPECT_TRUE(database.isDirty());
}

TEST(TestTimeSeriesDBDirty, DeleteTemporaryRecordsMarksDirty)
{
    auto temporary{makeSample()};
    temporary.ProjectName = "Office model";

    TimeSeriesLibrary::DB source;
    source.add(temporary);

    TimeSeriesLibrary::DB database;
    database.loadFromString(source.saveToString());

    database.deleteTemporaryRecords();
    EXPECT_TRUE(database.isDirty());
}
