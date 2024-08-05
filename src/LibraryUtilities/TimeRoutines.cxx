#include <ctime>
#include <iomanip>
#include <sstream>

#include "TimeRoutines.hxx"

namespace Timer
{
    std::string timeToString(const std::chrono::system_clock::time_point & time)
    {
        auto to_tm = [](const std::chrono::system_clock::time_point & time) -> std::tm {
          std::time_t tt = std::chrono::system_clock::to_time_t(time);
          std::tm tm;
#ifdef _WIN32
            localtime_s(&tm, &tt);
#else
            localtime_r(&tt, &tm);
#endif
          return tm;
        };

        std::tm tm = to_tm(time);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
}