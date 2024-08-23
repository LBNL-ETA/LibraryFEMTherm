#pragma once

#include "CMA.hxx"

namespace CMALibrary
{
    CMAData loadCMADataFromFile(const std::string & fileName);
    CMAData loadCMADataFromZipFile(const std::string & fileName);
}