#include <gtest/gtest.h>

#include <variant>

#include "BoundaryConditionsSteadyStateTestUtils.hxx"

MismatchRadiationTypes::MismatchRadiationTypes() :
    std::runtime_error("Mismatched types in radiation.")
{}

MismatchBoundaryConditionType::MismatchBoundaryConditionType() :
    std::runtime_error("Mismatched types in boundary condition.")
{}

ValidateSteadyStateRadiationResults::ValidateSteadyStateRadiationResults(const double tolerance) : tolerance_(tolerance)
{}

void ValidateSteadyStateRadiationResults::operator()(const BCSteadyStateLibrary::AutomaticEnclosure & correct,
                                                     const BCSteadyStateLibrary::AutomaticEnclosure & calculated) const
{
    EXPECT_NEAR(calculated.temperature, correct.temperature, tolerance_);
    EXPECT_NEAR(calculated.emissivity, correct.emissivity, tolerance_);
}

void ValidateSteadyStateRadiationResults::operator()(const BCSteadyStateLibrary::ManualEnclosure &,
                                                     const BCSteadyStateLibrary::ManualEnclosure &) const
{
    // Manual radiation enclosure does not have any data and it servers only as indicator that it is used as radiation
    // model.
}

void ValidateSteadyStateRadiationResults::operator()(const BCSteadyStateLibrary::BlackBodyRadiation & correct,
                                                     const BCSteadyStateLibrary::BlackBodyRadiation & calculated) const
{
    EXPECT_NEAR(calculated.temperature, correct.temperature, tolerance_);
    EXPECT_NEAR(calculated.emissivity, correct.emissivity, tolerance_);
}

void ValidateSteadyStateRadiationResults::operator()(const BCSteadyStateLibrary::LinearizedRadiation & correct,
                                                     const BCSteadyStateLibrary::LinearizedRadiation & calculated) const
{
    EXPECT_NEAR(correct.temperature, calculated.temperature, tolerance_);
    EXPECT_NEAR(correct.filmCoefficient, calculated.filmCoefficient, tolerance_);
}

void validateSteadyStateConvection(const BCSteadyStateLibrary::Convection & correct,
                                   const BCSteadyStateLibrary::Convection & calculated,
                                   double tolerance)
{
    EXPECT_NEAR(correct.temperature, calculated.temperature, tolerance);
    EXPECT_NEAR(correct.filmCoefficient, calculated.filmCoefficient, tolerance);
}

void validateSteadyStateConstantFlux(const BCSteadyStateLibrary::ConstantFlux & correct,
                                     const BCSteadyStateLibrary::ConstantFlux & calculated,
                                     double tolerance)
{
    EXPECT_NEAR(correct.flux, calculated.flux, tolerance);
}

void validateSteadyStateRadiation(const BCSteadyStateLibrary::Radiation & correct,
                                  const BCSteadyStateLibrary::Radiation & calculated,
                                  double tolerance)
{
    EXPECT_EQ(correct.radiation.has_value(), calculated.radiation.has_value());
    if(correct.radiation.has_value() && calculated.radiation.has_value())
    {
        std::visit(ValidateSteadyStateRadiationResults{tolerance}, correct.radiation.value(), calculated.radiation.value());
    }
}

void validateSteadyStateBoundaryCondition(const BCSteadyStateLibrary::BoundaryCondition & correct,
                                          const BCSteadyStateLibrary::BoundaryCondition & calculated,
                                          double tolerance)
{
    EXPECT_EQ(correct.UUID, calculated.UUID);
    EXPECT_EQ(correct.Name, calculated.Name);
    EXPECT_EQ(correct.Protected, calculated.Protected);
    EXPECT_EQ(correct.Color, calculated.Color);

    std::visit(ValidateSteadyStateBoundaryConditionDataType{tolerance}, correct.data, calculated.data);
}

ValidateSteadyStateBoundaryConditionDataType::ValidateSteadyStateBoundaryConditionDataType(const double tolerance) :
    tolerance_(tolerance)
{}

void ValidateSteadyStateBoundaryConditionDataType::operator()(const BCSteadyStateLibrary::Comprehensive & correct,
                                                              const BCSteadyStateLibrary::Comprehensive & calculated) const
{
    EXPECT_NEAR(correct.relativeHumidity, calculated.relativeHumidity, tolerance_);

    EXPECT_EQ(correct.constantFlux.has_value(), calculated.constantFlux.has_value());
    if(correct.constantFlux.has_value() && calculated.constantFlux.has_value())
    {
        validateSteadyStateConstantFlux(correct.constantFlux.value(), calculated.constantFlux.value(), tolerance_);
    }

    EXPECT_EQ(correct.convection.has_value(), calculated.convection.has_value());
    if(correct.convection.has_value() && calculated.convection.has_value())
    {
        validateSteadyStateConvection(correct.convection.value(), calculated.convection.value(), tolerance_);
    }

    EXPECT_EQ(correct.radiation.has_value(), calculated.radiation.has_value());
    if(correct.radiation.has_value() && calculated.radiation.has_value())
    {
        validateSteadyStateRadiation(correct.radiation.value(), calculated.radiation.value(), tolerance_);
    }
}

void ValidateSteadyStateBoundaryConditionDataType::operator()(const BCSteadyStateLibrary::Simplified & correct,
                                                              const BCSteadyStateLibrary::Simplified & calculated) const
{
    EXPECT_NEAR(correct.temperature, calculated.temperature, tolerance_);
    EXPECT_NEAR(correct.filmCoefficient, calculated.filmCoefficient, tolerance_);
    EXPECT_NEAR(correct.relativeHumidity, calculated.relativeHumidity, tolerance_);
}

void ValidateSteadyStateBoundaryConditionDataType::operator()(const BCSteadyStateLibrary::RadiationSurface & correct,
                                                              const BCSteadyStateLibrary::RadiationSurface & calculated) const
{
    EXPECT_NEAR(correct.temperature, calculated.temperature, tolerance_);
    EXPECT_NEAR(correct.emissivity, calculated.emissivity, tolerance_);
}
