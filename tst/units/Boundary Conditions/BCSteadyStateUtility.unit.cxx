#include "gtest/gtest.h"

#include "BCSteadyState/SteadyState.hxx"
#include "BCSteadyState/Utils.hxx"

using namespace BCSteadyStateLibrary;

class TestBCSteadyStateUtilityRoutines : public ::testing::Test
{
protected:
    BoundaryCondition bc;

    void SetUp() override
    {
        // This method will be called before each test
    }

    void TearDown() override
    {
        // This method will be called after each test
    }
};

TEST_F(TestBCSteadyStateUtilityRoutines, TestSimplifiedRelativeHumidity)
{
    bc.data = Simplified{};
    auto & simplified = std::get<Simplified>(bc.data);
    simplified.relativeHumidity = 0.8;
    EXPECT_DOUBLE_EQ(*getRelativeHumidity(bc), 0.8);
}

TEST_F(TestBCSteadyStateUtilityRoutines, TestComprehensiveRelativeHumidity)
{
    bc.data = Comprehensive{};
    auto & comprehensive = std::get<Comprehensive>(bc.data);
    comprehensive.relativeHumidity = 0.3;
    EXPECT_DOUBLE_EQ(*getRelativeHumidity(bc), 0.3);
}

TEST_F(TestBCSteadyStateUtilityRoutines, TestRadiationSurfaceRelativeHumidity)
{
    bc.data = RadiationSurface{};
    EXPECT_EQ(getRelativeHumidity(bc), std::nullopt);
}

TEST_F(TestBCSteadyStateUtilityRoutines, TestComprehensiveConvectionAirTemperature)
{
    bc.data = BCSteadyStateLibrary::Comprehensive{};
    auto & comp = std::get<BCSteadyStateLibrary::Comprehensive>(bc.data);
    comp.convection = BCSteadyStateLibrary::Convection{35.0, 1.0};
    auto temp = airTemperature(bc);
    ASSERT_TRUE(temp.has_value());
    EXPECT_DOUBLE_EQ(temp.value(), 35.0);
}

TEST_F(TestBCSteadyStateUtilityRoutines, TestComprehensiveNoConvectionAirTemperature)
{
    bc.data = BCSteadyStateLibrary::Comprehensive{};
    auto temp = airTemperature(bc);
    ASSERT_FALSE(temp.has_value());
}

TEST_F(TestBCSteadyStateUtilityRoutines, TestSimplifiedAirTemperature)
{
    bc.data = BCSteadyStateLibrary::Simplified{40.0, 1.0, 0.5};
    auto temp = airTemperature(bc);
    ASSERT_TRUE(temp.has_value());
    EXPECT_DOUBLE_EQ(temp.value(), 40.0);
}

TEST_F(TestBCSteadyStateUtilityRoutines, TestRadiationSurfaceAirTemperature)
{
    bc.data = RadiationSurface{.temperature=45.0, .emissivity=0.9};
    auto temp = airTemperature(bc);
    ASSERT_FALSE(temp.has_value());
}

TEST_F(TestBCSteadyStateUtilityRoutines, TestComprehensiveConvectionFilmCoefficient)
{
    bc.data = BCSteadyStateLibrary::Comprehensive{};
    auto & comp = std::get<BCSteadyStateLibrary::Comprehensive>(bc.data);
    comp.convection = BCSteadyStateLibrary::Convection{35.0, 1.0};
    auto filmCoefficient = BCSteadyStateLibrary::filmCoefficient(bc);
    EXPECT_TRUE(filmCoefficient.has_value());
    EXPECT_DOUBLE_EQ(filmCoefficient.value(), 1.0);
}

TEST_F(TestBCSteadyStateUtilityRoutines, TestComprehensiveNoConvectionFilmCoefficient)
{
    bc.data = BCSteadyStateLibrary::Comprehensive{};
    auto filmCoeff = filmCoefficient(bc);
    EXPECT_FALSE(filmCoeff.has_value());
}

TEST_F(TestBCSteadyStateUtilityRoutines, TestSimplifiedFilmCoefficient)
{
    bc.data = BCSteadyStateLibrary::Simplified{40.0, 2.3, 0.5};
    auto filmCoeff = filmCoefficient(bc);
    EXPECT_TRUE(filmCoeff.has_value());
    EXPECT_DOUBLE_EQ(filmCoeff.value(), 2.3);
}

TEST_F(TestBCSteadyStateUtilityRoutines, TestComprehensiveConstantHeatFlux)
{
    bc.data = BCSteadyStateLibrary::Comprehensive{};
    auto & comp = std::get<BCSteadyStateLibrary::Comprehensive>(bc.data);
    comp.constantFlux = BCSteadyStateLibrary::ConstantFlux{13.0};
    auto flux = constantHeatFlux(bc);
    EXPECT_TRUE(flux.has_value());
    EXPECT_DOUBLE_EQ(flux.value(), 13.0);
}

TEST_F(TestBCSteadyStateUtilityRoutines, TestComprehensiveNoConstantHeatFlux)
{
    bc.data = BCSteadyStateLibrary::Comprehensive{};
    auto flux = constantHeatFlux(bc);
    EXPECT_FALSE(flux.has_value());
}

TEST_F(TestBCSteadyStateUtilityRoutines, TestGetBoundaryConditionTypeComprehensive)
{
    bc.data = BCSteadyStateLibrary::Comprehensive{};
    EXPECT_EQ(getBoundaryConditionType(bc), BoundaryConditionType::Comprehensive);
}

