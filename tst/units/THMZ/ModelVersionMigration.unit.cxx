#include <map>
#include <string>

#include <gtest/gtest.h>

#include "THMZ/Model/THMX.hxx"
#include "THMZ/Model/migration/Pipeline.hxx"

namespace
{
    constexpr double storedTolerance{1e-5};
    constexpr double conradTolerance{1e-6};
    constexpr double comparisonLimit{1e-12};

    ThermFile::ThermModel makeModel(const std::string & version, const ThermFile::SimulationEngine engine)
    {
        ThermFile::ThermModel model;
        model.version = version;
        model.properties.calculationOptions.simulationEngine = engine;
        model.properties.calculationOptions.engineParameters.convergenceTolerance = storedTolerance;
        return model;
    }

    ThermFile::ThermModel migrate(ThermFile::ThermModel model)
    {
        return ThermFile::Migration::applyAllToModel(std::map<std::string, std::string>{}, std::move(model));
    }

    double toleranceOf(const ThermFile::ThermModel & model)
    {
        return model.properties.calculationOptions.engineParameters.convergenceTolerance;
    }
}   // namespace

TEST(ModelVersionMigration, OlderConradFileTakesTheToleranceItSolvedAt)
{
    const auto model{migrate(makeModel("3", ThermFile::SimulationEngine::Conrad))};

    EXPECT_NEAR(conradTolerance, toleranceOf(model), comparisonLimit);
}

TEST(ModelVersionMigration, OlderHygroThermFileKeepsItsTolerance)
{
    const auto model{migrate(makeModel("3", ThermFile::SimulationEngine::HygroThermFEM))};

    EXPECT_NEAR(storedTolerance, toleranceOf(model), comparisonLimit);
}

TEST(ModelVersionMigration, CurrentConradFileKeepsTheUsersTolerance)
{
    const auto model{
      migrate(makeModel(std::to_string(ThermFile::currentModelVersion), ThermFile::SimulationEngine::Conrad))};

    EXPECT_NEAR(storedTolerance, toleranceOf(model), comparisonLimit);
}

TEST(ModelVersionMigration, UnreadableVersionCountsAsOldest)
{
    const auto model{migrate(makeModel("not a number", ThermFile::SimulationEngine::Conrad))};

    EXPECT_NEAR(conradTolerance, toleranceOf(model), comparisonLimit);
    EXPECT_EQ(std::to_string(ThermFile::currentModelVersion), model.version);
}

TEST(ModelVersionMigration, LoadStampsTheCurrentVersion)
{
    const auto model{migrate(makeModel("1", ThermFile::SimulationEngine::Conrad))};

    EXPECT_EQ(std::to_string(ThermFile::currentModelVersion), model.version);
}

TEST(ModelVersionMigration, NewerFileKeepsItsVersion)
{
    const auto newerVersion{std::to_string(ThermFile::currentModelVersion + 1)};

    const auto model{migrate(makeModel(newerVersion, ThermFile::SimulationEngine::Conrad))};

    EXPECT_EQ(newerVersion, model.version);
    EXPECT_NEAR(storedTolerance, toleranceOf(model), comparisonLimit);
}

TEST(ModelVersionMigration, SecondLoadLeavesTheUsersToleranceAlone)
{
    auto model{migrate(makeModel("3", ThermFile::SimulationEngine::Conrad))};
    model.properties.calculationOptions.engineParameters.convergenceTolerance = storedTolerance;

    const auto reloaded{migrate(std::move(model))};

    EXPECT_NEAR(storedTolerance, toleranceOf(reloaded), comparisonLimit);
}
