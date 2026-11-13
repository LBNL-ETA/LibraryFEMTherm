#include <map>
#include <string>
#include <variant>

#include <gtest/gtest.h>

#include "THMZ/Model/DB.hxx"
#include "THMZ/Model/THMX.hxx"
#include "THMZ/Model/migration/Pipeline.hxx"
#include "THMZ/Model/migration/TimestepFiles.hxx"
#include "THMZ/ZipModule/ZipModule.hxx"
#include "TimeSeriesData/DB.hxx"

namespace
{
    using Entries = std::map<std::string, std::string>;

    const std::string fileName{"BC_TS_Exterior.xml"};
    const std::string modelEntryName{ThermZip::ModelFileName + ".xml"};

    std::string timestepRow(const int index, const int temperatureOffset)
    {
        return "<FixedConvectionFilmTimestep><Index>" + std::to_string(index) + "</Index><Temperature>"
               + std::to_string(index + temperatureOffset)
               + "</Temperature><Humidity>0.5</Humidity><FilmCoefficient>7</FilmCoefficient>"
                 "</FixedConvectionFilmTimestep>";
    }

    std::string timestepFile(const int rows, const int temperatureOffset = 0)
    {
        std::string text{"<?xml version=\"1.0\"?><InputBoundaryConditionsData><Version>1</Version>"
                         "<Name>Test</Name><BoundaryConditionTransient><ConvectionTimesteps>"};
        for(int index = 1; index <= rows; ++index)
        {
            text += timestepRow(index, temperatureOffset);
        }
        return text + "</ConvectionTimesteps></BoundaryConditionTransient></InputBoundaryConditionsData>";
    }

    ThermFile::ThermModel transientModel()
    {
        ThermFile::ThermModel model;
        model.version = "3";
        model.properties.calculationOptions.calculationMode = ThermFile::CalculationMode::cmTransient;
        ThermFile::Boundary boundary;
        boundary.transientRecordData = ThermFile::TransientData{.typeRecordUUID = "record", .transientFileName = fileName};
        model.boundaryConditions.push_back(boundary);
        return model;
    }

    //! The old format's clock lives in Model.xml, which the current format no longer writes.
    std::string modelEntry(const ThermFile::ThermModel & model)
    {
        auto text{ThermFile::toString(model)};
        const std::string closing{"</ThermModel>"};
        text.insert(text.rfind(closing), "<TimeStep>360</TimeStep><NumberOfTimesteps>3</NumberOfTimesteps>");
        return text;
    }

    Entries archiveEntries(const ThermFile::ThermModel & model, const int rows = 5)
    {
        return Entries{{modelEntryName, modelEntry(model)},
                       {ThermZip::TimestepFilesDir + "/" + fileName, timestepFile(rows)}};
    }

    TimeSeriesLibrary::TimeSeriesData onlyDataset(const Entries & entries)
    {
        const auto datasets{TimeSeriesLibrary::datasetsFromThmzEntries(entries)};
        EXPECT_EQ(1u, datasets.size());
        return datasets.front();
    }
}   // namespace

TEST(TimestepFilesMigration, FileBecomesAnEmbeddedDataset)
{
    const auto dataset{onlyDataset(ThermFile::Migration::TimestepFiles::withConvertedFiles(archiveEntries(transientModel())))};

    EXPECT_TRUE(TimeSeriesLibrary::hasRole(dataset, TimeSeriesLibrary::SeriesRole::AirTemperature));
    EXPECT_TRUE(TimeSeriesLibrary::hasRole(dataset, TimeSeriesLibrary::SeriesRole::ConvectiveCoefficient));
    EXPECT_EQ(std::string{"BC_TS_Exterior"}, dataset.Name);
}

TEST(TimestepFilesMigration, DatasetTakesTheModelsClock)
{
    const auto dataset{onlyDataset(ThermFile::Migration::TimestepFiles::withConvertedFiles(archiveEntries(transientModel())))};

    EXPECT_EQ(3u, TimeSeriesLibrary::steps(dataset));
    EXPECT_DOUBLE_EQ(360.0, std::get<double>(dataset.axis.stepSeconds));
}

TEST(TimestepFilesMigration, ShorterFileKeepsItsRows)
{
    const auto entries{archiveEntries(transientModel(), 2)};

    EXPECT_EQ(2u, TimeSeriesLibrary::steps(onlyDataset(ThermFile::Migration::TimestepFiles::withConvertedFiles(entries))));
}

TEST(TimestepFilesMigration, FilesOfDifferentLengthsLineUp)
{
    auto model{transientModel()};
    const std::string otherFile{"BC_TS_Interior.xml"};
    ThermFile::Boundary boundary;
    boundary.transientRecordData = ThermFile::TransientData{.typeRecordUUID = "record", .transientFileName = otherFile};
    model.boundaryConditions.push_back(boundary);
    auto entries{archiveEntries(model, 500)};
    entries[ThermZip::TimestepFilesDir + "/" + otherFile] = timestepFile(1000, 100);

    const auto datasets{TimeSeriesLibrary::datasetsFromThmzEntries(ThermFile::Migration::TimestepFiles::withConvertedFiles(entries))};

    ASSERT_EQ(2u, datasets.size());
    EXPECT_EQ(TimeSeriesLibrary::steps(datasets[0]), TimeSeriesLibrary::steps(datasets[1]));
    EXPECT_TRUE(datasets[0].axis == datasets[1].axis);
}

