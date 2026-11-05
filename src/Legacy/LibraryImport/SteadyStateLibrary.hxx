#pragma once

#include <string>
#include <vector>

#include "BoundaryConditions/BoundaryConditions.hxx"

//! BoundaryConditionsSteadyState.xml of earlier THERM 8 versions. It shares its top element with
//! the current library; its record kinds tell them apart.
namespace BCLibrary::Import::SteadyStateLibrary
{
    [[nodiscard]] bool recognizes(const std::string & text);
    [[nodiscard]] std::vector<BoundaryCondition> records(const std::string & text);
}   // namespace BCLibrary::Import::SteadyStateLibrary
