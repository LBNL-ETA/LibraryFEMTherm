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
                if(region.material.name.empty())
                {
                    found.push_back(std::format("region {} has a material with no name", index));
                }
            }
        }

        //! The library keeps one record per material name, so two regions that use the
        //! same name must state the same material.
        void checkMaterialNames(const ModelCase & modelCase, std::vector<std::string> & found)
        {
            const auto & regions{modelCase.regions};
            for(std::size_t first = 0U; first < regions.size(); ++first)
            {
                for(std::size_t second = first + 1U; second < regions.size(); ++second)
                {
                    const auto & mine{regions[first].material};
                    const auto & theirs{regions[second].material};
                    if(mine.name == theirs.name && mine != theirs)
                    {
                        found.push_back(std::format("regions {} and {} both use a material named '{}' with "
                                                    "different properties; the library keeps one record per name",
                                                    first,
                                                    second,
                                                    mine.name));
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
            return std::format("segment {} from ({}, {}) to ({}, {})",
                               index,
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

    std::optional<Material> regionMaterial(const ModelCase & modelCase, const std::size_t index)
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
