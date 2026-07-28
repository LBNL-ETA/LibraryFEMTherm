#include <filesystem>

#include <gtest/gtest.h>

#include "Legacy/Step1/BCSteadyState/DB.hxx"
#include "THMZ/Model/DB.hxx"
#include "THMZ/Model/migration/BCConsolidation.hxx"

namespace
{
    std::string fixturePath(const std::string & fileName)
    {
        std::filesystem::path path(TEST_DATA_DIR);
        path /= "products";
        path /= fileName;
        return path.string();
    }

    std::string legacySteadyLibraryXml()
    {
        BCSteadyStateLibrary::DB legacyDB;
        BCSteadyStateLibrary::BoundaryCondition record;
        record.UUID = "11111111-2222-3333-4444-555555555555";
        record.Name = "Exterior";
        legacyDB.add(record);
        return legacyDB.saveToString();
    }
}   // namespace

TEST(TestBCConsolidationMigration, SteadyNameBindingBecomesUUIDBinding)
{
    const auto legacy{ThermFile::Migration::BCConsolidation::capture(legacySteadyLibraryXml(), "", {})};

    ASSERT_EQ(legacy.boundaryConditions.size(), 1U);
    EXPECT_EQ(legacy.bcUUIDByName.at("Exterior"), "11111111-2222-3333-4444-555555555555");

    ThermFile::ThermModel model;
    ThermFile::Boundary segment;
    segment.name = "Exterior";
    segment.status = 0;
    model.boundaryConditions.push_back(segment);

    const auto migrated{ThermFile::Migration::BCConsolidation::applyToModel(legacy, model)};

    ASSERT_TRUE(migrated.boundaryConditions[0].bcUUID.has_value());
    EXPECT_EQ(migrated.boundaryConditions[0].bcUUID.value(), "11111111-2222-3333-4444-555555555555");
}

TEST(TestBCConsolidationMigration, MigrationIsIdempotentAndBestEffort)
{
    const auto legacy{ThermFile::Migration::BCConsolidation::capture(legacySteadyLibraryXml(), "", {})};

    ThermFile::ThermModel model;
    ThermFile::Boundary bound{};
    bound.name = "Not in the library";
    bound.status = 0;
    model.boundaryConditions.push_back(bound);

    const auto once{ThermFile::Migration::BCConsolidation::applyToModel(legacy, model)};
    EXPECT_FALSE(once.boundaryConditions[0].bcUUID.has_value());

    const auto twice{ThermFile::Migration::BCConsolidation::applyToModel(legacy, once)};
    EXPECT_FALSE(twice.boundaryConditions[0].bcUUID.has_value());

    const ThermFile::Migration::BCConsolidation::LegacyBCCapture emptyCapture;
    const auto untouched{ThermFile::Migration::BCConsolidation::applyToModel(emptyCapture, model)};
    EXPECT_FALSE(untouched.boundaryConditions[0].bcUUID.has_value());
}

TEST(TestBCConsolidationMigration, SteadyArchiveSegmentsGetUUIDBindings)
{
    const auto zipPath{fixturePath("sample-sill.thmz")};

    const auto legacy{ThermFile::Migration::BCConsolidation::captureFromArchive(zipPath)};
    EXPECT_FALSE(ThermFile::Migration::BCConsolidation::isEmpty(legacy));
    EXPECT_TRUE(legacy.datasets.empty());

    const auto model{ThermFile::loadThermModelFromZipFile(zipPath)};
    ASSERT_TRUE(model.has_value());

    size_t bound = 0;
    for(const auto & segment : model->boundaryConditions)
    {
        if(segment.bcUUID.has_value())
        {
            ++bound;
            const auto expected{legacy.bcUUIDByName.find(segment.name)};
            ASSERT_NE(expected, legacy.bcUUIDByName.end());
            EXPECT_EQ(segment.bcUUID.value(), expected->second);
        }
    }
    EXPECT_GT(bound, 0U);
}

TEST(TestBCConsolidationMigration, TransientArchiveTimestepsBecomeDatasets)
{
    const auto zipPath{fixturePath("StuccoWallMoisture.thmz")};

    const auto legacy{ThermFile::Migration::BCConsolidation::captureFromArchive(zipPath)};

    // The archive embeds two timestep files; the type-record library entry is an
    // empty root element, a real-world quirk this migration tolerates best-effort.
    ASSERT_EQ(legacy.datasets.size(), 2U);
    EXPECT_TRUE(legacy.datasetUUIDByFileName.contains("BC_TS_Fixed_T_RH_HC_120 steps-Exterior.xml"));
    EXPECT_TRUE(legacy.datasetUUIDByFileName.contains("BC_TS_Fixed_T_RH_HC_120 steps-Interior.xml"));

    const auto model{ThermFile::loadThermModelFromZipFile(zipPath)};
    ASSERT_TRUE(model.has_value());

    size_t transientSegments = 0;
    size_t withDataset = 0;
    for(const auto & segment : model->boundaryConditions)
    {
        if(segment.transientRecordData.has_value())
        {
            ++transientSegments;
            // No type records were embedded, so the record binding stays legacy-only.
            EXPECT_FALSE(segment.bcUUID.has_value());
            if(segment.environmentUUID.has_value())
            {
                ++withDataset;
                const auto expected{
                  legacy.datasetUUIDByFileName.find(segment.transientRecordData->transientFileName)};
                ASSERT_NE(expected, legacy.datasetUUIDByFileName.end());
                EXPECT_EQ(segment.environmentUUID.value(), expected->second);
            }
        }
    }
    EXPECT_GT(transientSegments, 0U);
    EXPECT_GT(withDataset, 0U);
}

TEST(TestBCConsolidationMigration, MinimalSteadyArchiveMigrates)
{
    const auto zipPath{fixturePath("Spacer.thmz")};

    const auto model{ThermFile::loadThermModelFromZipFile(zipPath)};
    ASSERT_TRUE(model.has_value());

    const auto legacy{ThermFile::Migration::BCConsolidation::captureFromArchive(zipPath)};
    EXPECT_FALSE(ThermFile::Migration::BCConsolidation::isEmpty(legacy));
}
