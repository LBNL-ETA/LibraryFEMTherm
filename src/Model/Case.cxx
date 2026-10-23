#include <algorithm>
#include <cmath>
#include <format>
#include <numeric>

#include "Case.hxx"
#include "Common/PlanarGeometry.hxx"
#include "TimeSeriesData/Tags.hxx"

namespace ThermFile::Model
{
    using TimeSeriesLibrary::SeriesRole;

    double Schedule::duration() const
    {
        return std::accumulate(stepLengths.begin(), stepLengths.end(), 0.0);
    }

    Schedule uniformSchedule(const double dtime, const std::size_t nSteps, const Start start)
    {
        return Schedule{.start = start, .stepLengths = std::vector<double>(nSteps, dtime)};
    }

    std::size_t nSteps(const Schedule & schedule)
    {
        return schedule.stepLengths.size();
    }

    std::optional<double> uniformStep(const Schedule & schedule)
    {
        const auto & lengths{schedule.stepLengths};
        const bool uniform{!lengths.empty()
                           && std::ranges::all_of(lengths, [&lengths](const double length) { return length == lengths.front(); })};
        return uniform ? std::optional{lengths.front()} : std::nullopt;
    }

    TimeSeriesLibrary::TimeAxis axisOf(const Schedule & schedule)
    {
        return TimeSeriesLibrary::TimeAxis{.month = schedule.start.month,
                                           .day = schedule.start.day,
                                           .hour = schedule.start.hour,
                                           .minute = schedule.start.minute,
                                           .stepSeconds = TimeSeriesLibrary::stepSecondsOf(schedule.stepLengths)};
    }

    Schedule scheduleFor(const TimeSeriesLibrary::TimeSeriesData & dataset)
    {
        const auto & axis{dataset.axis};
        return Schedule{.start = Start{.month = axis.month, .day = axis.day, .hour = axis.hour, .minute = axis.minute},
                        .stepLengths = TimeSeriesLibrary::stepLengths(axis, TimeSeriesLibrary::steps(dataset))};
    }

    namespace
    {
        BCLibrary::SurfaceExchange exchangeOf(const Convective & convective)
        {
            return BCLibrary::SurfaceExchange{
              .relativeHumidity = convective.humidity,
              .convection = BCLibrary::Convection{.model = convective.model,
                                                  .airTemperature = convective.airTemperature,
                                                  .filmCoefficient = convective.filmCoefficient,
                                                  .windSpeed = convective.windSpeed,
                                                  .windDirection = convective.windDirection},
              .radiation = convective.radiation,
              .flux = convective.flux};
        }
    }   // namespace

    RecordData recordData(const Boundary & boundary)
    {
        if(const auto * convective{std::get_if<Convective>(&boundary)})
        {
            return exchangeOf(*convective);
        }
        if(const auto * prescribed{std::get_if<Prescribed>(&boundary)})
        {
            return BCLibrary::PrescribedState{.temperature = prescribed->temperature,
                                              .relativeHumidity = prescribed->humidity};
        }
        return BCLibrary::NoExchange{};
    }

    namespace
    {
        //! How far off a region's outline a segment endpoint may sit and still count as on
        //! it, in metres. Coordinates are stated by hand; this absorbs arithmetic on them.
        constexpr double outlineTolerance{1e-9};

