#include <gtest/gtest.h>

#include "Legacy/Step1/Converters/Converters.hxx"

using namespace BCLibrary;
using TimeSeriesLibrary::ChannelRole;

namespace
{
    double constantValue(const Source & source)
    {
        return std::get<Constant>(source).value;
    }

    ChannelRole environmentRole(const Source & source)
    {
        return std::get<FromTimeSeries>(source).role;
    }
}   // namespace

TEST(TestUnifiedBCConverters, SteadyComprehensiveWithBlackBody)
{
    BCSteadyStateLibrary::BoundaryCondition legacy;
    legacy.UUID = "4c179ded-3f34-4102-b8d6-46d8e6d1c6d9";
    legacy.Name = "NFRC 100-2010 Exterior";
    legacy.Protected = true;
    legacy.isIGUSurface = false;
    legacy.data = BCSteadyStateLibrary::Comprehensive{
      0.5,
      BCSteadyStateLibrary::Convection{-18.0, 26.0},
      std::nullopt,
      BCSteadyStateLibrary::Radiation{BCSteadyStateLibrary::BlackBodyRadiation{-18.0, 1.0, 1.0}}};

    const auto converted{fromSteadyState(legacy)};

    EXPECT_EQ(converted.UUID, legacy.UUID);
    EXPECT_EQ(converted.Name, legacy.Name);
    EXPECT_TRUE(converted.Protected);
    EXPECT_TRUE(isSteadyCapable(converted));

    const auto * exchange{std::get_if<SurfaceExchange>(&converted.data)};
    ASSERT_NE(exchange, nullptr);
    EXPECT_NEAR(constantValue(exchange->relativeHumidity), 0.5, 1e-9);

    ASSERT_TRUE(exchange->convection.has_value());
    EXPECT_EQ(exchange->convection->model, ConvectionModel::Fixed_Convection_Coefficient);
    EXPECT_NEAR(constantValue(exchange->convection->airTemperature.value()), -18.0, 1e-9);
    EXPECT_NEAR(constantValue(exchange->convection->filmCoefficient.value()), 26.0, 1e-9);

    ASSERT_TRUE(exchange->radiation.has_value());
    const auto * blackBody{std::get_if<BlackBodyRadiation>(&exchange->radiation.value())};
    ASSERT_NE(blackBody, nullptr);
    EXPECT_NEAR(constantValue(blackBody->temperature), -18.0, 1e-9);
    EXPECT_NEAR(blackBody->viewFactor, 1.0, 1e-9);
}

TEST(TestUnifiedBCConverters, SteadySimplifiedAndLinearizedRadiation)
{
    BCSteadyStateLibrary::BoundaryCondition simplified;
    simplified.UUID = "s1";
    simplified.data = BCSteadyStateLibrary::Simplified{21.0, 3.29, 0.4};

    const auto convertedSimplified{fromSteadyState(simplified)};
    const auto * exchange{std::get_if<SurfaceExchange>(&convertedSimplified.data)};
    ASSERT_NE(exchange, nullptr);
    EXPECT_NEAR(constantValue(exchange->relativeHumidity), 0.4, 1e-9);
    ASSERT_TRUE(exchange->convection.has_value());
    EXPECT_NEAR(constantValue(exchange->convection->airTemperature.value()), 21.0, 1e-9);
    EXPECT_FALSE(exchange->radiation.has_value());

    BCSteadyStateLibrary::BoundaryCondition linearized;
    linearized.UUID = "l1";
    linearized.data = BCSteadyStateLibrary::Comprehensive{
      0.5,
      std::nullopt,
      std::nullopt,
      BCSteadyStateLibrary::Radiation{BCSteadyStateLibrary::LinearizedRadiation{20.0, 4.4}}};

    const auto convertedLinearized{fromSteadyState(linearized)};
    const auto * linearExchange{std::get_if<SurfaceExchange>(&convertedLinearized.data)};
    ASSERT_NE(linearExchange, nullptr);
    ASSERT_TRUE(linearExchange->radiation.has_value());
    const auto * fixed{std::get_if<FixedCoefficientRadiation>(&linearExchange->radiation.value())};
    ASSERT_NE(fixed, nullptr);
    EXPECT_NEAR(constantValue(fixed->temperature), 20.0, 1e-9);
    EXPECT_NEAR(constantValue(fixed->coefficient), 4.4, 1e-9);
}

