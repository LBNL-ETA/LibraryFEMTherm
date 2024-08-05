#include "gtest/gtest.h"

#include "BCSteadyState/SteadyState.hxx"
#include "BCSteadyState/Utils.hxx"

using namespace BCSteadyStateLibrary;

class TestSteadyStateBoundaryConditionType : public ::testing::Test
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

TEST_F(TestSteadyStateBoundaryConditionType, TestAutomaticEnclosure)
{
    bc.data = Comprehensive{};
    auto & comp = std::get<Comprehensive>(bc.data);
    comp.radiation = Radiation{AutomaticEnclosure{}};
    EXPECT_TRUE(isGreyBodyRadiation(bc));
}

TEST_F(TestSteadyStateBoundaryConditionType, TestManualEnclosure)
{
    bc.data = Comprehensive{};
    auto & comp = std::get<Comprehensive>(bc.data);
    comp.radiation = Radiation{ManualEnclosure{}};
    EXPECT_TRUE(isGreyBodyRadiation(bc));
}

TEST_F(TestSteadyStateBoundaryConditionType, TestBlackBodyRadiation)
{
    bc.data = Comprehensive{};
    auto & comp = std::get<Comprehensive>(bc.data);
    comp.radiation = Radiation{BlackBodyRadiation{}};
    EXPECT_FALSE(isGreyBodyRadiation(bc));
}

TEST_F(TestSteadyStateBoundaryConditionType, TestNoRadiation)
{
    bc.data = Comprehensive{};
    EXPECT_FALSE(isGreyBodyRadiation(bc));
}

TEST_F(TestSteadyStateBoundaryConditionType, TestSimplified)
{
    bc.data = Simplified{};
    EXPECT_FALSE(isGreyBodyRadiation(bc));
}

TEST_F(TestSteadyStateBoundaryConditionType, TestRadiationSurface)
{
    bc.data = RadiationSurface{};
    EXPECT_TRUE(isGreyBodyRadiation(bc));
}

TEST_F(TestSteadyStateBoundaryConditionType, IsRadiationSurfaceWhenRadiationSurface) {
    bc.data = RadiationSurface{};
    EXPECT_TRUE(isRadiationSurface(bc));
}

TEST_F(TestSteadyStateBoundaryConditionType, NotRadiationSurfaceWhenComprehensive) {
    bc.data = Comprehensive{};
    EXPECT_FALSE(isRadiationSurface(bc));
}

TEST_F(TestSteadyStateBoundaryConditionType, NotRadiationSurfaceWhenSimplified) {
    bc.data = Simplified{};
    EXPECT_FALSE(isRadiationSurface(bc));
}

TEST_F(TestSteadyStateBoundaryConditionType, IsComprehensive) {
    bc.data = Comprehensive{};
    EXPECT_TRUE(isComprehensive(bc));
}

TEST_F(TestSteadyStateBoundaryConditionType, IsNotComprehensive) {
    bc.data = RadiationSurface{};
    EXPECT_FALSE(isComprehensive(bc));
}

TEST_F(TestSteadyStateBoundaryConditionType, TestAutoGreyBodyRadiation)
{
    // Test with AutomaticEnclosure
    bc.data = Comprehensive{};
    auto& comp = std::get<Comprehensive>(bc.data);
    comp.radiation = Radiation{AutomaticEnclosure{}};
    EXPECT_TRUE(isAutoGreyBodyRadiation(bc));

    // Test with ManualEnclosure
    comp.radiation = Radiation{ManualEnclosure{}};
    EXPECT_FALSE(isAutoGreyBodyRadiation(bc));

    // Test with BlackBodyRadiation
    comp.radiation = Radiation{BlackBodyRadiation{}};
    EXPECT_FALSE(isAutoGreyBodyRadiation(bc));

    // Test with no radiation
    comp.radiation = std::nullopt;
    EXPECT_FALSE(isAutoGreyBodyRadiation(bc));

    // Test with Simplified type
    bc.data = Simplified{};
    EXPECT_FALSE(isAutoGreyBodyRadiation(bc));

    // Test with RadiationSurface type
    bc.data = RadiationSurface{};
    EXPECT_FALSE(isAutoGreyBodyRadiation(bc));
}

