#pragma once

#include <string>

#include "THMZ/Model/THMX.hxx"

namespace ThermFile
{
    ThermModel loadThermModelFromFile(std::string_view fileName);
    int saveToFile(const ThermModel & model, std::string_view fileName);

    ThermModel loadThermModelFromZipFile(const std::string & zipFileName);
    int saveToZipFile(const ThermModel & model, std::string_view zipFileName);

    std::string saveToString(const ThermModel & model);
    ThermModel loadThermModelFromString(const std::string & xmlString);
}   // namespace ThermFile