        //! True when both endpoints lie on the same edge of the region's outline: a segment
        //! is a span of one edge, never a chord or a bridge between two edges.
        bool liesOnAnEdge(const Region & region, const Segment & segment)
        {
            const auto & points{region.points};
            for(std::size_t index = 0U; index < points.size(); ++index)
            {
                const auto & edgeStart{points[index]};
                const auto & edgeEnd{points[(index + 1U) % points.size()]};
                if(Geometry::distanceToSegment(segment.start, edgeStart, edgeEnd) <= outlineTolerance
                   && Geometry::distanceToSegment(segment.end, edgeStart, edgeEnd) <= outlineTolerance)
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
            const auto & lengths{modelCase.schedule.stepLengths};
            if(lengths.empty())
            {
                found.emplace_back("the schedule needs at least one step");
                return;
            }
            const auto bad{std::ranges::find_if(lengths, [](const double length) { return length <= 0.0; })};
            if(bad != lengths.end())
            {
                found.push_back(std::format("the schedule's step {} must be positive", bad - lengths.begin() + 1));
            }
        }

        bool bindsDataset(const Segment & segment)
        {
            return segment.dataset.has_value();
        }

        //! A stated schedule beside bound datasets must be their clock; an unstated one is
        //! taken from them. Without datasets, the schedule is the clock and must be one.
        void checkClock(const ModelCase & modelCase, std::vector<std::string> & found)
        {
            if(std::ranges::none_of(modelCase.segments, bindsDataset))
            {
                checkSchedule(modelCase, found);
                return;
            }
            if(modelCase.schedule.stepLengths.empty())
            {
                return;
            }
            for(std::size_t index = 0U; index < modelCase.segments.size(); ++index)
            {
                const auto & segment{modelCase.segments[index]};
                if(bindsDataset(segment) && scheduleFor(segment.dataset.value()) != modelCase.schedule)
                {
                    found.push_back(std::format("{}: the stated schedule differs from the axis of its dataset '{}' "
                                                "({}); leave the schedule out to run on the datasets' clock",
                                                describe(index, segment),
                                                segment.dataset->Name,
                                                TimeSeriesLibrary::describeAxis(segment.dataset.value())));
                }
            }
        }

        //! The roles a segment's condition reads from its dataset, in the record's order.
        std::vector<SeriesRole> rolesRead(const Segment & segment)
        {
            return BCLibrary::requiredRoles(BCLibrary::BoundaryCondition{.data = recordData(segment.kind)});
        }

        std::string roleNames(const std::vector<SeriesRole> & roles)
        {
            std::string names;
            for(const auto role : roles)
            {
                names += (names.empty() ? "" : ", ") + TimeSeriesLibrary::seriesRoleToString(role);
            }
            return names;
        }

        //! The radiation a segment states, or nothing.
        const Radiation * radiationOf(const Segment & segment)
        {
            const auto * convective{std::get_if<Convective>(&segment.kind)};
            return convective != nullptr && convective->radiation.has_value() ? &convective->radiation.value()
                                                                              : nullptr;
        }

        //! Whether a black body reads any input from a dataset series.
        bool blackBodyReadsSeries(const Radiation & radiation)
        {
            const auto * blackBody{std::get_if<BCLibrary::BlackBodyRadiation>(&radiation)};
            return blackBody != nullptr
                   && (BCLibrary::isFromTimeSeries(blackBody->temperature)
                       || BCLibrary::isFromTimeSeries(blackBody->emissivity));
        }

        //! The wind a convection model reads: the correlations for an outside surface need
        //! the speed, and those that distinguish windward from leeward need the direction.
        struct WindInputs
        {
            bool speed{false};
            bool direction{false};
        };

        WindInputs windInputsOf(const BCLibrary::ConvectionModel model)
        {
            using Model = BCLibrary::ConvectionModel;
            switch(model)
            {
                case Model::ASHRAE_NFRC_Outside:
                    return {.speed = true, .direction = false};
                case Model::Yazdanian_Klems:
                case Model::Kimura:
                case Model::Montazeri:
                    return {.speed = true, .direction = true};
                default:
                    return {};
            }
        }

        //! The input a convection model reads but the segment does not give, if any.
        std::optional<std::string> missingConvectionInput(const Convective & convective)
        {
            const auto wind{windInputsOf(convective.model)};
            if(convective.model == BCLibrary::ConvectionModel::Fixed_Convection_Coefficient
               && !convective.filmCoefficient.has_value())
            {
                return "a film coefficient";
            }
            if(wind.speed && !convective.windSpeed.has_value())
            {
                return "the wind speed";
            }
            if(wind.direction && !convective.windDirection.has_value())
            {
                return "the wind direction";
            }
            return std::nullopt;
        }

        void checkConvectionInputs(const ModelCase & modelCase, std::vector<std::string> & found)
        {
            for(std::size_t index = 0U; index < modelCase.segments.size(); ++index)
            {
                const auto & segment{modelCase.segments[index]};
                const auto * convective{std::get_if<Convective>(&segment.kind)};
                if(convective == nullptr)
                {
                    continue;
                }
                if(const auto missing{missingConvectionInput(*convective)})
                {
                    found.push_back(std::format(
                      "{} needs {} for its convection model", describe(index, segment), missing.value()));
                }
            }
        }

        //! A steady-state archive keeps its conditions in THERM's steady-state library,
        //! which has no record for a prescribed surface state, and has no datasets to read.
        void checkSteadyStateSegments(const ModelCase & modelCase, std::vector<std::string> & found)
        {
            for(std::size_t index = 0U; index < modelCase.segments.size(); ++index)
            {
                const auto & segment{modelCase.segments[index]};
                if(std::holds_alternative<Prescribed>(segment.kind))
                {
                    found.push_back(std::format("{} is prescribed, which a steady-state case cannot write; "
                                                "state it as Convective",
                                                describe(index, segment)));
                }
                if(const auto roles{rolesRead(segment)}; !roles.empty())
                {
                    found.push_back(std::format("{} reads {} from a dataset, which a steady-state case has "
                                                "none of; state it as a constant",
                                                describe(index, segment),
                                                roleNames(roles)));
                }
                if(bindsDataset(segment))
                {
                    found.push_back(std::format("{} binds a dataset, which a steady-state case does not write",
                                                describe(index, segment)));
                }
            }
        }

        //! A segment whose condition reads roles needs a dataset that provides every one of
        //! them: the pairing THERM's assignment dialog checks. The engine would otherwise
        //! leave the condition out of the run.
        void checkSegmentDatasets(const ModelCase & modelCase, std::vector<std::string> & found)
        {
            for(std::size_t index = 0U; index < modelCase.segments.size(); ++index)
            {
                const auto & segment{modelCase.segments[index]};
                const auto roles{rolesRead(segment)};
                if(roles.empty())
                {
                    continue;
                }
                if(!bindsDataset(segment))
                {
                    found.push_back(std::format(
                      "{} reads {} from a dataset but binds none", describe(index, segment), roleNames(roles)));
                    continue;
                }
                const BCLibrary::BoundaryCondition record{.data = recordData(segment.kind)};
                if(const auto missing{BCLibrary::missingRoles(record, segment.dataset)}; !missing.empty())
                {
                    found.push_back(std::format("{}: its dataset '{}' provides no {}",
                                                describe(index, segment),
                                                segment.dataset->Name,
                                                roleNames(missing)));
                }
            }
        }

        //! The radiation the engine runs through a clock: fixed radiation, step by step, and
        //! a black body held at constants. An enclosure is not run at all in a transient run.
        std::optional<std::string> transientRadiationIssue(const Radiation & radiation)
        {
            if(std::holds_alternative<BCLibrary::AutomaticEnclosure>(radiation)
               || std::holds_alternative<BCLibrary::ManualEnclosure>(radiation))
            {
                return "radiates into an enclosure, which a transient run does not model; state fixed "
                       "radiation or a black body";
            }
            if(blackBodyReadsSeries(radiation))
            {
                return "radiates as a black body read from a series, which a transient run holds constant; "
                       "state constants, or fixed-coefficient radiation to read a dataset";
            }
            return std::nullopt;
        }

        void checkTransientRadiation(const ModelCase & modelCase, std::vector<std::string> & found)
        {
            for(std::size_t index = 0U; index < modelCase.segments.size(); ++index)
            {
                const auto * radiation{radiationOf(modelCase.segments[index])};
                if(radiation == nullptr)
                {
                    continue;
                }
                if(const auto issue{transientRadiationIssue(*radiation)})
                {
                    found.push_back(std::format("{} {}", describe(index, modelCase.segments[index]), issue.value()));
                }
            }
        }

        void checkMode(const ModelCase & modelCase, std::vector<std::string> & found)
        {
            if(modelCase.mode == ThermFile::CalculationMode::cmTransient)
            {
                checkClock(modelCase, found);
                checkSegmentDatasets(modelCase, found);
                checkTransientRadiation(modelCase, found);
            }
            else if(isSteadyState(modelCase))
            {
                checkSteadyStateSegments(modelCase, found);
            }
            else
            {
                found.emplace_back("the mode must be transient or steady state");
            }
        }
    }   // namespace

    std::vector<std::string> issues(const ModelCase & modelCase)
    {
        std::vector<std::string> found;
        checkRegions(modelCase, found);
        checkMaterialNames(modelCase, found);
        checkSegments(modelCase, found);
        checkConvectionInputs(modelCase, found);
        checkMode(modelCase, found);
        return found;
    }

    bool isSteadyState(const ModelCase & modelCase)
    {
        return modelCase.mode == ThermFile::CalculationMode::cmSteadyState;
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
}   // namespace ThermFile::Model
