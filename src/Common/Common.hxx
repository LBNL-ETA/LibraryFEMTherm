#pragma once

#include <string>
#include <chrono>

namespace Common
{
    std::string generateXmlContent(const std::string & rootElement,
                                   const std::string & schemaLocation,
                                   const std::string & version);

    void createDirectoryIfNotExists(std::string_view dirName);

    std::chrono::system_clock::time_point
      createTimePoint(int year, int month, int day, int hour, int minute, int second);
}   // namespace Common