#pragma once

#include "THMZ/Properties/Properties.hxx"

namespace ThermFile
{
    ////////////////////////////////////////////////////////////////////////////////////
    /// MeshLimits
    ////////////////////////////////////////////////////////////////////////////////////

    //! \brief Limits for mesh parameters. This is not to be serialized into file since it is depending on mesher type.
    struct Limits
    {
        Limits(size_t min, size_t max) : minimum(min), maximum(max)
        {}
        size_t minimum{0u};
        size_t maximum{0u};
    };

    Limits getMeshParameterLimits(MesherType type);
    Limits getIterationLimits();

    [[nodiscard]] ExposureType createExposureType(ThermFile::ModelPurpose model);
    [[nodiscard]] ModelExposure createModelExposure(ThermFile::ModelPurpose model);
}   // namespace Therm
