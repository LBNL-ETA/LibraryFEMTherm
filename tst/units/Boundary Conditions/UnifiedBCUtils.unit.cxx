#include <gtest/gtest.h>

#include "BoundaryConditions/Utils.hxx"
#include "Legacy/Step1/BCSteadyState/Utils.hxx"
#include "Legacy/Step1/Converters/Converters.hxx"

using namespace BCLibrary;

namespace
{
    BCSteadyStateLibrary::BoundaryCondition legacyRecord(const std::string & name)
    {
        BCSteadyStateLibrary::BoundaryCondition legacy;
        legacy.UUID = "0f6f2c11-8f4c-4a0a-9f2e-1c2b3d4e5f60";
        legacy.Name = name;
        return legacy;
    }

    //! The unified accessors have to answer exactly what the legacy ones answer about the
    //! same record, or moving a consumer from one to the other changes the solver input.
    void expectSameAnswers(const BCSteadyStateLibrary::BoundaryCondition & legacy)
    {
        const auto unified{fromSteadyState(legacy)};
        const auto roundTripped{toSteadyState(unified)};
        ASSERT_TRUE(roundTripped.has_value()) << roundTripped.error();
        const auto & steady{roundTripped.value()};

        EXPECT_EQ(isRadiationSurface(unified), BCSteadyStateLibrary::isRadiationSurface(steady)) << legacy.Name;
        EXPECT_EQ(isAdiabatic(unified), BCSteadyStateLibrary::isAdiabatic(steady)) << legacy.Name;
        EXPECT_EQ(isConvective(unified), BCSteadyStateLibrary::isConvective(steady)) << legacy.Name;
        EXPECT_EQ(isConstantFlux(unified), BCSteadyStateLibrary::isConstantFlux(steady)) << legacy.Name;
        EXPECT_EQ(isAutoGreyBodyRadiation(unified), BCSteadyStateLibrary::isAutoGreyBodyRadiation(steady))
          << legacy.Name;
        EXPECT_EQ(isGreyBodyRadiation(unified), BCSteadyStateLibrary::isGreyBodyRadiation(steady)) << legacy.Name;
        EXPECT_EQ(isBlackBodyRadiation(unified), BCSteadyStateLibrary::isBlackBodyRadiation(steady)) << legacy.Name;
        EXPECT_EQ(isLinearizedRadiation(unified), BCSteadyStateLibrary::isLinearizedRadiation(steady)) << legacy.Name;
        EXPECT_EQ(isRadiationBC(unified), BCSteadyStateLibrary::isRadiationBC(steady)) << legacy.Name;

        EXPECT_EQ(getRelativeHumidity(unified), BCSteadyStateLibrary::getRelativeHumidity(steady)) << legacy.Name;
        EXPECT_EQ(airTemperature(unified), BCSteadyStateLibrary::airTemperature(steady)) << legacy.Name;
        EXPECT_EQ(filmCoefficient(unified), BCSteadyStateLibrary::filmCoefficient(steady)) << legacy.Name;
        EXPECT_EQ(radiationTemperature(unified), BCSteadyStateLibrary::radiationTemperature(steady)) << legacy.Name;
        EXPECT_EQ(radiationFilmCoefficient(unified), BCSteadyStateLibrary::radiationFilmCoefficient(steady))
          << legacy.Name;
        EXPECT_EQ(emissivity(unified), BCSteadyStateLibrary::emissivity(steady)) << legacy.Name;
        EXPECT_EQ(constantHeatFlux(unified), BCSteadyStateLibrary::constantHeatFlux(steady)) << legacy.Name;
        EXPECT_EQ(viewFactor(unified), BCSteadyStateLibrary::viewFactor(steady)) << legacy.Name;
    }
}   // namespace

TEST(TestUnifiedBCAccessors, AutomaticEnclosureMatchesTheLegacyAnswers)
{
    auto legacy{legacyRecord("Interior Frame")};
    BCSteadyStateLibrary::Comprehensive comprehensive;
    comprehensive.relativeHumidity = 0.5;
    comprehensive.convection = BCSteadyStateLibrary::Convection{21.0, 4.65};
    comprehensive.radiation =
      BCSteadyStateLibrary::Radiation{BCSteadyStateLibrary::AutomaticEnclosure{25.0, 0.84}};
    legacy.data = comprehensive;
    expectSameAnswers(legacy);
}

