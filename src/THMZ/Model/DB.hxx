#pragma once

#include <string>

namespace ThermFile
{
    struct ThermModel;

    ThermModel loadThermModelFromFile(std::string_view fileName);
    int saveToFile(const ThermModel & model, std::string_view fileName);
}