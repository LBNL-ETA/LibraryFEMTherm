#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include "BoundaryConditions/DB.hxx"
#include "Gases/DB.hxx"
#include "Legacy/Step1/BCSteadyState/DB.hxx"
#include "Legacy/Step1/BCTransient/Types.hxx"
#include "Materials/DB.hxx"
#include "TimeSeriesData/DB.hxx"

//! A library opened on a file that does not exist is an empty library, and opening it writes
//! nothing: the file appears when the library is saved, which is a decision the application
//! makes on purpose.
namespace
{
    [[nodiscard]] std::filesystem::path missingFile(const std::string & name)
    {
        auto path{std::filesystem::temp_directory_path() / name};
        std::filesystem::remove(path);
        return path;
    }

    template<typename Library, typename CountRecords>
    void expectEmptyUntilSaved(const std::string & name, CountRecords count)
    {
        const auto path{missingFile(name)};
        Library library{path.string()};
        EXPECT_EQ(count(library), 0U);
        EXPECT_FALSE(std::filesystem::exists(path)) << "opening wrote " << path;

        EXPECT_EQ(library.saveToFile(), 0);
        ASSERT_TRUE(std::filesystem::exists(path));
        Library reopened{path.string()};
        EXPECT_EQ(count(reopened), 0U);
        std::filesystem::remove(path);
    }
}   // namespace

TEST(MissingLibraryFile, Materials)
{
    expectEmptyUntilSaved<MaterialsLibrary::DB>("missing_materials.xml",
                                                [](MaterialsLibrary::DB & library) { return library.getMaterials().size(); });
}

TEST(MissingLibraryFile, BoundaryConditions)
{
    expectEmptyUntilSaved<BCLibrary::DB>("missing_boundary_conditions.xml",
                                         [](BCLibrary::DB & library) { return library.getBoundaryConditions().size(); });
}

TEST(MissingLibraryFile, Gases)
{
    expectEmptyUntilSaved<GasesLibrary::DB>("missing_gases.xml", [](GasesLibrary::DB & library) {
        return library.getGases().size() + library.getPureGases().size();
    });
}

TEST(MissingLibraryFile, TimeSeriesData)
{
    expectEmptyUntilSaved<TimeSeriesLibrary::DB>("missing_time_series.xml",
                                                 [](TimeSeriesLibrary::DB & library) { return library.getTimeSeriesData().size(); });
}

TEST(MissingLibraryFile, LegacySteadyStateBoundaryConditions)
{
    expectEmptyUntilSaved<BCSteadyStateLibrary::DB>(
      "missing_bc_steady_state.xml", [](BCSteadyStateLibrary::DB & library) { return library.getBoundaryConditions().size(); });
}

TEST(MissingLibraryFile, LegacyTransientBoundaryConditionTypes)
{
    expectEmptyUntilSaved<BCTypesLibrary::DB>("missing_bc_transient_types.xml",
                                              [](BCTypesLibrary::DB & library) { return library.getBoundaryConditions().size(); });
}

TEST(MissingLibraryFile, GasesSavesTheCurrentVersion)
{
    const auto path{missingFile("missing_gases_version.xml")};
    GasesLibrary::DB library{path.string()};
    ASSERT_EQ(library.saveToFile(), 0);

    std::ifstream saved(path);
    std::stringstream content;
    content << saved.rdbuf();
    EXPECT_NE(content.str().find("<Version>1</Version>"), std::string::npos) << content.str();
    saved.close();
    std::filesystem::remove(path);
}
