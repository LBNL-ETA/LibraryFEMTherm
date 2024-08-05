#pragma once

#include <string>
#include <optional>
#include <vector>

namespace MaterialsLibrary
{
    struct point;

    void saveToCSV(const std::string & fileName,
                   const std::optional<std::vector<point>> & vector);

    std::optional<std::vector<point>> loadFromCSV(const std::string & fileName);
}   // namespace MaterialsLibrary
