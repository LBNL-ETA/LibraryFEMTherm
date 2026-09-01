#include <gtest/gtest.h>

#include "Gases/DB.hxx"

namespace
{
    GasesLibrary::PureGas makeArgon()
    {
        GasesLibrary::PureGas pure;
        pure.UUID = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee";
        pure.Name = "Argon";
        return pure;
    }

    GasesLibrary::Gas makeMixture()
    {
        GasesLibrary::Gas gas;
        gas.UUID = "ffffffff-0000-1111-2222-333333333333";
        gas.Name = "Argon 90%";
        return gas;
    }
}   // namespace

TEST(TestGasesDBDirty, FreshDBIsCleanAndAddsMarkDirty)
{
    GasesLibrary::DB database;
    EXPECT_FALSE(database.isDirty());

    database.addPureGas(makeArgon());
    EXPECT_TRUE(database.isDirty());

    GasesLibrary::DB second;
    second.addGas(makeMixture());
    EXPECT_TRUE(second.isDirty());
}

TEST(TestGasesDBDirty, NoOpMutationsStayClean)
{
    GasesLibrary::DB source;
    source.addPureGas(makeArgon());
    source.addGas(makeMixture());

    GasesLibrary::DB database;
    database.loadFromString(source.saveToString());
    EXPECT_FALSE(database.isDirty());

    database.deleteWithUUID("no-such-uuid");
    database.deleteWithProjectName("no such project");
    database.deleteTemporaryRecords();
    database.updatePureGas(database.getPureGasByName("Argon").value());
    EXPECT_FALSE(database.isDirty());

    database.deleteWithUUID(makeMixture().UUID);
    EXPECT_TRUE(database.isDirty());
}

TEST(TestGasesDBDirty, RenameProjectRecords)
{
    // Project-marked gases never round-trip through saveToString (temporary records
    // are session-only for gases), so the marked record enters through addGas.
    GasesLibrary::DB database;
    auto marked{makeMixture()};
    marked.ProjectName = "Old project";
    database.addGas(marked);

    database.renameRecordsWithProjectName("Old project", "New project");

    const auto record{database.getGasByUUID(marked.UUID)};
    ASSERT_TRUE(record.has_value());
    EXPECT_EQ(record->ProjectName, "New project");
}

TEST(TestGasesDBDirty, RenameWithNoMatchStaysClean)
{
    GasesLibrary::DB source;
    source.addGas(makeMixture());

    GasesLibrary::DB database;
    database.loadFromString(source.saveToString());
    EXPECT_FALSE(database.isDirty());

    database.renameRecordsWithProjectName("No such project", "New project");
    EXPECT_FALSE(database.isDirty());
}