TEST(TestUnifiedBCConverters, SteadyAdiabaticBecomesNoExchange)
{
    // The legacy library expressed adiabatic as a fixed convection with a zero film
    // coefficient and nothing else; the conversion produces the dedicated NoExchange
    // kind instead of carrying the degenerate zeros forward. This is the path both the
    // THM and the legacy THMZ load rely on (global steady library and archive capture
    // both convert through fromSteadyState).
    BCSteadyStateLibrary::BoundaryCondition legacy;
    legacy.UUID = "61d7bd1c-22c6-4ea0-8720-0696e8c194ad";
    legacy.Name = "Adiabatic";
    legacy.Protected = true;
    legacy.data = BCSteadyStateLibrary::Simplified{0.0, 0.0, 0.5};

    const auto converted{fromSteadyState(legacy)};
    EXPECT_TRUE(std::holds_alternative<NoExchange>(converted.data));
    EXPECT_TRUE(isSteadyCapable(converted));
    EXPECT_EQ(converted.UUID, legacy.UUID);

    // A zero film coefficient with radiation still exchanges - it must stay a surface
    // exchange.
    BCSteadyStateLibrary::BoundaryCondition radiating;
    radiating.UUID = "r1";
    radiating.data = BCSteadyStateLibrary::Comprehensive{
      0.5,
      BCSteadyStateLibrary::Convection{0.0, 0.0},
      std::nullopt,
      BCSteadyStateLibrary::Radiation{BCSteadyStateLibrary::LinearizedRadiation{20.0, 4.4}}};
    const auto convertedRadiating{fromSteadyState(radiating)};
    EXPECT_TRUE(std::holds_alternative<SurfaceExchange>(convertedRadiating.data));
}

TEST(TestUnifiedBCConverters, TransientTypeRecordInfersEnvironmentSources)
{
    BCTypesLibrary::TypeRecord legacy;
    legacy.UUID = "t1";
    legacy.Name = "ASHRAE Outside";
    legacy.bcType = BCTypesLibrary::BCType::Transient;
    legacy.bcModel = BCTypesLibrary::BCModel::Neumann;
    legacy.ConvectionBc = BCTypesLibrary::ConvectionRecord{};
    legacy.ConvectionBc->Model = BCTypesLibrary::ConvectionModel::ASHRAE_NFRC_Outside;
    legacy.RadiationBc = BCTypesLibrary::RadiationRecord{};
    legacy.RadiationBc->Model = BCTypesLibrary::RadiationModel::Fixed_Radiation_Coefficient;

    const auto converted{fromTypeRecord(legacy)};

    EXPECT_EQ(converted.UUID, "t1");
    EXPECT_FALSE(isSteadyCapable(converted));

    const auto * exchange{std::get_if<SurfaceExchange>(&converted.data)};
    ASSERT_NE(exchange, nullptr);
    EXPECT_EQ(environmentRole(exchange->relativeHumidity), ChannelRole::RelativeHumidity);

    ASSERT_TRUE(exchange->convection.has_value());
    EXPECT_EQ(exchange->convection->model, ConvectionModel::ASHRAE_NFRC_Outside);
    EXPECT_EQ(environmentRole(exchange->convection->airTemperature.value()), ChannelRole::AirTemperature);
    EXPECT_EQ(environmentRole(exchange->convection->windSpeed.value()), ChannelRole::WindSpeed);
    EXPECT_FALSE(exchange->convection->windDirection.has_value());

    ASSERT_TRUE(exchange->radiation.has_value());
    const auto * fixed{std::get_if<FixedCoefficientRadiation>(&exchange->radiation.value())};
    ASSERT_NE(fixed, nullptr);
    EXPECT_EQ(environmentRole(fixed->temperature), ChannelRole::RadiantTemperature);
    EXPECT_EQ(environmentRole(fixed->coefficient), ChannelRole::RadiativeCoefficient);
}

TEST(TestUnifiedBCConverters, SteadyTypeRecordBecomesAllConstant)
{
    BCTypesLibrary::TypeRecord legacy;
    legacy.UUID = "t2";
    legacy.bcType = BCTypesLibrary::BCType::SteadyState;
    legacy.bcModel = BCTypesLibrary::BCModel::Neumann;
    legacy.UseHumidity = true;
    legacy.Humidity = 0.35;
    legacy.ConvectionBc = BCTypesLibrary::ConvectionRecord{};
    legacy.ConvectionBc->Model = BCTypesLibrary::ConvectionModel::Fixed_Convection_Coefficient;
    legacy.ConvectionBc->Temperature = 21.0;
    legacy.ConvectionBc->FilmCoefficient = 8.0;
    legacy.UseHeatFlux = true;
    legacy.HeatFlux = 15.0;

    const auto converted{fromTypeRecord(legacy)};
    EXPECT_TRUE(isSteadyCapable(converted));

    const auto * exchange{std::get_if<SurfaceExchange>(&converted.data)};
    ASSERT_NE(exchange, nullptr);
    EXPECT_NEAR(constantValue(exchange->relativeHumidity), 0.35, 1e-9);
    EXPECT_NEAR(constantValue(exchange->convection->airTemperature.value()), 21.0, 1e-9);
    EXPECT_NEAR(constantValue(exchange->convection->filmCoefficient.value()), 8.0, 1e-9);
    ASSERT_TRUE(exchange->flux.has_value());
    EXPECT_NEAR(constantValue(exchange->flux.value()), 15.0, 1e-9);
}

