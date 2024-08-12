#pragma once

#include <string>

#include "THMZ/Model/THMX.hxx"

namespace ThermFile
{
    ThermModel loadThermModelFromFile(std::string_view fileName);
    int saveToFile(const ThermModel & model, std::string_view fileName);
}