#include <algorithm>
#include <cmath>
#include <format>

#include "Model.hxx"

namespace ThermFile::Authoring
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
                if(!modelCase.materials.contains(region.material))
                {
                    found.push_back(std::format("region {} names material '{}', which the case does not carry",
                                                index,
                                                region.material));
                }
            }
        }

        void checkSegments(const ModelCase & modelCase, std::vector<std::string> & found)
        {
            for(std::size_t index = 0U; index < modelCase.segments.size(); ++index)
            {
                const auto & segment{modelCase.segments[index]};
                if(segment.region >= modelCase.regions.size())
                {
                    found.push_back(std::format("segment {} borders region {}, which does not exist",
                                                index,
                                                segment.region));
                    continue;
                }
                const auto & region{modelCase.regions[segment.region]};
                if(region.points.size() >= 3U && !liesOnAnEdge(region, segment))
                {
                    found.push_back(std::format(
                      "segment {} from ({}, {}) to ({}, {}) does not lie along an edge of region {}",
                      index,
                      segment.start.x,
                      segment.start.y,
                      segment.end.x,
                      segment.end.y,
                      segment.region));
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
        checkSegments(modelCase, found);
        checkSchedule(modelCase, found);
        return found;
    }

    std::optional<Material> material(const ModelCase & modelCase, const std::string_view name)
    {
        const auto entry{modelCase.materials.find(std::string{name})};
        if(entry == modelCase.materials.end())
        {
            return std::nullopt;
        }
        return entry->second;
    }

    std::optional<Material> regionMaterial(const ModelCase & modelCase, const std::size_t index)
    {
        if(index >= modelCase.regions.size())
        {
            return std::nullopt;
        }
        return material(modelCase, modelCase.regions[index].material);
    }

    std::string recordName(const Boundary & boundary)
    {
        return std::visit(
          [](const auto & kind) -> std::string {
              using Kind = std::decay_t<decltype(kind)>;
              if constexpr(std::is_same_v<Kind, Sealed>)
              {
                  return "Sealed";
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
}   // namespace ThermFile::Authoring
