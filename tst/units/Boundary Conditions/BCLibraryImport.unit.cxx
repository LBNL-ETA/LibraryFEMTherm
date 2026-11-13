#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <variant>

#include <gtest/gtest.h>

#include "BoundaryConditions/DB.hxx"
#include "Legacy/LibraryImport/Pipeline.hxx"
#include "Legacy/Step1/BCSteadyState/DB.hxx"
#include "Legacy/Step1/BCTransient/Types.hxx"

namespace
{
    //! Copies of the libraries a previous THERM 8 version keeps in its lib folder.
    std::string productFile(const std::string & fileName)
    {
        std::ifstream file{std::filesystem::path{TEST_DATA_DIR} / "products" / fileName, std::ios::binary};
        std::stringstream text;
        text << file.rdbuf();
        return text.str();
    }

    double constantValue(const std::optional<BCLibrary::Source> & source)
    {
        EXPECT_TRUE(source.has_value());
        const auto * constant{source.has_value() ? std::get_if<BCLibrary::Constant>(&source.value()) : nullptr};
        EXPECT_NE(constant, nullptr);
        return constant != nullptr ? constant->value : 0.0;
    }

    const BCLibrary::BoundaryCondition * named(const BCLibrary::Import::ImportedRecords & imported,
                                               const std::string & name)
    {
        for(const auto & record : imported.records)
        {
            if(record.Name == name)
            {
                return &record;
            }
        }
        return nullptr;
    }
}   // namespace

TEST(BCLibraryImport, SteadyStateLibraryKeepsItsValues)
{
    const auto imported{BCLibrary::Import::recordsFromLibrary(productFile("BoundaryConditionsSteadyState.xml"))};

    ASSERT_TRUE(imported.has_value());
    EXPECT_EQ(imported->records.size(), 13u);
    EXPECT_EQ(imported->notApplicable, 1u);
    EXPECT_EQ(imported->projectRecords, 0u);

    const auto * record{named(imported.value(), "Interior (20 tilt) Aluminum Frame (convection only)")};
    ASSERT_NE(record, nullptr);
    EXPECT_EQ(record->UUID, "4c179ded-3f34-4102-b8d6-46d8e6d1c6d9");
    const auto * exchange{std::get_if<BCLibrary::SurfaceExchange>(&record->data)};
    ASSERT_NE(exchange, nullptr);
    ASSERT_TRUE(exchange->convection.has_value());
    EXPECT_DOUBLE_EQ(constantValue(exchange->convection->airTemperature), 21.0);
    EXPECT_DOUBLE_EQ(constantValue(exchange->convection->filmCoefficient), 4.65);
    EXPECT_TRUE(exchange->radiation.has_value());
}

TEST(BCLibraryImport, TransientLibraryImportsEveryRecord)
{
    const auto imported{BCLibrary::Import::recordsFromLibrary(productFile("BoundaryConditionsTransient.xml"))};

    ASSERT_TRUE(imported.has_value());
    EXPECT_EQ(imported->records.size(), 11u);
    EXPECT_EQ(imported->notApplicable, 0u);
}

TEST(BCLibraryImport, CurrentLibraryIsNotMistakenForTheSteadyStateOne)
{
    BCLibrary::BoundaryCondition record{.UUID = "shared-uuid", .Name = "Shared record"};
    record.data = BCLibrary::NoExchange{};
    BCLibrary::DB library;
    library.add(record);

    const auto imported{BCLibrary::Import::recordsFromLibrary(library.toString())};

    ASSERT_TRUE(imported.has_value());
    ASSERT_EQ(imported->records.size(), 1u);
    EXPECT_TRUE(std::holds_alternative<BCLibrary::NoExchange>(imported->records.front().data));
}

//! THERM's writer puts a byte order mark on every library, so the shipped copies have one.
TEST(BCLibraryImport, FileWithoutAByteOrderMarkReadsTheSame)
{
    const std::string bytesOfTheMark{"\xEF\xBB\xBF"};
    const auto text{productFile("BoundaryConditionsSteadyState.xml")};
    ASSERT_TRUE(text.starts_with(bytesOfTheMark));

    const auto imported{BCLibrary::Import::recordsFromLibrary(text.substr(bytesOfTheMark.size()))};

    ASSERT_TRUE(imported.has_value());
    EXPECT_EQ(imported->records.size(), 13u);
}

TEST(BCLibraryImport, ProjectRecordsAndRadiationSurfacesStayOut)
{
    BCLibrary::BoundaryCondition project{.UUID = "project-uuid", .Name = "Film", .ProjectName = "Model"};
    project.data = BCLibrary::NoExchange{};
    BCLibrary::BoundaryCondition surface{.UUID = "surface-uuid", .Name = "Surface"};
    surface.data = BCLibrary::RadiationSurface{};
    BCLibrary::DB library;
    library.add(project);
    library.add(surface);

    const auto imported{BCLibrary::Import::recordsFromLibrary(library.toString())};

    ASSERT_TRUE(imported.has_value());
    EXPECT_TRUE(imported->records.empty());
    EXPECT_EQ(imported->projectRecords, 1u);
    EXPECT_EQ(imported->notApplicable, 1u);
}

TEST(BCLibraryImport, OtherFilesAreNotLibraries)
{
    EXPECT_FALSE(BCLibrary::Import::recordsFromLibrary("<?xml version=\"1.0\"?>\n<Materials>\n</Materials>\n"));
    EXPECT_FALSE(BCLibrary::Import::recordsFromLibrary("<InputBoundaryConditionsData></InputBoundaryConditionsData>"));
    EXPECT_FALSE(BCLibrary::Import::recordsFromLibrary("not xml at all"));
    EXPECT_FALSE(BCLibrary::Import::recordsFromLibrary(""));
}
