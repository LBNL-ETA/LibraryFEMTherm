#include <algorithm>
#include <cmath>
#include <format>

#include "Case.hxx"

namespace ThermFile::Model
{
    double Schedule::duration() const
    {
        return dtime * static_cast<double>(nSteps);
    }

    namespace
    {
        //! How far off a region's outline a segment endpoint may sit and still count as on
        //! it, in metres. Coordinates are stated by hand; this absorbs arithmetic on them.
        constexpr double outlineTolerance{1e-9};

        double distanceToEdge(const Point & point, const Point & edgeStart, const Point & edgeEnd)
        {
            const double edgeX{edgeEnd.x - edgeStart.x};
            const double edgeY{edgeEnd.y - edgeStart.y};
            const double lengthSquared{edgeX * edgeX + edgeY * edgeY};
            double parameter{0.0};
            if(lengthSquared > 0.0)
            {
                parameter = ((point.x - edgeStart.x) * edgeX + (point.y - edgeStart.y) * edgeY) / lengthSquared;
                parameter = std::clamp(parameter, 0.0, 1.0);
            }
            const double offsetX{point.x - (edgeStart.x + parameter * edgeX)};
            const double offsetY{point.y - (edgeStart.y + parameter * edgeY)};
            return std::sqrt(offsetX * offsetX + offsetY * offsetY);
        }

        //! True when both endpoints lie on the same edge of the region's outline: a segment
        //! is a span of one edge, never a chord or a bridge between two edges.
        bool liesOnAnEdge(const Region & region, const Segment & segment)
        {
            const auto & points{region.points};
            for(std::size_t index = 0U; index < points.size(); ++index)
            {
                const auto & edgeStart{points[index]};
                const auto & edgeEnd{points[(index + 1U) % points.size()]};
                if(distanceToEdge(segment.start, edgeStart, edgeEnd) <= outlineTolerance
                   && distanceToEdge(segment.end, edgeStart, edgeEnd) <= outlineTolerance)
                {
                    return true;
                }
            }
            return false;
        }

        // --------------------------------------------------------------------------------
        // The outline: the parts of region edges nothing covers
        // --------------------------------------------------------------------------------

        //! A stretch of an edge, as fractions of its length from its start.
        struct Span
        {
            double start{0.0};
            double end{0.0};
        };

        double edgeLength(const Point & edgeStart, const Point & edgeEnd)
        {
            return std::hypot(edgeEnd.x - edgeStart.x, edgeEnd.y - edgeStart.y);
        }

        //! Distance from a point to the infinite line through an edge.
        double distanceToLine(const Point & point, const Point & edgeStart, const Point & edgeEnd)
        {
            const double length{edgeLength(edgeStart, edgeEnd)};
            if(length <= 0.0)
            {
                return edgeLength(point, edgeStart);
            }
            const double cross{(edgeEnd.x - edgeStart.x) * (point.y - edgeStart.y)
                               - (edgeEnd.y - edgeStart.y) * (point.x - edgeStart.x)};
            return std::abs(cross) / length;
        }

        //! Where a point on the line falls along the edge: 0 at its start, 1 at its end.
        double parameterAlong(const Point & point, const Point & edgeStart, const Point & edgeEnd)
        {
            const double edgeX{edgeEnd.x - edgeStart.x};
            const double edgeY{edgeEnd.y - edgeStart.y};
            return ((point.x - edgeStart.x) * edgeX + (point.y - edgeStart.y) * edgeY) / (edgeX * edgeX + edgeY * edgeY);
        }

        Point along(const Point & edgeStart, const Point & edgeEnd, const double parameter)
        {
            return Point{edgeStart.x + parameter * (edgeEnd.x - edgeStart.x),
                         edgeStart.y + parameter * (edgeEnd.y - edgeStart.y)};
        }

        //! The stretch of an edge a straight piece covers, if the piece is collinear with it
        //! and the overlap is longer than the tolerance.
        std::optional<Span>
          coveredBy(const Point & pieceStart, const Point & pieceEnd, const Point & edgeStart, const Point & edgeEnd)
        {
            if(distanceToLine(pieceStart, edgeStart, edgeEnd) > outlineTolerance
               || distanceToLine(pieceEnd, edgeStart, edgeEnd) > outlineTolerance)
            {
                return std::nullopt;
            }
            const double first{parameterAlong(pieceStart, edgeStart, edgeEnd)};
            const double second{parameterAlong(pieceEnd, edgeStart, edgeEnd)};
            const Span span{.start = std::clamp(std::min(first, second), 0.0, 1.0),
                            .end = std::clamp(std::max(first, second), 0.0, 1.0)};
            if((span.end - span.start) * edgeLength(edgeStart, edgeEnd) <= outlineTolerance)
            {
                return std::nullopt;
            }
            return span;
        }

