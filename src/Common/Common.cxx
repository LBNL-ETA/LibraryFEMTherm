#include <filesystem>
#include <string_view>

#include "Common.hxx"

namespace Common
{

    std::string generateXmlContent(const std::string & rootElement,
                                   const std::string & schemaLocation,
                                   const std::string & version)
    {
        return "<" + rootElement + " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
               + "        xsi:noNamespaceSchemaLocation=\"" + schemaLocation + "\">\n" + "    <Version>" + version
               + "</Version>\n" + "</" + rootElement + ">";
    }

    void createDirectoryIfNotExists(std::string_view dirName)
    {
        std::filesystem::path path(dirName);
        if(!std::filesystem::exists(path))
        {
            std::filesystem::create_directories(path);
        }
    }

    std::chrono::system_clock::time_point
      createTimePoint(int year, int month, int day, int hour, int minute, int second)
    {
        std::tm t = {};
        t.tm_year = year - 1900;
        t.tm_mon = month - 1;
        t.tm_mday = day;
        t.tm_hour = hour;
        t.tm_min = minute;
        t.tm_sec = second;
        t.tm_isdst = -1;
        std::time_t time = std::mktime(&t);
        return std::chrono::system_clock::from_time_t(time);
    }
}   // namespace Common