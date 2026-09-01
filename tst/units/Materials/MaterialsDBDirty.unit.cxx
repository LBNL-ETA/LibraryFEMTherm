#include <filesystem>

#include <gtest/gtest.h>

#include "Materials/DB.hxx"

namespace
{
    MaterialsLibrary::Material makeSample()
    {
        auto material{MaterialsLibrary::generate("aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee")};
        material.Name = "Wood pine";
        return material;
    }

    //! A DB seeded through a round-trip so it starts clean, the way a real library
    //! starts after loading its file.
    MaterialsLibrary::DB makeLoadedDB()
    {
        MaterialsLibrary::DB source;
        source.add(makeSample());

        MaterialsLibrary::DB loaded;
        loaded.loadFromString(source.saveToString());
        return loaded;
    }
}   // namespace

TEST(TestMaterialsDBDirty, FreshAndLoadedDBIsClean)
{
    const MaterialsLibrary::DB database;
    EXPECT_FALSE(database.isDirty());

    const auto loaded{makeLoadedDB()};
    EXPECT_FALSE(loaded.isDirty());
}

TEST(TestMaterialsDBDirty, EffectiveMutationsMarkDirty)
{
    auto database{makeLoadedDB()};

    database.update(makeSample());
    EXPECT_FALSE(database.isDirty());

    database.deleteWithUUID("no-such-uuid");
    EXPECT_FALSE(database.isDirty());

    auto renamed{makeSample()};
    renamed.Name = "Renamed";
    database.update(renamed);
    EXPECT_TRUE(database.isDirty());
}

TEST(TestMaterialsDBDirty, RenameProjectRecords)
{
    auto marked{makeSample()};
    marked.ProjectName = "Old project";

    MaterialsLibrary::DB source;
    source.add(marked);

    MaterialsLibrary::DB database;
    database.loadFromString(source.saveToString());

    database.renameRecordsWithProjectName("No such project", "New project");
    EXPECT_FALSE(database.isDirty());

    database.renameRecordsWithProjectName("Old project", "New project");
    EXPECT_TRUE(database.isDirty());

    const auto record{database.getByUUID(makeSample().UUID)};
    ASSERT_TRUE(record.has_value());
    ASSERT_TRUE(record->ProjectName.has_value());
    EXPECT_EQ(record->ProjectName.value(), "New project");
}

TEST(TestMaterialsDBDirty, SaveIfDirtyWritesOnceAndClears)
{
    const auto libraryPath{std::filesystem::temp_directory_path() / "materials_dirty.xml"};
    std::filesystem::remove(libraryPath);

    MaterialsLibrary::DB database(libraryPath.string());
    EXPECT_FALSE(database.isDirty());

    database.add(makeSample());
    EXPECT_TRUE(database.isDirty());

    EXPECT_EQ(database.saveIfDirty(), 0);
    EXPECT_FALSE(database.isDirty());

    const auto savedTime{std::filesystem::last_write_time(libraryPath)};
    EXPECT_EQ(database.saveIfDirty(), 0);
    EXPECT_EQ(std::filesystem::last_write_time(libraryPath), savedTime);

    std::filesystem::remove(libraryPath);
}
