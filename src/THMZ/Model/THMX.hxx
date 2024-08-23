#pragma once

#include <optional>

#include "THMZ/Preferences/Preferences.hxx"
#include "THMZ/Properties/Properties.hxx"
#include "THMZ/GlazingSystem/GlazingSystem.hxx"
#include "THMZ/CMA/CMA.hxx"

namespace ThermFile
{
    // Model for radiance must have correctly tagged boundary conditions
    // This structure will hold the name of the boundary condition tags used in radiance mode
    struct RadianceModeTags
    {
        std::string shadeInName;
        std::string shadeInTag;
        std::string shadeOutName;
        std::string shadeOutTag;
    };

    struct ThermModel
    {
        std::string version{"1"};
        bool calculationReady{false};
        Preferences preferences;
        Properties properties;
        Point glazingOrigin;
        std::optional<RadianceModeTags> radianceTags;
        std::vector<Cavity> cavities;
        std::vector<Polygon> polygons;
        std::vector<Boundary> boundaryConditions;
        std::vector<GlazingSystem> glazingSystems;
        std::optional<CMAData> cmaData;
    };
}