TEST_F(TestBCSteadyStateUtilityRoutines, TestGetBoundaryConditionTypeSimplified)
{
    bc.data = BCSteadyStateLibrary::Simplified{};
    EXPECT_EQ(getBoundaryConditionType(bc), BoundaryConditionType::Simplified);
}

TEST_F(TestBCSteadyStateUtilityRoutines, TestGetBoundaryConditionTypeRadiationSurface)
{
    bc.data = BCSteadyStateLibrary::RadiationSurface{};
    EXPECT_EQ(getBoundaryConditionType(bc), BoundaryConditionType::RadiationSurface);
}

TEST_F(TestBCSteadyStateUtilityRoutines, TestLinearizedRadiationNoFilmCoefficient)
{
    bc.data = BCSteadyStateLibrary::Comprehensive{};
    auto filmCoeff = radiationFilmCoefficient(bc);
    EXPECT_FALSE(filmCoeff.has_value());
}

TEST_F(TestBCSteadyStateUtilityRoutines, TestLinearizedRadiationFilmCoefficient)
{
    BCSteadyStateLibrary::Comprehensive comp;
    BCSteadyStateLibrary::Radiation radiation;
    BCSteadyStateLibrary::LinearizedRadiation linearizedRadiation{291.15, 3.41};
    radiation.radiation = linearizedRadiation;
    comp.radiation = radiation;
    bc.data = comp;

    auto filmCoeff = radiationFilmCoefficient(bc);
    EXPECT_TRUE(filmCoeff.has_value());
    EXPECT_DOUBLE_EQ(filmCoeff.value(), 3.41);
}

TEST_F(TestBCSteadyStateUtilityRoutines, RadiationSurfaceTemperature)
{
    BCSteadyStateLibrary::RadiationSurface surface{.temperature=291.15, .emissivity=0.5};
    bc.data = surface;

    auto temperature = radiationTemperature(bc);
    EXPECT_TRUE(temperature.has_value());
    EXPECT_DOUBLE_EQ(temperature.value(), 291.15);
}

TEST_F(TestBCSteadyStateUtilityRoutines, LinearizedRadiationTemperature)
{
    BCSteadyStateLibrary::Comprehensive comp;
    BCSteadyStateLibrary::Radiation radiation;
    BCSteadyStateLibrary::LinearizedRadiation linearizedRadiation{291.15, 3.41};
    radiation.radiation = linearizedRadiation;
    comp.radiation = radiation;
    bc.data = comp;

    auto temperature = radiationTemperature(bc);
    EXPECT_TRUE(temperature.has_value());
    EXPECT_DOUBLE_EQ(temperature.value(), 291.15);
}

TEST_F(TestBCSteadyStateUtilityRoutines, BlackBodyRadiationTemperature)
{
    BCSteadyStateLibrary::Comprehensive comp;
    BCSteadyStateLibrary::Radiation radiation;
    BCSteadyStateLibrary::BlackBodyRadiation blackBodyRadiation{291.15, 0.75};
    radiation.radiation = blackBodyRadiation;
    comp.radiation = radiation;
    bc.data = comp;

    auto temperature = radiationTemperature(bc);
    EXPECT_TRUE(temperature.has_value());
    EXPECT_DOUBLE_EQ(temperature.value(), 291.15);
}

TEST_F(TestBCSteadyStateUtilityRoutines, AutoEnclosureTemperature)
{
    BCSteadyStateLibrary::Comprehensive comp;
    BCSteadyStateLibrary::Radiation radiation;
    BCSteadyStateLibrary::AutomaticEnclosure autoEnclosure{291.15, 0.75};
    radiation.radiation = autoEnclosure;
    comp.radiation = radiation;
    bc.data = comp;

    auto temperature = radiationTemperature(bc);
    EXPECT_TRUE(temperature.has_value());
    EXPECT_DOUBLE_EQ(temperature.value(), 291.15);
}

TEST_F(TestBCSteadyStateUtilityRoutines, BlackBodyViewFactor)
{
    BCSteadyStateLibrary::Comprehensive comp;
    BCSteadyStateLibrary::Radiation radiation;
    BCSteadyStateLibrary::BlackBodyRadiation blackBodyRadiation{291.15, 0.75};
    radiation.radiation = blackBodyRadiation;
    comp.radiation = radiation;
    bc.data = comp;

    auto temperature = viewFactor(bc);
    EXPECT_TRUE(temperature.has_value());
    EXPECT_DOUBLE_EQ(temperature.value(), 0.75);
}

TEST_F(TestBCSteadyStateUtilityRoutines, AutoEnclosureEmissivity)
{
    BCSteadyStateLibrary::Comprehensive comp;
    BCSteadyStateLibrary::Radiation radiation;
    BCSteadyStateLibrary::AutomaticEnclosure autoEnclosure{291.15, 0.75};
    radiation.radiation = autoEnclosure;
    comp.radiation = radiation;
    bc.data = comp;

    auto temperature = emissivity(bc);
    EXPECT_TRUE(temperature.has_value());
    EXPECT_DOUBLE_EQ(temperature.value(), 0.75);
}

TEST_F(TestBCSteadyStateUtilityRoutines, RadiationSurfaceEmissivity)
{
    BCSteadyStateLibrary::RadiationSurface surface{.temperature = 291.15, .emissivity=0.5};
    bc.data = surface;

    auto temperature = emissivity(bc);
    EXPECT_TRUE(temperature.has_value());
    EXPECT_DOUBLE_EQ(temperature.value(), 0.5);
}