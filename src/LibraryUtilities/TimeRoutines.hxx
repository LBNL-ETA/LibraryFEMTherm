#pragma once

#include <chrono>

#include <fileParse/Base.hxx>
#include "StringRoutines.hxx"

namespace Timer
{
    std::string timeToString(const std::chrono::system_clock::time_point & time);
}   // namespace FileParse