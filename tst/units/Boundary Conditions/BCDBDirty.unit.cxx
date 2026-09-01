#include <gtest/gtest.h>

#include "BoundaryConditions/DB.hxx"
#include "Legacy/Step1/BCSteadyState/DB.hxx"
#include "Legacy/Step1/BCTransient/Types.hxx"

namespace
{
    const std::string sampleUuid{"aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee"};
}   // namespace

TEST(TestBCDBDirty, SteadyStateEffectiveMutations)
{
    BCSteadyStateLibrary::BoundaryCondition record;
    record.UUID = sampleUuid;
    record.Name = "Exterior";

    BCSteadyStateLibrary::DB source;
    source.add(record);
    EXPECT_TRUE(source.isDirty());

    BCSteadyStateLibrary::DB database;
    database.loadFromString(source.saveToString());
    EXPECT_FALSE(database.isDirty());

    database.update(database.getByUUID(sampleUuid).value());
    database.deleteWithUUID("no-such-uuid");
    database.deleteRecordsWithProjectName("no such project");
    EXPECT_FALSE(database.isDirty());

    auto renamed{database.getByUUID(sampleUuid).value()};
    renamed.Name = "Renamed";
    database.update(renamed);
    EXPECT_TRUE(database.isDirty());
}

TEST(TestBCDBDirty, SteadyStateRenameSkipsIGUSurfaces)
{
    BCSteadyStateLibrary::BoundaryCondition regular;
    regular.UUID = sampleUuid;
    regular.Name = "Exterior";
    regular.ProjectName = "Old project";

    BCSteadyStateLibrary::BoundaryCondition surface;
    surface.UUID = "ffffffff-0000-1111-2222-333333333333";
    surface.Name = "Glazing surface";
    surface.ProjectName = "Old project";
    surface.isIGUSurface = true;

    BCSteadyStateLibrary::DB source;
    source.add(regular);
    source.add(surface);

    BCSteadyStateLibrary::DB database;
    database.loadFromString(source.saveToString());

    database.renameRecordsWithProjectName("Old project", "New project");
    EXPECT_TRUE(database.isDirty());

    EXPECT_EQ(database.getByUUID(regular.UUID)->ProjectName, "New project");
    EXPECT_EQ(database.getByUUID(surface.UUID)->ProjectName, "Old project");
}

TEST(TestBCDBDirty, TransientTypeEffectiveMutations)
{
    BCTypesLibrary::TypeRecord record{sampleUuid};
    record.Name = "Transient exterior";

    BCTypesLibrary::DB source;
    source.add(record);
    EXPECT_TRUE(source.isDirty());

    BCTypesLibrary::DB database;
    database.loadFromString(source.saveToString());
    EXPECT_FALSE(database.isDirty());

    database.update(database.getByUUID(sampleUuid).value());
    database.deleteWithUUID("no-such-uuid");
    EXPECT_FALSE(database.isDirty());

    auto renamed{database.getByUUID(sampleUuid).value()};
    renamed.Name = "Renamed";
    database.update(renamed);
    EXPECT_TRUE(database.isDirty());
}

TEST(TestBCDBDirty, UnifiedEffectiveMutations)
{
    BCLibrary::BoundaryCondition record;
    record.UUID = sampleUuid;
    record.Name = "Exterior";

    BCLibrary::DB source;
    source.add(record);
    EXPECT_TRUE(source.isDirty());

    BCLibrary::DB database;
    database.loadFromString(source.saveToString());
    EXPECT_FALSE(database.isDirty());

    database.updateOrAdd(database.getByUUID(sampleUuid).value());
    database.deleteWithUUID("no-such-uuid");
    database.deleteTemporaryRecords();
    EXPECT_FALSE(database.isDirty());

    auto renamed{database.getByUUID(sampleUuid).value()};
    renamed.Name = "Renamed";
    database.updateOrAdd(renamed);
    EXPECT_TRUE(database.isDirty());
}
