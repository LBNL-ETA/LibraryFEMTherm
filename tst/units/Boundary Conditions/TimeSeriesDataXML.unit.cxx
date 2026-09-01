#include <cmath>

#include <gtest/gtest.h>

#include "TimeSeriesData/ContentHash.hxx"
#include "TimeSeriesData/DB.hxx"
#include "TimeSeriesData/Serializers.hxx"
#include "TimeSeriesData/Tags.hxx"
#include "TimeSeriesData/ValueText.hxx"

using TimeSeriesLibrary::Series;
using TimeSeriesLibrary::SeriesRole;
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
        data.series = {Series{SeriesRole::AirTemperature, {21.3, 21.1, 20.8}},
                         Series{SeriesRole::RelativeHumidity, {0.52, 0.55, 0.57}},
                         Series{SeriesRole::WindSpeed, {1.4, 1.7, 2.1}}};
        return data;
    }
}   // namespace

TEST(TestTimeSeriesData, StepsAndRoleQueries)
{
    const auto data{makeSample()};

    EXPECT_EQ(TimeSeriesLibrary::steps(data), 3U);
    EXPECT_TRUE(TimeSeriesLibrary::hasRole(data, SeriesRole::AirTemperature));
    EXPECT_FALSE(TimeSeriesLibrary::hasRole(data, SeriesRole::SolarIrradiance));

    const auto values{TimeSeriesLibrary::valuesForRole(data, SeriesRole::WindSpeed)};
    ASSERT_TRUE(values.has_value());
    EXPECT_NEAR(values->at(1), 1.7, 1e-9);

    const auto roles{TimeSeriesLibrary::providedRoles(data)};
    EXPECT_EQ(roles.size(), 3U);
}

TEST(TestTimeSeriesData, SeriesRoleStringsRoundTrip)
{
    const auto names{TimeSeriesLibrary::seriesRoleStrings()};
    EXPECT_EQ(names.size(), 12U);

    for(const auto & name : names)
    {
        const auto role{TimeSeriesLibrary::seriesRoleFromString(name)};
        EXPECT_EQ(TimeSeriesLibrary::seriesRoleToString(role), name);
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
    ASSERT_EQ(record->series.size(), 3U);
    EXPECT_EQ(record->series[0].role, SeriesRole::AirTemperature);
    ASSERT_EQ(record->series[0].values.size(), 3U);
    EXPECT_NEAR(record->series[0].values[2], 20.8, 1e-9);
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

TEST(TestTimeSeriesData, ContentUuidIgnoresEnvelopeAndSeriesOrder)
{
    auto data{makeSample()};
    const auto baseline{TimeSeriesLibrary::contentUuid(data)};

    data.Name = "Renamed";
    data.ProjectName = "Office model";
    data.Protected = false;
    EXPECT_EQ(TimeSeriesLibrary::contentUuid(data), baseline);

    std::swap(data.series[0], data.series[2]);
    EXPECT_EQ(TimeSeriesLibrary::contentUuid(data), baseline);
}

TEST(TestTimeSeriesData, ContentUuidChangesWithValues)
{
    auto data{makeSample()};
    const auto baseline{TimeSeriesLibrary::contentUuid(data)};

    data.series[0].values[0] += 0.1;
    EXPECT_NE(TimeSeriesLibrary::contentUuid(data), baseline);
}

TEST(TestTimeSeriesData, SeriesValueTextRoundTrip)
{
    const std::vector<double> values{21.3, -0.00042, 0.0, 98765.4321, 1e-9};

    const auto text{TimeSeriesLibrary::formatSeriesValues(values)};
    const auto parsed{TimeSeriesLibrary::parseSeriesValues(text)};

    ASSERT_EQ(parsed.size(), values.size());
    for(size_t index = 0U; index < values.size(); ++index)
    {
        EXPECT_NEAR(parsed[index], values[index], std::abs(values[index]) * 1e-6 + 1e-12);
    }

    EXPECT_TRUE(TimeSeriesLibrary::parseSeriesValues("").empty());
    EXPECT_TRUE(TimeSeriesLibrary::formatSeriesValues({}).empty());
}

TEST(TestTimeSeriesData, XMLCarriesCommaSeparatedValues)
{
    TimeSeriesLibrary::DB source;
    source.add(makeSample());

    const auto content{source.saveToString()};

    EXPECT_NE(content.find("<Values>21.3,21.1,20.8</Values>"), std::string::npos);
    EXPECT_EQ(content.find("<Value>"), std::string::npos);
}

TEST(TestTimeSeriesData, JSONRoundTripWithCommaSeparatedValues)
{
    TimeSeriesLibrary::DB source;
    source.add(makeSample());

    const auto content{source.saveToString(FileParse::FileFormat::JSON)};
    EXPECT_NE(content.find("21.3,21.1,20.8"), std::string::npos);

    TimeSeriesLibrary::DB loaded;
    loaded.loadFromString(content);

    const auto record{loaded.getByUUID("11111111-2222-3333-4444-555555555555")};
    ASSERT_TRUE(record.has_value());
    ASSERT_EQ(record->series.size(), 3U);
    ASSERT_EQ(record->series[0].values.size(), 3U);
    EXPECT_NEAR(record->series[0].values[2], 20.8, 1e-9);
}
