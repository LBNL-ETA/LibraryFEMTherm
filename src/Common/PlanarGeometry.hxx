#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <limits>
#include <ranges>

//! Plane geometry on anything that has an x and a y: the file's points, the model's points,
//! a mesher's nodes. The functions do not care which; they read the two coordinates and
//! nothing else, so every caller keeps its own point type.
namespace Geometry
{
    template<typename PointType>
    concept PlanarPoint = requires(const PointType & point) {
        { point.x } -> std::convertible_to<double>;
        { point.y } -> std::convertible_to<double>;
    };

    template<typename RangeType>
    concept PointRange = std::ranges::forward_range<RangeType> && PlanarPoint<std::ranges::range_value_t<RangeType>>;

    //! A closed polygon given by its corners in order; the last corner connects back to the first.
    template<typename RangeType>
    concept Polygon = std::ranges::random_access_range<RangeType> && std::ranges::sized_range<RangeType>
                      && PlanarPoint<std::ranges::range_value_t<RangeType>>;

    //! Axis-aligned bounds. A default-constructed box is empty: it holds no point, and the
    //! first point extended into it becomes the whole box.
    struct BoundingBox
    {
        // Parenthesized, like the (std::min)/(std::max) calls below: the header is included
        // after <windows.h>, whose min/max macros would otherwise expand here.
        double minX{(std::numeric_limits<double>::max)()};
        double minY{(std::numeric_limits<double>::max)()};
        double maxX{std::numeric_limits<double>::lowest()};
        double maxY{std::numeric_limits<double>::lowest()};

        [[nodiscard]] bool empty() const
        {
            return minX > maxX || minY > maxY;
        }

        [[nodiscard]] double width() const
        {
            return empty() ? 0.0 : maxX - minX;
        }

        [[nodiscard]] double height() const
        {
            return empty() ? 0.0 : maxY - minY;
        }

        [[nodiscard]] friend bool operator==(const BoundingBox & lhs, const BoundingBox & rhs) = default;
    };

    template<PlanarPoint PointType>
    [[nodiscard]] BoundingBox extended(const BoundingBox & box, const PointType & point)
    {
        return {.minX = (std::min)(box.minX, static_cast<double>(point.x)),
                .minY = (std::min)(box.minY, static_cast<double>(point.y)),
                .maxX = (std::max)(box.maxX, static_cast<double>(point.x)),
                .maxY = (std::max)(box.maxY, static_cast<double>(point.y))};
    }

    //! The box holding both boxes; an empty box adds nothing.
    [[nodiscard]] inline BoundingBox merged(const BoundingBox & first, const BoundingBox & second)
    {
        return {.minX = (std::min)(first.minX, second.minX),
                .minY = (std::min)(first.minY, second.minY),
                .maxX = (std::max)(first.maxX, second.maxX),
                .maxY = (std::max)(first.maxY, second.maxY)};
    }

    //! The bounds of the points; empty when there are none.
    template<PointRange RangeType>
    [[nodiscard]] BoundingBox boundingBox(const RangeType & points)
    {
        BoundingBox box;
        for(const auto & point : points)
        {
            box = extended(box, point);
        }
        return box;
    }

    //! Shortest distance from the point to the segment between start and end: to the nearest
    //! point between them, not to the infinite line. A zero-length segment is its start point.
    template<PlanarPoint PointType, PlanarPoint EndType>
    [[nodiscard]] double distanceToSegment(const PointType & point, const EndType & start, const EndType & end)
    {
        const double segmentX{static_cast<double>(end.x) - static_cast<double>(start.x)};
        const double segmentY{static_cast<double>(end.y) - static_cast<double>(start.y)};
        const double offsetX{static_cast<double>(point.x) - static_cast<double>(start.x)};
        const double offsetY{static_cast<double>(point.y) - static_cast<double>(start.y)};
        const double lengthSquared{segmentX * segmentX + segmentY * segmentY};
        const double parameter{
          lengthSquared > 0.0 ? std::clamp((offsetX * segmentX + offsetY * segmentY) / lengthSquared, 0.0, 1.0) : 0.0};
        return std::hypot(offsetX - parameter * segmentX, offsetY - parameter * segmentY);
    }

    //! Even-odd ray casting. A point exactly on an edge may fall either side; callers that test
    //! cell or element centres never meet that case.
    template<PlanarPoint PointType, Polygon PolygonType>
    [[nodiscard]] bool isInsidePolygon(const PointType & point, const PolygonType & polygon)
    {
        const double pointX{static_cast<double>(point.x)};
        const double pointY{static_cast<double>(point.y)};
        const std::size_t count{std::ranges::size(polygon)};
        bool inside{false};
        for(std::size_t current = 0U, previous = count - 1U; current < count; previous = current++)
        {
            const double currentX{static_cast<double>(polygon[current].x)};
            const double currentY{static_cast<double>(polygon[current].y)};
            const double previousX{static_cast<double>(polygon[previous].x)};
            const double previousY{static_cast<double>(polygon[previous].y)};
            if((currentY > pointY) != (previousY > pointY)
               && pointX < currentX + (pointY - currentY) * (previousX - currentX) / (previousY - currentY))
            {
                inside = !inside;
            }
        }
        return inside;
    }

    //! Enclosed area by the shoelace formula, the same for either winding.
    template<Polygon PolygonType>
    [[nodiscard]] double polygonArea(const PolygonType & polygon)
    {
        const std::size_t count{std::ranges::size(polygon)};
        double doubledArea{0.0};
        for(std::size_t current = 0U; current < count; ++current)
        {
            const auto & corner{polygon[current]};
            const auto & following{polygon[(current + 1U) % count]};
            doubledArea += static_cast<double>(corner.x) * static_cast<double>(following.y)
                           - static_cast<double>(following.x) * static_cast<double>(corner.y);
        }
        return std::abs(doubledArea) / 2.0;
    }
}   // namespace Geometry
