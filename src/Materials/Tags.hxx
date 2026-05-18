#pragma once

#include <map>
#include <optional>
#include <string>

#include "Definitions.hxx"

namespace MaterialsLibrary
{
    [[nodiscard]] std::string materialsString();
    [[nodiscard]] std::string materialString();

    [[nodiscard]] std::string solidString();
    [[nodiscard]] std::string opticalString();

    [[nodiscard]] std::string materialRoughnessToString(MaterialRoughness materialRoughness);
    [[nodiscard]] MaterialRoughness materialRoughnessFromString(std::string_view value);

    [[nodiscard]] std::string frontSideString();
    [[nodiscard]] std::string backSideString();

    [[nodiscard]] std::string directString();
    [[nodiscard]] std::string diffuseString();

    [[nodiscard]] std::string solarString();
    [[nodiscard]] std::string visibleString();
}   // namespace MaterialsLibrary