        //! Every stretch of the edge covered by another region's edge or by a stated segment.
        std::vector<Span> coveredSpans(const ModelCase & modelCase,
                                       const std::size_t regionIndex,
                                       const Point & edgeStart,
                                       const Point & edgeEnd)
        {
            std::vector<Span> spans;
            const auto consider{[&](const Point & pieceStart, const Point & pieceEnd) {
                if(const auto span{coveredBy(pieceStart, pieceEnd, edgeStart, edgeEnd)})
                {
                    spans.push_back(span.value());
                }
            }};
            for(std::size_t other = 0U; other < modelCase.regions.size(); ++other)
            {
                const auto & theirs{modelCase.regions[other].points};
                for(std::size_t corner = 0U; other != regionIndex && corner < theirs.size(); ++corner)
                {
                    consider(theirs[corner], theirs[(corner + 1U) % theirs.size()]);
                }
            }
            for(const auto & segment : modelCase.segments)
            {
                consider(segment.start, segment.end);
            }
            return spans;
        }

        //! The complement of the covered stretches within [0, 1], ignoring gaps shorter
        //! than the tolerance (given as a fraction of the edge).
        std::vector<Span> gapsBetween(std::vector<Span> covered, const double fractionTolerance)
        {
            std::ranges::sort(covered, {}, &Span::start);
            std::vector<Span> gaps;
            double reached{0.0};
            for(const auto & span : covered)
            {
                if(span.start - reached > fractionTolerance)
                {
                    gaps.push_back(Span{.start = reached, .end = span.start});
                }
                reached = std::max(reached, span.end);
            }
            if(1.0 - reached > fractionTolerance)
            {
                gaps.push_back(Span{.start = reached, .end = 1.0});
            }
            return gaps;
        }

        //! The adiabatic segments one edge of a region needs.
        std::vector<Segment> edgeGaps(const ModelCase & modelCase, const std::size_t regionIndex, const std::size_t corner)
        {
            const auto & points{modelCase.regions[regionIndex].points};
            const Point & edgeStart{points[corner]};
            const Point & edgeEnd{points[(corner + 1U) % points.size()]};
            const double length{edgeLength(edgeStart, edgeEnd)};
            std::vector<Segment> found;
            if(length <= outlineTolerance)
            {
                return found;
            }
            const auto covered{coveredSpans(modelCase, regionIndex, edgeStart, edgeEnd)};
            for(const auto & gap : gapsBetween(covered, outlineTolerance / length))
            {
                found.push_back(Segment{.kind = Adiabatic{},
                                        .start = along(edgeStart, edgeEnd, gap.start),
                                        .end = along(edgeStart, edgeEnd, gap.end),
                                        .region = regionIndex});
            }
            return found;
        }

        void checkRegions(const ModelCase & modelCase, std::vector<std::string> & found)
        {
            if(modelCase.regions.empty())
            {
                found.emplace_back("a model needs at least one region");
            }
            for(std::size_t index = 0U; index < modelCase.regions.size(); ++index)
            {
                const auto & region{modelCase.regions[index]};
                if(region.points.size() < 3U)
                {
                    found.push_back(std::format("region {} has {} points; a region needs at least three",
                                                index,
                                                region.points.size()));
                }
                if(region.material.Name.empty())
                {
                    found.push_back(std::format("region {} has a material with no name", index));
                }
            }
        }

        //! The archive's library keeps one record per UUID and THERM finds records by name,
        //! so two regions whose materials share a name must carry the same record.
        void checkMaterialNames(const ModelCase & modelCase, std::vector<std::string> & found)
        {
            const auto & regions{modelCase.regions};
            for(std::size_t first = 0U; first < regions.size(); ++first)
            {
                for(std::size_t second = first + 1U; second < regions.size(); ++second)
                {
                    const auto & mine{regions[first].material};
                    const auto & theirs{regions[second].material};
                    if(mine.Name == theirs.Name && mine.UUID != theirs.UUID)
                    {
                        found.push_back(std::format("regions {} and {} carry two different materials both named "
                                                    "'{}'; the library keeps one record per name",
                                                    first,
                                                    second,
                                                    mine.Name));
                    }
                }
            }
        }

        //! The regions whose outline carries the segment.
        std::vector<std::size_t> regionsAlong(const ModelCase & modelCase, const Segment & segment)
        {
            std::vector<std::size_t> along;
            for(std::size_t index = 0U; index < modelCase.regions.size(); ++index)
            {
                if(liesOnAnEdge(modelCase.regions[index], segment))
                {
                    along.push_back(index);
                }
            }
            return along;
        }

        std::string describe(const std::size_t index, const Segment & segment)
        {
            const std::string who{segment.name.has_value() ? std::format("segment {} '{}'", index, segment.name.value())
                                                           : std::format("segment {}", index)};
            return std::format("{} from ({}, {}) to ({}, {})",
                               who,
                               segment.start.x,
                               segment.start.y,
                               segment.end.x,
                               segment.end.y);
        }

        //! The issue with a segment that names its region, if any.
        std::optional<std::string> namedRegionIssue(const ModelCase & modelCase, const std::size_t index)
        {
            const auto & segment{modelCase.segments[index]};
            const auto region{segment.region.value()};
            if(region >= modelCase.regions.size())
            {
                return std::format("segment {} borders region {}, which does not exist", index, region);
            }
            if(!liesOnAnEdge(modelCase.regions[region], segment))
            {
                return std::format("{} does not lie along an edge of region {}", describe(index, segment), region);
            }
            return std::nullopt;
        }