TEST(TestUnifiedBCConverters, DirichletTypeRecordBecomesPrescribedState)
{
    BCTypesLibrary::TypeRecord legacy;
    legacy.UUID = "t3";
    legacy.bcType = BCTypesLibrary::BCType::Transient;
    legacy.bcModel = BCTypesLibrary::BCModel::Dirichlet;
    legacy.UseTemperature = true;
    legacy.UseHumidity = true;

    const auto converted{fromTypeRecord(legacy)};

    const auto * prescribed{std::get_if<PrescribedState>(&converted.data)};
    ASSERT_NE(prescribed, nullptr);
    EXPECT_EQ(environmentRole(prescribed->temperature.value()), ChannelRole::PrescribedTemperature);
    EXPECT_EQ(environmentRole(prescribed->relativeHumidity.value()), ChannelRole::PrescribedHumidity);
}

TEST(TestUnifiedBCConverters, TimestepFileDecomposesIntoChannels)
{
    BCInputFileLibrary::BoundaryConditionTimestep legacy;
    legacy.convection.ashraeOutside = {{std::nullopt, 21.3, 0.52, 1.4},
                                       {std::nullopt, 21.1, 0.55, 1.7},
                                       {std::nullopt, 20.8, 0.57, 2.1}};
    legacy.heatFlux = {{std::nullopt, 10.0}, {std::nullopt, 12.0}, {std::nullopt, 14.0}};

    const auto dataset{environmentFromTimestep(legacy, "ORNL Test Exterior")};

    EXPECT_EQ(dataset.Name, "ORNL Test Exterior");
    EXPECT_EQ(TimeSeriesLibrary::steps(dataset), 3U);
    EXPECT_TRUE(TimeSeriesLibrary::hasRole(dataset, ChannelRole::AirTemperature));
    EXPECT_TRUE(TimeSeriesLibrary::hasRole(dataset, ChannelRole::RelativeHumidity));
    EXPECT_TRUE(TimeSeriesLibrary::hasRole(dataset, ChannelRole::WindSpeed));
    EXPECT_TRUE(TimeSeriesLibrary::hasRole(dataset, ChannelRole::HeatFlux));
    EXPECT_FALSE(TimeSeriesLibrary::hasRole(dataset, ChannelRole::WindDirection));

    const auto windSpeed{TimeSeriesLibrary::valuesForRole(dataset, ChannelRole::WindSpeed)};
    ASSERT_TRUE(windSpeed.has_value());
    EXPECT_NEAR(windSpeed->at(2), 2.1, 1e-9);

    // Content-hash identity: same file content yields the same dataset UUID.
    const auto again{environmentFromTimestep(legacy, "Renamed copy")};
    EXPECT_EQ(dataset.UUID, again.UUID);

    auto modified{legacy};
    modified.convection.ashraeOutside[0].temperature += 0.1;
    const auto different{environmentFromTimestep(modified, "ORNL Test Exterior")};
    EXPECT_NE(dataset.UUID, different.UUID);
}

TEST(TestUnifiedBCConverters, BothRadiationTypesKeepFirstRoleOccurrence)
{
    BCInputFileLibrary::BoundaryConditionTimestep legacy;
    legacy.radiation.fixedRadiation = {{std::nullopt, -10.0, 4.4}, {std::nullopt, -11.0, 4.5}};
    legacy.radiation.blackBodyRadiation = {{std::nullopt, -20.0, 0.9}, {std::nullopt, -21.0, 0.9}};

    const auto dataset{environmentFromTimestep(legacy, "Radiation sample")};

    // One channel per role: RadiantTemperature comes from the fixed-radiation rows (first
    // occurrence), Emissivity from the black-body rows, RadiativeCoefficient from fixed.
    const auto radiantTemperature{
      TimeSeriesLibrary::valuesForRole(dataset, ChannelRole::RadiantTemperature)};
    ASSERT_TRUE(radiantTemperature.has_value());
    EXPECT_NEAR(radiantTemperature->at(0), -10.0, 1e-9);

    EXPECT_TRUE(TimeSeriesLibrary::hasRole(dataset, ChannelRole::RadiativeCoefficient));
    EXPECT_TRUE(TimeSeriesLibrary::hasRole(dataset, ChannelRole::Emissivity));
}
