#pragma once

#include <string>
#include <string_view>
#include <chrono>

namespace Common
{
    void createDirectoryIfNotExists(std::string_view dirName);

    std::chrono::system_clock::time_point
      createTimePoint(int year, int month, int day, int hour, int minute, int second);
}   // namespace Common