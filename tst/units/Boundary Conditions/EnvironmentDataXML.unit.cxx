#include <gtest/gtest.h>

#include "EnvironmentData/ContentHash.hxx"
#include "EnvironmentData/DB.hxx"
#include "EnvironmentData/Serializers.hxx"
#include "EnvironmentData/Tags.hxx"

using EnvironmentDataLibrary::Channel;
using EnvironmentDataLibrary::ChannelRole;
using EnvironmentDataLibrary::EnvironmentData;

namespace
{
    EnvironmentData makeSample()
    {
        EnvironmentData data;
        data.UUID = "11111111-2222-3333-4444-555555555555";
        data.Name = "Sample exterior";
        data.Protected = true;
        data.channels = {Channel{ChannelRole::AirTemperature, {21.3, 21.1, 20.8}},
                         Channel{ChannelRole::RelativeHumidity, {0.52, 0.55, 0.57}},
                         Channel{ChannelRole::WindSpeed, {1.4, 1.7, 2.1}}};
        return data;
    }
}   // namespace

TEST(TestEnvironmentData, StepsAndRoleQueries)
{
    const auto data{makeSample()};

    EXPECT_EQ(EnvironmentDataLibrary::steps(data), 3U);
    EXPECT_TRUE(EnvironmentDataLibrary::hasRole(data, ChannelRole::AirTemperature));
    EXPECT_FALSE(EnvironmentDataLibrary::hasRole(data, ChannelRole::SolarIrradiance));

    const auto values{EnvironmentDataLibrary::valuesForRole(data, ChannelRole::WindSpeed)};
    ASSERT_TRUE(values.has_value());
    EXPECT_NEAR(values->at(1), 1.7, 1e-9);

    const auto roles{EnvironmentDataLibrary::providedRoles(data)};
    EXPECT_EQ(roles.size(), 3U);
}

TEST(TestEnvironmentData, ChannelRoleStringsRoundTrip)
{
    const auto names{EnvironmentDataLibrary::channelRoleStrings()};
    EXPECT_EQ(names.size(), 12U);

    for(const auto & name : names)
    {
        const auto role{EnvironmentDataLibrary::channelRoleFromString(name)};
        EXPECT_EQ(EnvironmentDataLibrary::channelRoleToString(role), name);
    }
}

TEST(TestEnvironmentData, DBSaveLoadRoundTrip)
{
    EnvironmentDataLibrary::DB source;
    source.add(makeSample());

    const auto content{source.saveToString()};

    EnvironmentDataLibrary::DB loaded;
    loaded.loadFromString(content);

    const auto record{loaded.getByUUID("11111111-2222-3333-4444-555555555555")};
    ASSERT_TRUE(record.has_value());
    EXPECT_EQ(record->Name, "Sample exterior");
    EXPECT_FALSE(record->ProjectName.has_value());
    EXPECT_TRUE(record->Protected);
    ASSERT_EQ(record->channels.size(), 3U);
    EXPECT_EQ(record->channels[0].role, ChannelRole::AirTemperature);
    ASSERT_EQ(record->channels[0].values.size(), 3U);
    EXPECT_NEAR(record->channels[0].values[2], 20.8, 1e-9);
}

TEST(TestEnvironmentData, ProjectNameRoundTrip)
{
    auto sample{makeSample()};
    sample.ProjectName = "Office model";

    EnvironmentDataLibrary::DB source;
    source.add(sample);

    EnvironmentDataLibrary::DB loaded;
    loaded.loadFromString(source.saveToString());

    const auto record{loaded.getByUUID(sample.UUID)};
    ASSERT_TRUE(record.has_value());
    ASSERT_TRUE(record->ProjectName.has_value());
    EXPECT_EQ(record->ProjectName.value(), "Office model");
}

TEST(TestEnvironmentData, DeleteTemporaryRecords)
{
    auto temporary{makeSample()};
    temporary.UUID = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee";
    temporary.ProjectName = "Office model";

    EnvironmentDataLibrary::DB database;
    database.add(makeSample());
    database.add(temporary);

    database.deleteTemporaryRecords();

    EXPECT_TRUE(database.getByUUID(makeSample().UUID).has_value());
    EXPECT_FALSE(database.getByUUID(temporary.UUID).has_value());
}

TEST(TestEnvironmentData, DeleteRecordsWithProjectName)
{
    auto firstProject{makeSample()};
    firstProject.UUID = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee";
    firstProject.ProjectName = "Office model";

    auto secondProject{makeSample()};
    secondProject.UUID = "ffffffff-0000-1111-2222-333333333333";
    secondProject.ProjectName = "Warehouse model";

    EnvironmentDataLibrary::DB database;
    database.add(makeSample());
    database.add(firstProject);
    database.add(secondProject);

    database.deleteRecordsWithProjectName("Office model");

    EXPECT_TRUE(database.getByUUID(makeSample().UUID).has_value());
    EXPECT_FALSE(database.getByUUID(firstProject.UUID).has_value());
    EXPECT_TRUE(database.getByUUID(secondProject.UUID).has_value());
}

TEST(TestEnvironmentData, ContentUuidIsDeterministic)
{
    const auto first{EnvironmentDataLibrary::contentUuid(makeSample())};
    const auto second{EnvironmentDataLibrary::contentUuid(makeSample())};
    EXPECT_EQ(first, second);
    EXPECT_EQ(first.size(), 36U);
}

TEST(TestEnvironmentData, ContentUuidIgnoresEnvelopeAndChannelOrder)
{
    auto data{makeSample()};
    const auto baseline{EnvironmentDataLibrary::contentUuid(data)};

    data.Name = "Renamed";
    data.ProjectName = "Office model";
    data.Protected = false;
    EXPECT_EQ(EnvironmentDataLibrary::contentUuid(data), baseline);

    std::swap(data.channels[0], data.channels[2]);
    EXPECT_EQ(EnvironmentDataLibrary::contentUuid(data), baseline);
}

TEST(TestEnvironmentData, ContentUuidChangesWithValues)
{
    auto data{makeSample()};
    const auto baseline{EnvironmentDataLibrary::contentUuid(data)};

    data.channels[0].values[0] += 0.1;
    EXPECT_NE(EnvironmentDataLibrary::contentUuid(data), baseline);
}
