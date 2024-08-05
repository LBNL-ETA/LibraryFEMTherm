#pragma once

#include <string>

namespace ThermFile
{
    enum class UValueDimensionType
    {
        Unknown,
        TotalLength,
        XProjected,
        YProjected,
        Custom,
        GlassRotationProjected,
        CustomRotationProjected
    };

    UValueDimensionType uValueDimensionTypeFromString(std::string_view uValueDimensionType);

    std::string uValueDimensionTypeToString(UValueDimensionType uValueDimensionType);
}   // namespace ThermFile