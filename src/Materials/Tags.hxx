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
    [[nodiscard]] std::string cavityString();
    [[nodiscard]] std::string enclosureString();

    [[nodiscard]] std::string materialRoughnessToString(MaterialRoughness materialRoughness);
    [[nodiscard]] MaterialRoughness materialRoughnessFromString(std::string_view value);

    [[nodiscard]] std::string cavityStandardToString(CavityStandard cavStandard);
    [[nodiscard]] CavityStandard cavityStandardFromString(std::string_view cavityStandardTag);

    [[nodiscard]] std::string frontSideString();
    [[nodiscard]] std::string backSideString();

    [[nodiscard]] std::string directString();
    [[nodiscard]] std::string diffuseString();

    [[nodiscard]] std::string solarString();
    [[nodiscard]] std::string visibleString();

    [[nodiscard]] std::string cavityStandardToDisplayString(CavityStandard cavityStandard);
    [[nodiscard]] CavityStandard cavityStandardFromDisplayString(std::string_view cavity);
}   // namespace MaterialsLibrary
