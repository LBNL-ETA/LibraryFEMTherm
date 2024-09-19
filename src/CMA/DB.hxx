#pragma once

#include <optional>

#include "CMA.hxx"

namespace CMALibrary
{
    std::optional<CMAData> loadCMADataFromFile(const std::string & fileName);
    std::optional<CMAData> loadCMADataFromZipFile(const std::string & fileName);
}