TEST_F(TestSteadyStateBoundaryConditionType, TestIsBlackBodyRadiation)
{
    // Test 1: When BoundaryCondition has a Comprehensive type and radiation is BlackBodyRadiation
    bc.data = Comprehensive{};
    auto & comp = std::get<Comprehensive>(bc.data);
    comp.radiation = Radiation{BlackBodyRadiation{}};
    EXPECT_TRUE(isBlackBodyRadiation(bc));
    EXPECT_TRUE(isRadiationBC(bc));

    // Test 2: When BoundaryCondition has a Comprehensive type and radiation is AutomaticEnclosure
    bc.data = Comprehensive{};
    auto & comp1 = std::get<Comprehensive>(bc.data);
    comp1.radiation = Radiation{AutomaticEnclosure{}};
    EXPECT_FALSE(isBlackBodyRadiation(bc));
    EXPECT_TRUE(isRadiationBC(bc));

    // Test 3: When BoundaryCondition has a Comprehensive type and radiation is ManualEnclosure
    bc.data = Comprehensive{};
    auto & comp2 = std::get<Comprehensive>(bc.data);
    comp2.radiation = Radiation{ManualEnclosure{}};
    EXPECT_FALSE(isBlackBodyRadiation(bc));
    EXPECT_TRUE(isRadiationBC(bc));

    // Test 4: When BoundaryCondition has a Simplified type
    bc.data = Simplified{};
    EXPECT_FALSE(isBlackBodyRadiation(bc));
    EXPECT_FALSE(isRadiationBC(bc));

    // Test 5: When BoundaryCondition has a RadiationSurface type
    bc.data = RadiationSurface{};
    EXPECT_FALSE(isBlackBodyRadiation(bc));
    EXPECT_TRUE(isRadiationBC(bc));
}

TEST_F(TestSteadyStateBoundaryConditionType, TestIsLinearizedRadiation)
{
    // Test 1: When BoundaryCondition has a Comprehensive type and radiation is BlackBodyRadiation
    bc.data = Comprehensive{};
    auto & comp = std::get<Comprehensive>(bc.data);
    comp.radiation = Radiation{LinearizedRadiation{}};
    EXPECT_TRUE(isLinearizedRadiation(bc));

    // Test 2: When BoundaryCondition has a Comprehensive type and radiation is AutomaticEnclosure
    bc.data = Comprehensive{};
    auto & comp1 = std::get<Comprehensive>(bc.data);
    comp1.radiation = Radiation{AutomaticEnclosure{}};
    EXPECT_FALSE(isLinearizedRadiation(bc));

    // Test 3: When BoundaryCondition has a Comprehensive type and radiation is ManualEnclosure
    bc.data = Comprehensive{};
    auto & comp2 = std::get<Comprehensive>(bc.data);
    comp2.radiation = Radiation{ManualEnclosure{}};
    EXPECT_FALSE(isLinearizedRadiation(bc));

    // Test 4: When BoundaryCondition has a Simplified type
    bc.data = Simplified{};
    EXPECT_FALSE(isLinearizedRadiation(bc));

    // Test 5: When BoundaryCondition has a RadiationSurface type
    bc.data = RadiationSurface{};
    EXPECT_FALSE(isLinearizedRadiation(bc));
}

TEST_F(TestSteadyStateBoundaryConditionType, SimplifiedAdiabatic) {
    bc.data = Simplified{0.0, 0.0, 0.5};
    EXPECT_TRUE(isAdiabatic(bc));
    EXPECT_FALSE(isConvective(bc));
    EXPECT_FALSE(isRadiationBC(bc));
}

TEST_F(TestSteadyStateBoundaryConditionType, SimplifiedNonAdiabatic) {
    bc.data = Simplified{0.0, 1.0, 0.5};
    EXPECT_FALSE(isAdiabatic(bc));
    EXPECT_TRUE(isConvective(bc));
    EXPECT_FALSE(isRadiationBC(bc));
}

TEST_F(TestSteadyStateBoundaryConditionType, ComprehensiveAdiabatic) {
    bc.data = BCSteadyStateLibrary::Comprehensive{};
    auto &comp = std::get<BCSteadyStateLibrary::Comprehensive>(bc.data);
    comp.convection = Convection{0.0, 0.0};
    EXPECT_TRUE(isAdiabatic(bc));
}

TEST_F(TestSteadyStateBoundaryConditionType, ComprehensiveNonAdiabatic) {
    bc.data = BCSteadyStateLibrary::Comprehensive{};
    auto &comp = std::get<BCSteadyStateLibrary::Comprehensive>(bc.data);
    comp.convection = Convection{0.0, 1.0};
    EXPECT_FALSE(isAdiabatic(bc));
}

TEST_F(TestSteadyStateBoundaryConditionType, isConstantFlux)
{
    bc.data = BCSteadyStateLibrary::Comprehensive{};
    auto &comp = std::get<BCSteadyStateLibrary::Comprehensive>(bc.data);
    comp.constantFlux = ConstantFlux{12.0};
    EXPECT_TRUE(isConstantFlux(bc));
}

TEST_F(TestSteadyStateBoundaryConditionType, isConstantFluxZero)
{
    bc.data = BCSteadyStateLibrary::Comprehensive{};
    auto &comp = std::get<BCSteadyStateLibrary::Comprehensive>(bc.data);
    comp.constantFlux = ConstantFlux{0.0};
    EXPECT_FALSE(isConstantFlux(bc));
}

TEST_F(TestSteadyStateBoundaryConditionType, isNotConstantFlux) {
    bc.data = BCSteadyStateLibrary::Comprehensive{};
    auto &comp = std::get<BCSteadyStateLibrary::Comprehensive>(bc.data);
    comp.convection = Convection{0.0, 1.0};
    EXPECT_FALSE(isConstantFlux(bc));
}