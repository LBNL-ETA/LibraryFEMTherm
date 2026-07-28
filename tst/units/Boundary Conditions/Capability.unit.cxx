#include <gtest/gtest.h>

#include "BoundaryConditions/Capability.hxx"

using BCLibrary::BoundaryCondition;
using BCLibrary::capabilityIssues;
using BCLibrary::Constant;
using BCLibrary::Convection;
using BCLibrary::ConvectionModel;
using BCLibrary::FixedCoefficientRadiation;
using BCLibrary::FromEnvironment;
using BCLibrary::PrescribedState;
using BCLibrary::RadiationSurface;
using BCLibrary::SurfaceExchange;
using BCLibrary::usable;
using EnvironmentDataLibrary::ChannelRole;
using ThermFile::CalculationMode;
using ThermFile::SimulationEngine;

namespace
{
    BoundaryCondition allConstantFixedConvection()
    {
        BoundaryCondition record;
        record.Name = "Interior frame";
        SurfaceExchange exchange;
        exchange.convection = Convection{.model = ConvectionModel::Fixed_Convection_Coefficient,
                                         .airTemperature = Constant{21.0},
                                         .filmCoefficient = Constant{3.29},
                                         .windSpeed = std::nullopt,
                                         .windDirection = std::nullopt};
        record.data = exchange;
        return record;
    }

    BoundaryCondition correlationConvection()
    {
        auto record{allConstantFixedConvection()};
        auto exchange{std::get<SurfaceExchange>(record.data)};
        exchange.convection->model = ConvectionModel::TARP;
        record.data = exchange;
        return record;
    }

    BoundaryCondition environmentSourced()
    {
        auto record{allConstantFixedConvection()};
        auto exchange{std::get<SurfaceExchange>(record.data)};
        exchange.convection->airTemperature = FromEnvironment{ChannelRole::AirTemperature};
        record.data = exchange;
        return record;
    }
}   // namespace

TEST(TestBCCapability, AllConstantFixedConvectionRunsEverywhere)
{
    const auto record{allConstantFixedConvection()};

    EXPECT_TRUE(usable(record, SimulationEngine::Conrad, CalculationMode::cmSteadyState));
    EXPECT_TRUE(usable(record, SimulationEngine::HygroThermFEM, CalculationMode::cmSteadyState));
    EXPECT_TRUE(usable(record, SimulationEngine::HygroThermFEM, CalculationMode::cmTransient));
}

TEST(TestBCCapability, CorrelationConvectionIsHygroThermFEMOnly)
{
    const auto record{correlationConvection()};

    const auto issues{capabilityIssues(record, SimulationEngine::Conrad, CalculationMode::cmSteadyState)};
    ASSERT_EQ(issues.size(), 1U);
    EXPECT_EQ(issues[0].feature, "TARP");
    EXPECT_EQ(issues[0].reason, "not supported by the Conrad engine");

    EXPECT_TRUE(usable(record, SimulationEngine::HygroThermFEM, CalculationMode::cmSteadyState));
    EXPECT_TRUE(usable(record, SimulationEngine::HygroThermFEM, CalculationMode::cmTransient));
}

TEST(TestBCCapability, EnvironmentSourcesNeedTransientHygroThermFEM)
{
    const auto record{environmentSourced()};

    EXPECT_FALSE(usable(record, SimulationEngine::Conrad, CalculationMode::cmSteadyState));

    const auto steadyIssues{capabilityIssues(record, SimulationEngine::HygroThermFEM, CalculationMode::cmSteadyState)};
    ASSERT_EQ(steadyIssues.size(), 1U);
    EXPECT_EQ(steadyIssues[0].feature, "AirTemperature");
    EXPECT_EQ(steadyIssues[0].reason, "requires the transient calculation mode");

    EXPECT_TRUE(usable(record, SimulationEngine::HygroThermFEM, CalculationMode::cmTransient));
}

TEST(TestBCCapability, PrescribedStateHasNoConradPath)
{
    BoundaryCondition record;
    PrescribedState prescribed;
    prescribed.temperature = Constant{20.0};
    record.data = prescribed;

    const auto issues{capabilityIssues(record, SimulationEngine::Conrad, CalculationMode::cmSteadyState)};
    ASSERT_EQ(issues.size(), 1U);
    EXPECT_EQ(issues[0].feature, "Prescribed state");

    EXPECT_TRUE(usable(record, SimulationEngine::HygroThermFEM, CalculationMode::cmSteadyState));
    EXPECT_TRUE(usable(record, SimulationEngine::HygroThermFEM, CalculationMode::cmTransient));
}

TEST(TestBCCapability, RadiationOnlyFixedCoefficientFlaggedUnderConrad)
{
    BoundaryCondition record;
    SurfaceExchange exchange;
    exchange.radiation = FixedCoefficientRadiation{Constant{15.0}, Constant{4.0}};
    record.data = exchange;

    const auto issues{capabilityIssues(record, SimulationEngine::Conrad, CalculationMode::cmSteadyState)};
    ASSERT_EQ(issues.size(), 1U);
    EXPECT_EQ(issues[0].feature, "Fixed-coefficient radiation");

    EXPECT_TRUE(usable(record, SimulationEngine::HygroThermFEM, CalculationMode::cmSteadyState));
}

TEST(TestBCCapability, RadiationSurfaceRecordsRunEverywhere)
{
    BoundaryCondition record;
    record.data = RadiationSurface{.isDefault = true, .temperature = 20.0, .emissivity = 0.9};

    EXPECT_TRUE(usable(record, SimulationEngine::Conrad, CalculationMode::cmSteadyState));
    EXPECT_TRUE(usable(record, SimulationEngine::HygroThermFEM, CalculationMode::cmSteadyState));
    EXPECT_TRUE(usable(record, SimulationEngine::HygroThermFEM, CalculationMode::cmTransient));
}