        //! The issue with a segment that leaves its region to be found, if any.
        std::optional<std::string> foundRegionIssue(const ModelCase & modelCase, const std::size_t index)
        {
            const auto & segment{modelCase.segments[index]};
            const auto along{regionsAlong(modelCase, segment)};
            if(along.empty())
            {
                return std::format("{} does not lie along an edge of any region", describe(index, segment));
            }
            if(along.size() > 1U)
            {
                return std::format("{} lies on an edge shared by regions {} and {}; state which with region",
                                   describe(index, segment),
                                   along[0],
                                   along[1]);
            }
            return std::nullopt;
        }

        std::optional<std::string> segmentIssue(const ModelCase & modelCase, const std::size_t index)
        {
            return modelCase.segments[index].region.has_value() ? namedRegionIssue(modelCase, index)
                                                                : foundRegionIssue(modelCase, index);
        }

        bool everyRegionHasEdges(const ModelCase & modelCase)
        {
            return std::ranges::all_of(modelCase.regions,
                                       [](const Region & region) { return region.points.size() >= 3U; });
        }

        void checkSegments(const ModelCase & modelCase, std::vector<std::string> & found)
        {
            if(!everyRegionHasEdges(modelCase))
            {
                return;
            }
            for(std::size_t index = 0U; index < modelCase.segments.size(); ++index)
            {
                if(const auto issue{segmentIssue(modelCase, index)})
                {
                    found.push_back(issue.value());
                }
            }
        }

        void checkSchedule(const ModelCase & modelCase, std::vector<std::string> & found)
        {
            if(modelCase.schedule.dtime <= 0.0)
            {
                found.emplace_back("the time step must be positive");
            }
            if(modelCase.schedule.nSteps == 0U)
            {
                found.emplace_back("the schedule needs at least one step");
            }
        }
    }   // namespace

    std::vector<std::string> issues(const ModelCase & modelCase)
    {
        std::vector<std::string> found;
        checkRegions(modelCase, found);
        checkMaterialNames(modelCase, found);
        checkSegments(modelCase, found);
        checkSchedule(modelCase, found);
        return found;
    }

    std::optional<std::size_t> segmentRegion(const ModelCase & modelCase, const std::size_t index)
    {
        if(index >= modelCase.segments.size() || !everyRegionHasEdges(modelCase)
           || segmentIssue(modelCase, index).has_value())
        {
            return std::nullopt;
        }
        const auto & segment{modelCase.segments[index]};
        return segment.region.has_value() ? segment.region : std::optional{regionsAlong(modelCase, segment)[0]};
    }

    std::string kindColor(const Boundary & boundary)
    {
        return std::visit(
          [](const auto & kind) -> std::string {
              using Kind = std::decay_t<decltype(kind)>;
              if constexpr(std::is_same_v<Kind, Adiabatic>)
              {
                  return "0x000000";
              }
              else if constexpr(std::is_same_v<Kind, Prescribed>)
              {
                  return "0xD32F2F";
              }
              else
              {
                  return "0x1E5A96";
              }
          },
          boundary);
    }

    std::string segmentColor(const Segment & segment)
    {
        return segment.color.value_or(kindColor(segment.kind));
    }

    std::vector<Segment> outlineGaps(const ModelCase & modelCase)
    {
        std::vector<Segment> found;
        for(std::size_t regionIndex = 0U; regionIndex < modelCase.regions.size(); ++regionIndex)
        {
            const auto & points{modelCase.regions[regionIndex].points};
            for(std::size_t corner = 0U; points.size() >= 3U && corner < points.size(); ++corner)
            {
                const auto gaps{edgeGaps(modelCase, regionIndex, corner)};
                found.insert(found.end(), gaps.begin(), gaps.end());
            }
        }
        return found;
    }

    ModelCase completed(const ModelCase & modelCase)
    {
        auto result{modelCase};
        const auto gaps{outlineGaps(modelCase)};
        result.segments.insert(result.segments.end(), gaps.begin(), gaps.end());
        return result;
    }

    std::optional<MaterialsLibrary::Material> regionMaterial(const ModelCase & modelCase, const std::size_t index)
    {
        if(index >= modelCase.regions.size())
        {
            return std::nullopt;
        }
        return modelCase.regions[index].material;
    }

    std::string recordName(const Boundary & boundary)
    {
        return std::visit(
          [](const auto & kind) -> std::string {
              using Kind = std::decay_t<decltype(kind)>;
              if constexpr(std::is_same_v<Kind, Adiabatic>)
              {
                  return "Adiabatic";
              }
              else if constexpr(std::is_same_v<Kind, Prescribed>)
              {
                  return kind.humidity.has_value() ? "Prescribed temperature and humidity"
                                                   : "Prescribed temperature";
              }
              else
              {
                  return "Convective exchange";
              }
          },
          boundary);
    }
}   // namespace ThermFile::Model
