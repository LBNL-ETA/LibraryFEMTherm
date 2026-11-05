#pragma once

#include <string>
#include <vector>

#include "BoundaryConditions/BoundaryConditions.hxx"

//! BoundaryConditionsTransient.xml of earlier THERM 8 versions.
namespace BCLibrary::Import::TransientLibrary
{
    [[nodiscard]] bool recognizes(const std::string & text);
    [[nodiscard]] std::vector<BoundaryCondition> records(const std::string & text);
}   // namespace BCLibrary::Import::TransientLibrary
