#include "gtest/gtest.h"

#include "BCSteadyState/SteadyState.hxx"
#include "BCSteadyState/Operators.hxx"

class TestBCSteadyStateOperators : public testing::Test
{
protected:

    void SetUp() override
    {}
};


// Test Convection Equality
TEST(TestBCSteadyStateOperators, ConvectionEquality) {
    BCSteadyStateLibrary::Convection convection1(25.0, 0.8);
    BCSteadyStateLibrary::Convection convection2(25.0, 0.8);
    EXPECT_TRUE(convection1 == convection2);
}

// Test Convection Inequality
TEST(TestBCSteadyStateOperators, ConvectionInequality) {
    BCSteadyStateLibrary::Convection convection1(25.0, 0.8);
    BCSteadyStateLibrary::Convection convection2(26.0, 0.7);
    EXPECT_TRUE(convection1 != convection2);
}

// Test Radiation Equality
TEST(TestBCSteadyStateOperators, RadiationEquality) {
    BCSteadyStateLibrary::Radiation radiation1;
    BCSteadyStateLibrary::Radiation radiation2;
    radiation1.radiation = BCSteadyStateLibrary::AutomaticEnclosure(100.0, 0.9);
    radiation2.radiation = BCSteadyStateLibrary::AutomaticEnclosure(100.0, 0.9);
    EXPECT_TRUE(radiation1 == radiation2);
}

// Test Radiation Inequality
TEST(TestBCSteadyStateOperators, RadiationInequality) {
    BCSteadyStateLibrary::Radiation radiation1;
    BCSteadyStateLibrary::Radiation radiation2;
    radiation1.radiation = BCSteadyStateLibrary::AutomaticEnclosure(100.0, 0.9);
    radiation2.radiation = BCSteadyStateLibrary::BlackBodyRadiation(100.0, 0.8);
    EXPECT_TRUE(radiation1 != radiation2);
}

// Test Comprehensive Equality
TEST(TestBCSteadyStateOperators, ComprehensiveEquality) {
    BCSteadyStateLibrary::Comprehensive comp1;
    BCSteadyStateLibrary::Comprehensive comp2;
    comp1.relativeHumidity = 0.7;
    comp2.relativeHumidity = 0.7;
    comp1.convection = BCSteadyStateLibrary::Convection(25.0, 0.8);
    comp2.convection = BCSteadyStateLibrary::Convection(25.0, 0.8);
    EXPECT_TRUE(comp1 == comp2);
}

// Test Comprehensive Inequality
TEST(TestBCSteadyStateOperators, ComprehensiveInequality) {
    BCSteadyStateLibrary::Comprehensive comp1;
    BCSteadyStateLibrary::Comprehensive comp2;
    comp1.relativeHumidity = 0.7;
    comp2.relativeHumidity = 0.7;
    comp1.convection = BCSteadyStateLibrary::Convection(25.0, 0.8);
    comp2.convection = BCSteadyStateLibrary::Convection(25.0, 0.7);
    EXPECT_TRUE(comp1 != comp2);
}

// Test Simplified Equality
TEST(TestBCSteadyStateOperators, SimplifiedEquality) {
    BCSteadyStateLibrary::Simplified simp1(25.0, 0.8, 0.5);
    BCSteadyStateLibrary::Simplified simp2(25.0, 0.8, 0.5);
    EXPECT_TRUE(simp1 == simp2);
}

// Test Simplified Inequality
TEST(TestBCSteadyStateOperators, SimplifiedInequality) {
    BCSteadyStateLibrary::Simplified simp1(25.0, 0.8, 0.5);
    BCSteadyStateLibrary::Simplified simp2(26.0, 0.7, 0.6);
    EXPECT_TRUE(simp1 != simp2);
}

// Test RadiationSurface Equality
TEST(TestBCSteadyStateOperators, RadiationSurfaceEquality) {
    BCSteadyStateLibrary::RadiationSurface surface1(false, 100.0, 0.9);
    BCSteadyStateLibrary::RadiationSurface surface2(false, 100.0, 0.9);
    EXPECT_TRUE(surface1 == surface2);
}

// Test RadiationSurface Inequality
TEST(TestBCSteadyStateOperators, RadiationSurfaceInequality) {
    BCSteadyStateLibrary::RadiationSurface surface3(true, 100.0, 0.9);
    BCSteadyStateLibrary::RadiationSurface surface4(false, 101.0, 0.8);
    EXPECT_TRUE(surface3 != surface4);
}

// Test BoundaryCondition Equality
TEST(TestBCSteadyStateOperators, BoundaryConditionEquality) {
    BCSteadyStateLibrary::BoundaryCondition bc1;
    BCSteadyStateLibrary::BoundaryCondition bc2;
    bc1.UUID = "12345";
    bc1.Name = "Test";
    bc2.UUID = "12345";
    bc2.Name = "Test";
    bc1.data = BCSteadyStateLibrary::Simplified(25.0, 0.8, 0.5);
    bc2.data = BCSteadyStateLibrary::Simplified(25.0, 0.8, 0.5);
    EXPECT_TRUE(bc1 == bc2);
}

// Test BoundaryCondition Inequality
TEST(TestBCSteadyStateOperators, BoundaryConditionInequality) {
    BCSteadyStateLibrary::BoundaryCondition bc1;
    BCSteadyStateLibrary::BoundaryCondition bc2;
    bc1.UUID = "12345";
    bc1.Name = "Test";
    bc2.UUID = "12345";
    bc2.Name = "Test2";
    bc1.data = BCSteadyStateLibrary::Simplified(25.0, 0.8, 0.5);
    bc2.data = BCSteadyStateLibrary::Simplified(25.0, 0.8, 0.5);
    EXPECT_TRUE(bc1 != bc2);
}

// Test PhysicalProperties Equality
TEST(TestBCSteadyStateOperators, PhysicalPropertiesEquality) {
    BCSteadyStateLibrary::BoundaryCondition bc1;
    BCSteadyStateLibrary::BoundaryCondition bc2;
    bc1.data = BCSteadyStateLibrary::Simplified(25.0, 0.8, 0.5);
    bc2.data = BCSteadyStateLibrary::Simplified(25.0, 0.8, 0.5);
    EXPECT_TRUE(BCSteadyStateLibrary::arePhysicalPropertiesEqual(bc1, bc2));
}

// Test PhysicalProperties Inequality
TEST(TestBCSteadyStateOperators, PhysicalPropertiesInequality) {
    BCSteadyStateLibrary::BoundaryCondition bc1;
    BCSteadyStateLibrary::BoundaryCondition bc2;
    bc1.data = BCSteadyStateLibrary::Simplified(25.0, 0.8, 0.5);
    bc2.data = BCSteadyStateLibrary::Simplified(26.0, 0.7, 0.6);
    EXPECT_FALSE(BCSteadyStateLibrary::arePhysicalPropertiesEqual(bc1, bc2));
}
