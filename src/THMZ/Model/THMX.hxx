#pragma once

#include <optional>
#include <set>
#include <string>

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

    //! The Model.xml version this library writes. Loading through the migration pipeline stamps
    //! it on the model, so a saved file records which migrations it has been through.
    inline constexpr int currentModelVersion{6};

    struct ThermModel
    {
        std::string version{std::to_string(currentModelVersion)};
        bool calculationReady{false};
        Preferences preferences;
        Properties properties;
        Point glazingOrigin;
        std::optional<RadianceModeTags> radianceTags;
        std::vector<Polygon> polygons;
        std::vector<Boundary> boundaryConditions;
        std::vector<GlazingSystem> glazingSystems;
        std::optional<CMAData> cmaData;
    };
}