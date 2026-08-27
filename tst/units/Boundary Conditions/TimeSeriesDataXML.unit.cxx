#include <gtest/gtest.h>

#include "TimeSeriesData/ContentHash.hxx"
#include "TimeSeriesData/DB.hxx"
#include "TimeSeriesData/Serializers.hxx"
#include "TimeSeriesData/Tags.hxx"

using TimeSeriesLibrary::Channel;
using TimeSeriesLibrary::ChannelRole;
using TimeSeriesLibrary::TimeSeriesData;

namespace
{
    TimeSeriesData makeSample()
    {
        TimeSeriesData data;
        data.UUID = "11111111-2222-3333-4444-555555555555";
        data.Name = "Sample exterior";
        data.Protected = true;
        data.Color = "0x2A6ACC";
        data.Source = "Imported";
        data.channels = {Channel{ChannelRole::AirTemperature, {21.3, 21.1, 20.8}},
                         Channel{ChannelRole::RelativeHumidity, {0.52, 0.55, 0.57}},
                         Channel{ChannelRole::WindSpeed, {1.4, 1.7, 2.1}}};
        return data;
    }
}   // namespace

TEST(TestTimeSeriesData, StepsAndRoleQueries)
{
    const auto data{makeSample()};

    EXPECT_EQ(TimeSeriesLibrary::steps(data), 3U);
    EXPECT_TRUE(TimeSeriesLibrary::hasRole(data, ChannelRole::AirTemperature));
    EXPECT_FALSE(TimeSeriesLibrary::hasRole(data, ChannelRole::SolarIrradiance));

    const auto values{TimeSeriesLibrary::valuesForRole(data, ChannelRole::WindSpeed)};
    ASSERT_TRUE(values.has_value());
    EXPECT_NEAR(values->at(1), 1.7, 1e-9);

    const auto roles{TimeSeriesLibrary::providedRoles(data)};
    EXPECT_EQ(roles.size(), 3U);
}

TEST(TestTimeSeriesData, ChannelRoleStringsRoundTrip)
{
    const auto names{TimeSeriesLibrary::channelRoleStrings()};
    EXPECT_EQ(names.size(), 12U);

    for(const auto & name : names)
    {
        const auto role{TimeSeriesLibrary::channelRoleFromString(name)};
        EXPECT_EQ(TimeSeriesLibrary::channelRoleToString(role), name);
    }
}

TEST(TestTimeSeriesData, DBSaveLoadRoundTrip)
{
    TimeSeriesLibrary::DB source;
    source.add(makeSample());

    const auto content{source.saveToString()};

    TimeSeriesLibrary::DB loaded;
    loaded.loadFromString(content);

    const auto record{loaded.getByUUID("11111111-2222-3333-4444-555555555555")};
    ASSERT_TRUE(record.has_value());
    EXPECT_EQ(record->Name, "Sample exterior");
    EXPECT_FALSE(record->ProjectName.has_value());
    EXPECT_TRUE(record->Protected);
    ASSERT_TRUE(record->Source.has_value());
    EXPECT_EQ(record->Source.value(), "Imported");
    EXPECT_EQ(record->Color, "0x2A6ACC");
    ASSERT_EQ(record->channels.size(), 3U);
    EXPECT_EQ(record->channels[0].role, ChannelRole::AirTemperature);
    ASSERT_EQ(record->channels[0].values.size(), 3U);
    EXPECT_NEAR(record->channels[0].values[2], 20.8, 1e-9);
}

TEST(TestTimeSeriesData, ProjectNameRoundTrip)
{
    auto sample{makeSample()};
    sample.ProjectName = "Office model";

    TimeSeriesLibrary::DB source;
    source.add(sample);

    TimeSeriesLibrary::DB loaded;
    loaded.loadFromString(source.saveToString());

    const auto record{loaded.getByUUID(sample.UUID)};
    ASSERT_TRUE(record.has_value());
    ASSERT_TRUE(record->ProjectName.has_value());
    EXPECT_EQ(record->ProjectName.value(), "Office model");
}

TEST(TestTimeSeriesData, DeleteTemporaryRecords)
{
    auto temporary{makeSample()};
    temporary.UUID = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee";
    temporary.ProjectName = "Office model";

    TimeSeriesLibrary::DB database;
    database.add(makeSample());
    database.add(temporary);

    database.deleteTemporaryRecords();

    EXPECT_TRUE(database.getByUUID(makeSample().UUID).has_value());
    EXPECT_FALSE(database.getByUUID(temporary.UUID).has_value());
}

TEST(TestTimeSeriesData, DeleteRecordsWithProjectName)
{
    auto firstProject{makeSample()};
    firstProject.UUID = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee";
    firstProject.ProjectName = "Office model";

    auto secondProject{makeSample()};
    secondProject.UUID = "ffffffff-0000-1111-2222-333333333333";
    secondProject.ProjectName = "Warehouse model";

    TimeSeriesLibrary::DB database;
    database.add(makeSample());
    database.add(firstProject);
    database.add(secondProject);

    database.deleteRecordsWithProjectName("Office model");

    EXPECT_TRUE(database.getByUUID(makeSample().UUID).has_value());
    EXPECT_FALSE(database.getByUUID(firstProject.UUID).has_value());
    EXPECT_TRUE(database.getByUUID(secondProject.UUID).has_value());
}

TEST(TestTimeSeriesData, ContentUuidIsDeterministic)
{
    const auto first{TimeSeriesLibrary::contentUuid(makeSample())};
    const auto second{TimeSeriesLibrary::contentUuid(makeSample())};
    EXPECT_EQ(first, second);
    EXPECT_EQ(first.size(), 36U);
}

TEST(TestTimeSeriesData, ContentUuidIgnoresEnvelopeAndChannelOrder)
{
    auto data{makeSample()};
    const auto baseline{TimeSeriesLibrary::contentUuid(data)};

    data.Name = "Renamed";
    data.ProjectName = "Office model";
    data.Protected = false;
    EXPECT_EQ(TimeSeriesLibrary::contentUuid(data), baseline);

    std::swap(data.channels[0], data.channels[2]);
    EXPECT_EQ(TimeSeriesLibrary::contentUuid(data), baseline);
}

TEST(TestTimeSeriesData, ContentUuidChangesWithValues)
{
    auto data{makeSample()};
    const auto baseline{TimeSeriesLibrary::contentUuid(data)};

    data.channels[0].values[0] += 0.1;
    EXPECT_NE(TimeSeriesLibrary::contentUuid(data), baseline);
}