//! The shape every library converted before the enclosure had a temperature of its own:
//! the record carries none and follows the air. Answering nothing here leaves the solver
//! with an enclosure that has no temperature at all.
TEST(TestUnifiedBCAccessors, AnEnclosureWithoutATemperatureFollowsTheAir)
{
    BoundaryCondition unified;
    unified.Name = "Interior Frame, converted before the field existed";
    SurfaceExchange exchange;
    exchange.convection = Convection{.model = ConvectionModel::Fixed_Convection_Coefficient,
                                     .airTemperature = Constant{21.0},
                                     .filmCoefficient = Constant{4.65}};
    exchange.radiation = AutomaticEnclosure{.emissivity = Constant{1.0}};
    unified.data = exchange;

    ASSERT_FALSE(std::get<AutomaticEnclosure>(exchange.radiation.value()).temperature.has_value());
    EXPECT_EQ(radiationTemperature(unified), std::optional<double>{21.0});

    // And it must agree with what the consumer sees today, through the conversion.
    const auto steady{toSteadyState(unified)};
    ASSERT_TRUE(steady.has_value()) << steady.error();
    EXPECT_EQ(radiationTemperature(unified), BCSteadyStateLibrary::radiationTemperature(steady.value()));
    EXPECT_EQ(emissivity(unified), BCSteadyStateLibrary::emissivity(steady.value()));
}

TEST(TestUnifiedBCAccessors, BlackBodyMatchesTheLegacyAnswers)
{
    auto legacy{legacyRecord("Exterior")};
    BCSteadyStateLibrary::Comprehensive comprehensive;
    comprehensive.relativeHumidity = 0.3;
    comprehensive.convection = BCSteadyStateLibrary::Convection{-18.0, 26.0};
    // Emissivity and view factor deliberately differ: the legacy view factor accessor
    // reports the emissivity, and the unified one has to reproduce that.
    comprehensive.radiation =
      BCSteadyStateLibrary::Radiation{BCSteadyStateLibrary::BlackBodyRadiation{-18.0, 0.84, 0.62}};
    legacy.data = comprehensive;
    expectSameAnswers(legacy);
}

TEST(TestUnifiedBCAccessors, LinearizedWithFluxMatchesTheLegacyAnswers)
{
    auto legacy{legacyRecord("Linearized")};
    BCSteadyStateLibrary::Comprehensive comprehensive;
    comprehensive.relativeHumidity = 0.5;
    comprehensive.convection = BCSteadyStateLibrary::Convection{21.0, 3.0};
    comprehensive.constantFlux = BCSteadyStateLibrary::ConstantFlux{12.5};
    comprehensive.radiation =
      BCSteadyStateLibrary::Radiation{BCSteadyStateLibrary::LinearizedRadiation{21.0, 4.4}};
    legacy.data = comprehensive;
    expectSameAnswers(legacy);
}

TEST(TestUnifiedBCAccessors, ManualEnclosureAndRadiationSurfaceMatchTheLegacyAnswers)
{
    auto manual{legacyRecord("Manual Enclosure")};
    BCSteadyStateLibrary::Comprehensive comprehensive;
    comprehensive.relativeHumidity = 0.5;
    comprehensive.convection = BCSteadyStateLibrary::Convection{21.0, 8.0};
    comprehensive.radiation = BCSteadyStateLibrary::Radiation{BCSteadyStateLibrary::ManualEnclosure{}};
    manual.data = comprehensive;
    expectSameAnswers(manual);

    auto surface{legacyRecord("Radiation Surface")};
    surface.data = BCSteadyStateLibrary::RadiationSurface{true, 21.0, 0.9};
    expectSameAnswers(surface);
}

TEST(TestUnifiedBCAccessors, AdiabaticMatchesTheLegacyAnswers)
{
    auto legacy{legacyRecord("Adiabatic")};
    legacy.data = BCSteadyStateLibrary::Simplified{0.0, 0.0, 0.5};
    expectSameAnswers(legacy);
}

//! A record whose input reads a channel has no steady value to report. It cannot reach a
//! steady consumer at all - toSteadyState refuses it - so the accessors need only decline
//! to invent a number.
TEST(TestUnifiedBCAccessors, ASeriesBackedInputHasNoSteadyValue)
{
    BoundaryCondition unified;
    unified.Name = "Reads a series";
    SurfaceExchange exchange;
    exchange.convection = Convection{.model = ConvectionModel::Fixed_Convection_Coefficient,
                                     .airTemperature = FromTimeSeries{TimeSeriesLibrary::ChannelRole::AirTemperature},
                                     .filmCoefficient = Constant{8.0}};
    unified.data = exchange;

    EXPECT_FALSE(airTemperature(unified).has_value());
    EXPECT_EQ(filmCoefficient(unified), std::optional<double>{8.0});
    EXPECT_FALSE(toSteadyState(unified).has_value());
}
