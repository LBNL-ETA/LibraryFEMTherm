#include <gtest/gtest.h>

#include "BoundaryConditions/DB.hxx"
#include "BoundaryConditions/Serializers.hxx"

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

    BoundaryCondition makeTransientExterior()
    {
        BoundaryCondition record;
        record.UUID = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee";
        record.Name = "ASHRAE Outside, black body";
        record.Protected = true;
        record.Color = "0x0078D7";

        SurfaceExchange exchange;
        exchange.relativeHumidity = FromTimeSeries{ChannelRole::RelativeHumidity};
        exchange.convection = Convection{.model = ConvectionModel::ASHRAE_NFRC_Outside,
                                         .airTemperature = FromTimeSeries{ChannelRole::AirTemperature},
                                         .filmCoefficient = std::nullopt,
                                         .windSpeed = FromTimeSeries{ChannelRole::WindSpeed},
                                         .windDirection = std::nullopt};
        exchange.radiation = BlackBodyRadiation{.temperature = Constant{-18.0},
                                                .emissivity = Constant{0.9},
                                                .viewFactor = 1.0};
        record.data = exchange;
        return record;
    }

    BoundaryCondition makeSteadyInterior()
    {
        BoundaryCondition record;
        record.UUID = "99999999-8888-7777-6666-555555555555";
        record.Name = "Interior frame";

        SurfaceExchange exchange;
        exchange.relativeHumidity = Constant{0.5};
        exchange.convection = Convection{.model = ConvectionModel::Fixed_Convection_Coefficient,
                                         .airTemperature = Constant{21.0},
                                         .filmCoefficient = Constant{3.29},
                                         .windSpeed = std::nullopt,
                                         .windDirection = std::nullopt};
        exchange.radiation = AutomaticEnclosure{Constant{0.9}};
        record.data = exchange;
        return record;
    }
}   // namespace

TEST(TestUnifiedBC, RequiredRolesTransient)
{
    const auto record{makeTransientExterior()};
    const auto roles{requiredRoles(record)};

    ASSERT_EQ(roles.size(), 3U);
    EXPECT_EQ(roles[0], ChannelRole::RelativeHumidity);
    EXPECT_EQ(roles[1], ChannelRole::AirTemperature);
    EXPECT_EQ(roles[2], ChannelRole::WindSpeed);
    EXPECT_FALSE(isSteadyCapable(record));
}

TEST(TestUnifiedBC, AllConstantRecordIsSteadyCapable)
{
    const auto record{makeSteadyInterior()};
    EXPECT_TRUE(requiredRoles(record).empty());
    EXPECT_TRUE(isSteadyCapable(record));
}

TEST(TestUnifiedBC, SurfaceExchangeRoundTrip)
{
    DB source;
    source.add(makeTransientExterior());

    DB loaded;
    loaded.loadFromString(source.saveToString());

    const auto record{loaded.getByUUID("aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee")};
    ASSERT_TRUE(record.has_value());
    EXPECT_EQ(record->Name, "ASHRAE Outside, black body");
    EXPECT_TRUE(record->Protected);

    const auto * exchange{std::get_if<SurfaceExchange>(&record->data)};
    ASSERT_NE(exchange, nullptr);

    EXPECT_EQ(environmentRole(exchange->relativeHumidity), ChannelRole::RelativeHumidity);

    ASSERT_TRUE(exchange->convection.has_value());
    EXPECT_EQ(exchange->convection->model, ConvectionModel::ASHRAE_NFRC_Outside);
    ASSERT_TRUE(exchange->convection->airTemperature.has_value());
    EXPECT_EQ(environmentRole(exchange->convection->airTemperature.value()), ChannelRole::AirTemperature);
    ASSERT_TRUE(exchange->convection->windSpeed.has_value());
    EXPECT_FALSE(exchange->convection->filmCoefficient.has_value());

    ASSERT_TRUE(exchange->radiation.has_value());
    const auto * blackBody{std::get_if<BlackBodyRadiation>(&exchange->radiation.value())};
    ASSERT_NE(blackBody, nullptr);
    EXPECT_NEAR(constantValue(blackBody->temperature), -18.0, 1e-9);
    EXPECT_NEAR(constantValue(blackBody->emissivity), 0.9, 1e-9);
    EXPECT_NEAR(blackBody->viewFactor, 1.0, 1e-9);
}

TEST(TestUnifiedBC, PrescribedStateRoundTrip)
{
    BoundaryCondition record;
    record.UUID = "12121212-3434-5656-7878-909090909090";
    record.Name = "Fixed temperature and humidity";
    record.data = PrescribedState{.temperature = FromTimeSeries{ChannelRole::PrescribedTemperature},
                                  .relativeHumidity = Constant{0.65}};

    DB source;
    source.add(record);

    DB loaded;
    loaded.loadFromString(source.saveToString());

    const auto reloaded{loaded.getByUUID(record.UUID)};
    ASSERT_TRUE(reloaded.has_value());

    const auto * prescribed{std::get_if<PrescribedState>(&reloaded->data)};
    ASSERT_NE(prescribed, nullptr);
    ASSERT_TRUE(prescribed->temperature.has_value());
    EXPECT_EQ(environmentRole(prescribed->temperature.value()), ChannelRole::PrescribedTemperature);
    ASSERT_TRUE(prescribed->relativeHumidity.has_value());
    EXPECT_NEAR(constantValue(prescribed->relativeHumidity.value()), 0.65, 1e-9);

    const auto roles{requiredRoles(reloaded.value())};
    ASSERT_EQ(roles.size(), 1U);
    EXPECT_EQ(roles[0], ChannelRole::PrescribedTemperature);
}

TEST(TestUnifiedBC, NoExchangeRoundTrip)
{
    // The dedicated adiabatic kind: deliberately no exchange, steady-capable, no
    // required channels, and it survives the XML round trip as its own variant.
    BoundaryCondition record;
    record.UUID = "adiabatic-no-exchange";
    record.Name = "Adiabatic";
    record.Protected = true;
    record.data = NoExchange{};

    DB source;
    source.add(record);

    DB loaded;
    loaded.loadFromString(source.saveToString());

    const auto reloaded{loaded.getByUUID(record.UUID)};
    ASSERT_TRUE(reloaded.has_value());
    EXPECT_TRUE(std::holds_alternative<NoExchange>(reloaded->data));
    EXPECT_TRUE(isSteadyCapable(reloaded.value()));
    EXPECT_TRUE(requiredRoles(reloaded.value()).empty());
}

TEST(TestUnifiedBC, RadiationSurfaceRoundTripAndDefaultLookup)
{
    BoundaryCondition record;
    record.UUID = "abcdabcd-abcd-abcd-abcd-abcdabcdabcd";
    record.Name = "Radiation Surface";
    record.data = RadiationSurface{.isDefault = true, .temperature = 21.0, .emissivity = 0.9};

    DB source;
    source.add(makeSteadyInterior());
    source.add(record);

    DB loaded;
    loaded.loadFromString(source.saveToString());

    const auto defaultSurface{loaded.getDefaultRadiationSurface()};
    ASSERT_TRUE(defaultSurface.has_value());
    EXPECT_EQ(defaultSurface->UUID, record.UUID);
    EXPECT_TRUE(isSteadyCapable(defaultSurface.value()));
}
