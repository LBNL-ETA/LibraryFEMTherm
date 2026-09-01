#include <gtest/gtest.h>

#include "THMZ/ZipModule/ZipModule.hxx"
#include "TimeSeriesData/ContentHash.hxx"
#include "TimeSeriesData/EntryPlan.hxx"

using TimeSeriesLibrary::Series;
using TimeSeriesLibrary::SeriesRole;
using TimeSeriesLibrary::TimeSeriesData;

namespace
{
    const std::string sampleUuid{"aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee"};

    TimeSeriesData makeSample()
    {
        TimeSeriesData data;
        data.UUID = sampleUuid;
        data.Name = "Golden CO";
        data.Color = "0x1F6FB4";
        data.Source = "Imported";
        data.series = {Series{SeriesRole::AirTemperature, {21.3, 21.1, 20.8}},
                         Series{SeriesRole::RelativeHumidity, {0.52, 0.55, 0.57}}};
        return data;
    }

    std::map<std::string, std::uint64_t> fingerprintsFor(const TimeSeriesData & data)
    {
        return {{data.UUID, TimeSeriesLibrary::datasetFingerprint(data)}};
    }
}   // namespace

TEST(TestTimeSeriesFingerprint, StableForIdenticalData)
{
    EXPECT_EQ(TimeSeriesLibrary::datasetFingerprint(makeSample()),
              TimeSeriesLibrary::datasetFingerprint(makeSample()));
}

TEST(TestTimeSeriesFingerprint, ChangesOnAnyMetadataField)
{
    const auto baseline{TimeSeriesLibrary::datasetFingerprint(makeSample())};

    auto renamed{makeSample()};
    renamed.Name = "Renamed";
    EXPECT_NE(TimeSeriesLibrary::datasetFingerprint(renamed), baseline);

    auto recolored{makeSample()};
    recolored.Color = "0xFF0000";
    EXPECT_NE(TimeSeriesLibrary::datasetFingerprint(recolored), baseline);

    auto reassigned{makeSample()};
    reassigned.ProjectName = "Office model";
    EXPECT_NE(TimeSeriesLibrary::datasetFingerprint(reassigned), baseline);

    auto described{makeSample()};
    described.Description = "";
    EXPECT_NE(TimeSeriesLibrary::datasetFingerprint(described), baseline);

    auto guarded{makeSample()};
    guarded.Protected = true;
    EXPECT_NE(TimeSeriesLibrary::datasetFingerprint(guarded), baseline);
}

TEST(TestTimeSeriesFingerprint, ChangesOnSeriesValueOrRole)
{
    const auto baseline{TimeSeriesLibrary::datasetFingerprint(makeSample())};

    auto edited{makeSample()};
    edited.series[0].values[1] += 0.1;
    EXPECT_NE(TimeSeriesLibrary::datasetFingerprint(edited), baseline);

    auto rerolled{makeSample()};
    rerolled.series[1].role = SeriesRole::WindSpeed;
    EXPECT_NE(TimeSeriesLibrary::datasetFingerprint(rerolled), baseline);
}

TEST(TestTimeSeriesFingerprint, InvariantUnderSeriesOrder)
{
    auto reordered{makeSample()};
    std::swap(reordered.series[0], reordered.series[1]);

    EXPECT_EQ(TimeSeriesLibrary::datasetFingerprint(reordered),
              TimeSeriesLibrary::datasetFingerprint(makeSample()));
}

TEST(TestTimeSeriesEntryPlan, UnchangedDatasetIsSkipped)
{
    const auto dataset{makeSample()};
    const std::vector<std::string> existing{
      ThermZip::timeSeriesEntryName(sampleUuid, FileParse::FileFormat::XML)};

    const auto plan{TimeSeriesLibrary::planEntryUpdates(
      existing, {dataset}, fingerprintsFor(dataset), FileParse::FileFormat::XML)};

    EXPECT_TRUE(plan.toWrite.empty());
    EXPECT_TRUE(plan.toErase.empty());
}

TEST(TestTimeSeriesEntryPlan, MissingEntryIsWritten)
{
    const auto dataset{makeSample()};

    const auto plan{TimeSeriesLibrary::planEntryUpdates(
      {}, {dataset}, fingerprintsFor(dataset), FileParse::FileFormat::XML)};

    ASSERT_EQ(plan.toWrite.size(), 1U);
    EXPECT_EQ(plan.toWrite[0], sampleUuid);
    EXPECT_TRUE(plan.toErase.empty());
}

TEST(TestTimeSeriesEntryPlan, UnknownFingerprintIsWritten)
{
    const auto dataset{makeSample()};
    const std::vector<std::string> existing{
      ThermZip::timeSeriesEntryName(sampleUuid, FileParse::FileFormat::XML)};

    const auto plan{TimeSeriesLibrary::planEntryUpdates(
      existing, {dataset}, {}, FileParse::FileFormat::XML)};

    ASSERT_EQ(plan.toWrite.size(), 1U);
    EXPECT_EQ(plan.toWrite[0], sampleUuid);
}

TEST(TestTimeSeriesEntryPlan, FingerprintMismatchIsWritten)
{
    auto dataset{makeSample()};
    const std::vector<std::string> existing{
      ThermZip::timeSeriesEntryName(sampleUuid, FileParse::FileFormat::XML)};
    const auto staleFingerprints{fingerprintsFor(dataset)};

    dataset.Name = "Renamed after load";
    const auto plan{TimeSeriesLibrary::planEntryUpdates(
      existing, {dataset}, staleFingerprints, FileParse::FileFormat::XML)};

    ASSERT_EQ(plan.toWrite.size(), 1U);
    EXPECT_EQ(plan.toWrite[0], sampleUuid);
    EXPECT_TRUE(plan.toErase.empty());
}

TEST(TestTimeSeriesEntryPlan, FormatSwitchRewritesAndErasesOldSpelling)
{
    const auto dataset{makeSample()};
    const auto xmlName{ThermZip::timeSeriesEntryName(sampleUuid, FileParse::FileFormat::XML)};

    const auto plan{TimeSeriesLibrary::planEntryUpdates(
      {xmlName}, {dataset}, fingerprintsFor(dataset), FileParse::FileFormat::JSON)};

    ASSERT_EQ(plan.toWrite.size(), 1U);
    EXPECT_EQ(plan.toWrite[0], sampleUuid);
    ASSERT_EQ(plan.toErase.size(), 1U);
    EXPECT_EQ(plan.toErase[0], xmlName);
}

TEST(TestTimeSeriesEntryPlan, UnboundEntryIsErased)
{
    const auto dataset{makeSample()};
    const auto boundName{ThermZip::timeSeriesEntryName(sampleUuid, FileParse::FileFormat::XML)};
    const auto unboundName{
      ThermZip::timeSeriesEntryName("ffffffff-0000-1111-2222-333333333333", FileParse::FileFormat::XML)};

    const auto plan{TimeSeriesLibrary::planEntryUpdates({boundName, unboundName},
                                                        {dataset},
                                                        fingerprintsFor(dataset),
                                                        FileParse::FileFormat::XML)};

    EXPECT_TRUE(plan.toWrite.empty());
    ASSERT_EQ(plan.toErase.size(), 1U);
    EXPECT_EQ(plan.toErase[0], unboundName);
}
