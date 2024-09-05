#pragma once

#include <string>
#include <optional>

#include "THMZ/Model/THMX.hxx"

namespace ThermFile
{
    std::optional<ThermModel> loadThermModelFromFile(std::string_view fileName);
    int saveToFile(const ThermModel & model, std::string_view fileName);

    std::optional<ThermModel> loadThermModelFromZipFile(const std::string & zipFileName);
    int saveToZipFile(const ThermModel & model, std::string_view zipFileName);

    std::optional<ThermModel> loadThermModelFromString(const std::string & xmlString);
    std::string saveToString(const ThermModel & model);
}   // namespace ThermFile