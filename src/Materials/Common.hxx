#pragma once

#include <string>
#include <optional>
#include <vector>

#include "Materials.hxx"

namespace MaterialsLibrary
{
    void saveToCSV(const std::string & fileName,
                   const std::optional<std::vector<point>> & vector);

    std::optional<std::vector<point>> loadFromCSV(const std::string & fileName);
}   // namespace MaterialsLibrary
