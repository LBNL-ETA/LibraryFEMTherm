#include <array>
#include <vector>

#include <gtest/gtest.h>

#include "Common/PlanarGeometry.hxx"
#include "Model/Case.hxx"
#include "THMZ/Geometry/Geometry.hxx"

namespace
{
    //! A point type of the test's own: the functions must take anything with an x and a y.
    struct Corner
    {
        double x{0.0};
        double y{0.0};
    };

    //! A 4 x 2 rectangle with its lower-left corner at the origin, counter-clockwise.
    const std::vector<Corner> rectangle{
      {.x = 0.0, .y = 0.0}, {.x = 4.0, .y = 0.0}, {.x = 4.0, .y = 2.0}, {.x = 0.0, .y = 2.0}};

    //! An L made of a 2 x 2 square with a 1 x 1 square cut from its upper-right corner.
    const std::vector<Corner> lShape{{.x = 0.0, .y = 0.0},
                                     {.x = 2.0, .y = 0.0},
                                     {.x = 2.0, .y = 1.0},
                                     {.x = 1.0, .y = 1.0},
                                     {.x = 1.0, .y = 2.0},
                                     {.x = 0.0, .y = 2.0}};
}   // namespace

TEST(PlanarGeometry, DistanceToSegmentInterior)
{
    const Corner start{.x = 0.0, .y = 0.0};
    const Corner end{.x = 4.0, .y = 0.0};
    EXPECT_DOUBLE_EQ(Geometry::distanceToSegment(Corner{.x = 1.0, .y = 3.0}, start, end), 3.0);
}

TEST(PlanarGeometry, DistanceToSegmentBeyondAnEndIsToThatEnd)
{
    const Corner start{.x = 0.0, .y = 0.0};
    const Corner end{.x = 4.0, .y = 0.0};
    EXPECT_DOUBLE_EQ(Geometry::distanceToSegment(Corner{.x = 7.0, .y = 4.0}, start, end), 5.0);
    EXPECT_DOUBLE_EQ(Geometry::distanceToSegment(Corner{.x = -3.0, .y = -4.0}, start, end), 5.0);
}

TEST(PlanarGeometry, DistanceToZeroLengthSegmentIsToItsPoint)
{
    const Corner point{.x = 1.0, .y = 1.0};
    EXPECT_DOUBLE_EQ(Geometry::distanceToSegment(Corner{.x = 4.0, .y = 5.0}, point, point), 5.0);
}

TEST(PlanarGeometry, InsidePolygon)
{
    EXPECT_TRUE(Geometry::isInsidePolygon(Corner{.x = 1.0, .y = 1.0}, rectangle));
    EXPECT_FALSE(Geometry::isInsidePolygon(Corner{.x = 5.0, .y = 1.0}, rectangle));
    EXPECT_FALSE(Geometry::isInsidePolygon(Corner{.x = 1.0, .y = -0.5}, rectangle));
}

TEST(PlanarGeometry, InsideConcavePolygon)
{
    EXPECT_TRUE(Geometry::isInsidePolygon(Corner{.x = 0.5, .y = 1.5}, lShape));
    EXPECT_FALSE(Geometry::isInsidePolygon(Corner{.x = 1.5, .y = 1.5}, lShape));
}

TEST(PlanarGeometry, AreaIsTheSameForEitherWinding)
{
    const std::vector<Corner> clockwise{rectangle.rbegin(), rectangle.rend()};
    EXPECT_DOUBLE_EQ(Geometry::polygonArea(rectangle), 8.0);
    EXPECT_DOUBLE_EQ(Geometry::polygonArea(clockwise), 8.0);
    EXPECT_DOUBLE_EQ(Geometry::polygonArea(lShape), 3.0);
}

TEST(PlanarGeometry, BoundingBox)
{
    const auto box{Geometry::boundingBox(lShape)};
    EXPECT_EQ(box, (Geometry::BoundingBox{.minX = 0.0, .minY = 0.0, .maxX = 2.0, .maxY = 2.0}));
    EXPECT_DOUBLE_EQ(box.width(), 2.0);
    EXPECT_DOUBLE_EQ(box.height(), 2.0);
}

TEST(PlanarGeometry, BoundingBoxOfNothingIsEmpty)
{
    const auto box{Geometry::boundingBox(std::vector<Corner>{})};
    EXPECT_TRUE(box.empty());
    EXPECT_DOUBLE_EQ(box.width(), 0.0);
    EXPECT_DOUBLE_EQ(box.height(), 0.0);
}

TEST(PlanarGeometry, ExtendedBoxTakesInThePoint)
{
    const Geometry::BoundingBox box{.minX = 0.0, .minY = 0.0, .maxX = 1.0, .maxY = 1.0};
    EXPECT_EQ(Geometry::extended(box, Corner{.x = 3.0, .y = -2.0}),
              (Geometry::BoundingBox{.minX = 0.0, .minY = -2.0, .maxX = 3.0, .maxY = 1.0}));
    EXPECT_EQ(Geometry::extended(box, Corner{.x = 0.5, .y = 0.5}), box);
}

TEST(PlanarGeometry, EmptyBoxExtendedIsThePoint)
{
    EXPECT_EQ(Geometry::extended(Geometry::BoundingBox{}, Corner{.x = 2.0, .y = 3.0}),
              (Geometry::BoundingBox{.minX = 2.0, .minY = 3.0, .maxX = 2.0, .maxY = 3.0}));
}

TEST(PlanarGeometry, NothingIsInsideAnEmptyPolygon)
{
    EXPECT_FALSE(Geometry::isInsidePolygon(Corner{.x = 0.0, .y = 0.0}, std::vector<Corner>{}));
    EXPECT_DOUBLE_EQ(Geometry::polygonArea(std::vector<Corner>{}), 0.0);
}

TEST(PlanarGeometry, TakesTheLibraryPointTypes)
{
    static_assert(Geometry::PlanarPoint<ThermFile::Point>);
    static_assert(Geometry::PlanarPoint<ThermFile::Model::Point>);
    static_assert(!Geometry::PlanarPoint<double>);

    const std::vector<ThermFile::Point> square{
      {.x = 0.0, .y = 0.0}, {.x = 1.0, .y = 0.0}, {.x = 1.0, .y = 1.0}, {.x = 0.0, .y = 1.0}};
    EXPECT_DOUBLE_EQ(Geometry::polygonArea(square), 1.0);
    EXPECT_TRUE(Geometry::isInsidePolygon(ThermFile::Model::Point{.x = 0.5, .y = 0.5}, square));
}

TEST(PlanarGeometry, MergedBoxHoldsBoth)
{
    const std::array<Corner, 2U> far{Corner{.x = 10.0, .y = -1.0}, Corner{.x = 11.0, .y = 3.0}};
    const auto box{Geometry::merged(Geometry::boundingBox(rectangle), Geometry::boundingBox(far))};
    EXPECT_EQ(box, (Geometry::BoundingBox{.minX = 0.0, .minY = -1.0, .maxX = 11.0, .maxY = 3.0}));
    EXPECT_EQ(Geometry::merged(box, Geometry::BoundingBox{}), box);
}
