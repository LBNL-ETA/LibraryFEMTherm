#pragma once

#include <stdexcept>
#include <BCSteadyState/SteadyState.hxx>

class MismatchRadiationTypes : public std::runtime_error
{
public:
    MismatchRadiationTypes();
};

class MismatchBoundaryConditionType : public std::runtime_error
{
public:
    MismatchBoundaryConditionType();
};

struct ValidateSteadyStateRadiationResults
{
    explicit ValidateSteadyStateRadiationResults(double tolerance);
    void operator()(const BCSteadyStateLibrary::AutomaticEnclosure & correct,
                    const BCSteadyStateLibrary::AutomaticEnclosure & calculated) const;
    void operator()(const BCSteadyStateLibrary::ManualEnclosure & correct,
                    const BCSteadyStateLibrary::ManualEnclosure & calculated) const;
    void operator()(const BCSteadyStateLibrary::BlackBodyRadiation & correct,
                    const BCSteadyStateLibrary::BlackBodyRadiation & calculated) const;
    void operator()(const BCSteadyStateLibrary::LinearizedRadiation & correct,
                    const BCSteadyStateLibrary::LinearizedRadiation & calculated) const;

    // Catch-all for any non-matching types. Using std::visit to compare two variants will require all possible
    // combinations
    template<typename T, typename U>
    void operator()(const T &, const U &) const
    {
        throw MismatchRadiationTypes();
    }

private:
    double tolerance_;
};

struct ValidateSteadyStateBoundaryConditionDataType
{
    explicit ValidateSteadyStateBoundaryConditionDataType(double tolerance);
    void operator()(const BCSteadyStateLibrary::Comprehensive & correct,
                    const BCSteadyStateLibrary::Comprehensive & calculated) const;
    void operator()(const BCSteadyStateLibrary::Simplified & correct, const BCSteadyStateLibrary::Simplified & calculated) const;
    void operator()(const BCSteadyStateLibrary::RadiationSurface & correct,
                    const BCSteadyStateLibrary::RadiationSurface & calculated) const;

    // Catch-all for any non-matching types. Using std::visit to compare two variants will require all possible
    // combinations
    template<typename T, typename U>
    void operator()(const T &, const U &) const
    {
        throw MismatchBoundaryConditionType();
    }

private:
    double tolerance_;
};

void validateSteadyStateConvection(const BCSteadyStateLibrary::Convection & correct,
                                   const BCSteadyStateLibrary::Convection & calculated,
                                   double tolerance);

void validateSteadyStateConstantFlux(const BCSteadyStateLibrary::ConstantFlux & correct,
                                     const BCSteadyStateLibrary::ConstantFlux & calculated,
                                     double tolerance);

void validateSteadyStateRadiation(const BCSteadyStateLibrary::Radiation & correct,
                                  const BCSteadyStateLibrary::Radiation & calculated,
                                  double tolerance);

void validateSteadyStateBoundaryCondition(const BCSteadyStateLibrary::BoundaryCondition & correct,
                                          const BCSteadyStateLibrary::BoundaryCondition & calculated,
                                          double tolerance);