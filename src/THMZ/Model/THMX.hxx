#pragma once

#include "THMZ/Preferences/Preferences.hxx"
#include "THMZ/Properties/Properties.hxx"
#include "THMZ/GlazingSystem/GlazingSystem.hxx"
#include "THMZ/CMA/CMA.hxx"

namespace ThermFile
{
    struct ThermModel
    {
        std::string version{"1"};
        bool calculationReady{false};
        Preferences preferences;
        Properties properties;
        Point glazingOrigin;
        std::vector<Cavity> cavities;
        std::vector<Polygon> polygons;
        std::vector<Boundary> boundaryConditions;
        std::vector<GlazingSystem> glazingSystems;
        std::optional<CMAData> cmaData;
    };
}