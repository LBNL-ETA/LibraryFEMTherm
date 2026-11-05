#pragma once

#include <string>
#include <vector>

#include "BoundaryConditions/BoundaryConditions.hxx"

//! A BoundaryConditions.xml of the current format, which needs no conversion.
namespace BCLibrary::Import::CurrentLibrary
{
    [[nodiscard]] bool recognizes(const std::string & text);
    [[nodiscard]] std::vector<BoundaryCondition> records(const std::string & text);
}   // namespace BCLibrary::Import::CurrentLibrary