TEST(TimestepFilesMigration, LoadedModelReadsTheDataset)
{
    const auto entries{ThermFile::Migration::TimestepFiles::withConvertedFiles(archiveEntries(transientModel()))};

    const auto model{ThermFile::loadThermModelFromEntries(entries)};

    ASSERT_TRUE(model.has_value());
    const auto & boundary{model->boundaryConditions.front()};
    EXPECT_EQ(onlyDataset(entries).UUID, boundary.timeSeriesUUID);
    EXPECT_TRUE(boundary.transientRecordData->transientFileName.empty());
}

TEST(TimestepFilesMigration, FileInTheArchiveRootConverts)
{
    const auto model{transientModel()};
    const Entries entries{{modelEntryName, modelEntry(model)}, {fileName, timestepFile(5)}};

    EXPECT_EQ(3u, TimeSeriesLibrary::steps(onlyDataset(ThermFile::Migration::TimestepFiles::withConvertedFiles(entries))));
}

TEST(TimestepFilesMigration, SteadyStateModelIsLeftAlone)
{
    auto model{transientModel()};
    model.properties.calculationOptions.calculationMode = ThermFile::CalculationMode::cmSteadyState;

    const auto entries{ThermFile::Migration::TimestepFiles::withConvertedFiles(archiveEntries(model))};

    EXPECT_TRUE(TimeSeriesLibrary::datasetsFromThmzEntries(entries).empty());
}

TEST(TimestepFilesMigration, BoundaryWithADatasetIsLeftAlone)
{
    auto model{transientModel()};
    model.boundaryConditions.front().timeSeriesUUID = "already bound";

    const auto entries{ThermFile::Migration::TimestepFiles::withConvertedFiles(archiveEntries(model))};

    EXPECT_TRUE(TimeSeriesLibrary::datasetsFromThmzEntries(entries).empty());
}

TEST(TimestepFilesMigration, UnreadableFileKeepsItsBoundary)
{
    const auto model{transientModel()};
    const Entries source{{modelEntryName, modelEntry(model)},
                         {ThermZip::TimestepFilesDir + "/" + fileName, "this is not a timestep file"}};

    const auto entries{ThermFile::Migration::TimestepFiles::withConvertedFiles(source)};
    const auto loaded{ThermFile::loadThermModelFromEntries(entries)};

    EXPECT_TRUE(TimeSeriesLibrary::datasetsFromThmzEntries(entries).empty());
    ASSERT_TRUE(loaded.has_value());
    EXPECT_FALSE(loaded->boundaryConditions.front().timeSeriesUUID.has_value());
    EXPECT_EQ(fileName, loaded->boundaryConditions.front().transientRecordData->transientFileName);
}

TEST(TimestepFilesMigration, MissingFileKeepsItsBoundary)
{
    const auto model{transientModel()};
    const Entries entries{{modelEntryName, modelEntry(model)}};

    const auto loaded{ThermFile::loadThermModelFromEntries(ThermFile::Migration::TimestepFiles::withConvertedFiles(entries))};

    ASSERT_TRUE(loaded.has_value());
    EXPECT_FALSE(loaded->boundaryConditions.front().timeSeriesUUID.has_value());
}

TEST(TimestepFilesMigration, PipelineConvertsTheFileAndBindsItsBoundary)
{
    const auto entries{ThermFile::Migration::applyAllToEntries(archiveEntries(transientModel()))};

    const auto loaded{ThermFile::loadThermModelFromEntries(entries)};

    ASSERT_TRUE(loaded.has_value());
    const auto & boundary{loaded->boundaryConditions.front()};
    ASSERT_TRUE(boundary.timeSeriesUUID.has_value());
    EXPECT_TRUE(boundary.transientRecordData->transientFileName.empty());
}

TEST(TimestepFilesMigration, PipelineLeavesAnArchiveWithoutTimestepFilesAlone)
{
    auto model{transientModel()};
    model.boundaryConditions.front().transientRecordData->transientFileName.clear();
    const Entries source{{modelEntryName, modelEntry(model)}};

    EXPECT_EQ(source, ThermFile::Migration::applyAllToEntries(source));
}

TEST(TimestepFilesMigration, ModelLoadedWithoutTheDatasetKeepsTheFileName)
{
    const auto loaded{ThermFile::loadThermModelFromEntries(archiveEntries(transientModel()))};

    ASSERT_TRUE(loaded.has_value());
    EXPECT_FALSE(loaded->boundaryConditions.front().timeSeriesUUID.has_value());
    EXPECT_EQ(fileName, loaded->boundaryConditions.front().transientRecordData->transientFileName);
}
