#pragma once

namespace Geometry
{
    struct GravityVector
    {
        GravityVector() = default;
        GravityVector(double x, double y, double z);
        double x{0};
        double y{0};
        double z{0};
    };
}   // namespace Geometry
