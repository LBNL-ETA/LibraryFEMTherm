#include <algorithm>
#include <cmath>
#include <format>
#include <numeric>

#include <lbnl/algorithm.hxx>
#include <lbnl/optional.hxx>

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

        void checkRegions(const ModelCase & modelCase, std::vector<Issue> & found)
        {
            if(modelCase.regions.empty())
            {
                found.push_back(Issue{.kind = IssueKind::NoRegions});
            }
            for(std::size_t index = 0U; index < modelCase.regions.size(); ++index)
            {
                const auto & [material, points]{modelCase.regions[index]};
                if(points.size() < 3U)
                {
                    found.push_back(Issue{.kind = IssueKind::TooFewPoints, .region = index});
                }
                if(material.Name.empty())
                {
                    found.push_back(Issue{.kind = IssueKind::UnnamedMaterial, .region = index});
                }
            }
        }

        //! The archive's library keeps one record per UUID and THERM finds records by name,
        //! so two regions whose materials share a name must carry the same record.
        void checkMaterialNames(const ModelCase & modelCase, std::vector<Issue> & found)
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
                        found.push_back(Issue{.kind = IssueKind::MaterialNameClash, .region = first, .otherRegion = second});
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

        //! The issue with a segment that names its region, if any.
        std::optional<Issue> namedRegionIssue(const ModelCase & modelCase, const std::size_t index)
        {
            const auto & segment{modelCase.segments[index]};
            const auto region{segment.region.value()};
            if(region >= modelCase.regions.size())
            {
                return Issue{.kind = IssueKind::RegionMissing, .region = region, .segment = index};
            }
            if(!liesOnAnEdge(modelCase.regions[region], segment))
            {
                return Issue{.kind = IssueKind::OffRegion, .region = region, .segment = index};
            }
            return std::nullopt;
        }

        //! The issue with a segment that leaves its region to be found, if any.
        std::optional<Issue> foundRegionIssue(const ModelCase & modelCase, const std::size_t index)
        {
            const auto along{regionsAlong(modelCase, modelCase.segments[index])};
            if(along.empty())
            {
                return Issue{.kind = IssueKind::OffEveryRegion, .segment = index};
            }
            if(along.size() > 1U)
            {
                return Issue{.kind = IssueKind::SharedEdge, .region = along[0], .otherRegion = along[1], .segment = index};
            }
            return std::nullopt;
        }

        std::optional<Issue> segmentIssue(const ModelCase & modelCase, const std::size_t index)
        {
            return modelCase.segments[index].region.has_value() ? namedRegionIssue(modelCase, index)
                                                                : foundRegionIssue(modelCase, index);
        }

        bool everyRegionHasEdges(const ModelCase & modelCase)
        {
            return std::ranges::all_of(modelCase.regions,
                                       [](const Region & region) { return region.points.size() >= 3U; });
        }

        void checkSegments(const ModelCase & modelCase, std::vector<Issue> & found)
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

        void checkSchedule(const ModelCase & modelCase, std::vector<Issue> & found)
        {
            const auto & lengths{modelCase.schedule.stepLengths};
            if(lengths.empty())
            {
                found.push_back(Issue{.kind = IssueKind::NoSteps});
                return;
            }
            const auto bad{std::ranges::find_if(lengths, [](const double length) { return length <= 0.0; })};
            if(bad != lengths.end())
            {
                found.push_back(Issue{.kind = IssueKind::NonPositiveStep,
                                      .step = static_cast<std::size_t>(bad - lengths.begin())});
            }
        }

        bool bindsDataset(const Segment & segment)
        {
            return segment.dataset.has_value();
        }

        //! A stated schedule beside bound datasets must be their clock; an unstated one is
        //! taken from them. Without datasets, the schedule is the clock and must be one.
        void checkClock(const ModelCase & modelCase, std::vector<Issue> & found)
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
                    found.push_back(Issue{.kind = IssueKind::ScheduleDisagrees, .segment = index});
                }
            }
        }

        //! The roles a segment's condition reads from its dataset, in the record's order.
        std::vector<SeriesRole> rolesRead(const Segment & segment)
        {
            return BCLibrary::requiredRoles(BCLibrary::BoundaryCondition{.data = recordData(segment.kind)});
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
        std::optional<ConvectionInput> missingConvectionInput(const Convective & convective)
        {
            const auto wind{windInputsOf(convective.model)};
            if(convective.model == BCLibrary::ConvectionModel::Fixed_Convection_Coefficient
               && !convective.filmCoefficient.has_value())
            {
                return ConvectionInput::FilmCoefficient;
            }
            if(wind.speed && !convective.windSpeed.has_value())
            {
                return ConvectionInput::WindSpeed;
            }
            if(wind.direction && !convective.windDirection.has_value())
            {
                return ConvectionInput::WindDirection;
            }
            return std::nullopt;
        }

        void checkConvectionInputs(const ModelCase & modelCase, std::vector<Issue> & found)
        {
            for(std::size_t index = 0U; index < modelCase.segments.size(); ++index)
            {
                const auto * convective{std::get_if<Convective>(&modelCase.segments[index].kind)};
                if(convective == nullptr)
                {
                    continue;
                }
                if(const auto missing{missingConvectionInput(*convective)})
                {
                    found.push_back(Issue{.kind = IssueKind::ConvectionInputMissing, .segment = index, .input = missing});
                }
            }
        }

        //! A steady-state archive keeps its conditions in THERM's steady-state library,
        //! which has no record for a prescribed surface state, and has no datasets to read.
        void checkSteadyStateSegments(const ModelCase & modelCase, std::vector<Issue> & found)
        {
            for(std::size_t index = 0U; index < modelCase.segments.size(); ++index)
            {
                const auto & segment{modelCase.segments[index]};
                if(std::holds_alternative<Prescribed>(segment.kind))
                {
                    found.push_back(Issue{.kind = IssueKind::PrescribedInSteadyState, .segment = index});
                }
                if(auto roles{rolesRead(segment)}; !roles.empty())
                {
                    found.push_back(Issue{.kind = IssueKind::RoleInSteadyState, .segment = index, .roles = std::move(roles)});
                }
                if(bindsDataset(segment))
                {
                    found.push_back(Issue{.kind = IssueKind::DatasetInSteadyState, .segment = index});
                }
            }
        }

        //! The issue with a segment's pairing of condition and dataset, if any: every role
        //! the condition reads needs a dataset that provides it, the pairing THERM's
        //! assignment dialog checks. The engine would otherwise leave the condition out.
        std::optional<Issue> pairingIssue(const Segment & segment, const std::size_t index)
        {
            auto roles{rolesRead(segment)};
            if(roles.empty())
            {
                return std::nullopt;
            }
            if(!bindsDataset(segment))
            {
                return Issue{.kind = IssueKind::RoleWithoutDataset, .segment = index, .roles = std::move(roles)};
            }
            auto missing{BCLibrary::missingRoles(BCLibrary::BoundaryCondition{.data = recordData(segment.kind)},
                                                 segment.dataset)};
            if(missing.empty())
            {
                return std::nullopt;
            }
            return Issue{.kind = IssueKind::RoleNotProvided, .segment = index, .roles = std::move(missing)};
        }

        void checkSegmentDatasets(const ModelCase & modelCase, std::vector<Issue> & found)
        {
            for(std::size_t index = 0U; index < modelCase.segments.size(); ++index)
            {
                if(auto issue{pairingIssue(modelCase.segments[index], index)})
                {
                    found.push_back(std::move(issue.value()));
                }
            }
        }

        //! The radiation the engine runs through a clock: fixed radiation, step by step, and
        //! a black body held at constants. An enclosure is not run at all in a transient run.
        std::optional<IssueKind> transientRadiationIssue(const Radiation & radiation)
        {
            if(std::holds_alternative<BCLibrary::AutomaticEnclosure>(radiation)
               || std::holds_alternative<BCLibrary::ManualEnclosure>(radiation))
            {
                return IssueKind::EnclosureInTransient;
            }
            if(blackBodyReadsSeries(radiation))
            {
                return IssueKind::BlackBodyFromDataset;
            }
            return std::nullopt;
        }

        void checkTransientRadiation(const ModelCase & modelCase, std::vector<Issue> & found)
        {
            for(std::size_t index = 0U; index < modelCase.segments.size(); ++index)
            {
                const auto * radiation{radiationOf(modelCase.segments[index])};
                if(radiation == nullptr)
                {
                    continue;
                }
                if(const auto kind{transientRadiationIssue(*radiation)})
                {
                    found.push_back(Issue{.kind = kind.value(), .segment = index});
                }
            }
        }

        void checkMode(const ModelCase & modelCase, std::vector<Issue> & found)
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
                found.push_back(Issue{.kind = IssueKind::UnsupportedMode});
            }
        }
        // --------------------------------------------------------------------------------
        // The default wording
        // --------------------------------------------------------------------------------

        std::string segmentLabel(const ModelCase & modelCase, const std::size_t index)
        {
            const auto & segment{modelCase.segments[index]};
            const std::string who{segment.name.has_value() ? std::format("segment {} '{}'", index, segment.name.value())
                                                           : std::format("segment {}", index)};
            return std::format(
              "{} from ({}, {}) to ({}, {})", who, segment.start.x, segment.start.y, segment.end.x, segment.end.y);
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

        std::string inputName(const ConvectionInput input)
        {
            switch(input)
            {
                case ConvectionInput::WindSpeed:
                    return "the wind speed";
                case ConvectionInput::WindDirection:
                    return "the wind direction";
                default:
                    return "a film coefficient";
            }
        }

        //! The wording of an issue that is not about one segment's condition.
        std::optional<std::string> caseText(const ModelCase & modelCase, const Issue & issue)
        {
            switch(issue.kind)
            {
                case IssueKind::NoRegions:
                    return "a model needs at least one region";
                case IssueKind::TooFewPoints:
                    return std::format("region {} has {} points; a region needs at least three",
                                       issue.region.value(),
                                       modelCase.regions[issue.region.value()].points.size());
                case IssueKind::UnnamedMaterial:
                    return std::format("region {} has a material with no name", issue.region.value());
                case IssueKind::MaterialNameClash:
                    return std::format("regions {} and {} carry two different materials both named '{}'; the "
                                       "library keeps one record per name",
                                       issue.region.value(),
                                       issue.otherRegion.value(),
                                       modelCase.regions[issue.region.value()].material.Name);
                case IssueKind::RegionMissing:
                    return std::format(
                      "segment {} borders region {}, which does not exist", issue.segment.value(), issue.region.value());
                case IssueKind::NoSteps:
                    return "the schedule needs at least one step";
                case IssueKind::NonPositiveStep:
                    return std::format("the schedule's step {} must be positive", issue.step.value() + 1U);
                case IssueKind::UnsupportedMode:
                    return "the mode must be transient or steady state";
                default:
                    return std::nullopt;
            }
        }

        //! What follows the segment's label when the segment does not lie where it should.
        std::optional<std::string> placementText(const Issue & issue)
        {
            switch(issue.kind)
            {
                case IssueKind::OffRegion:
                    return std::format(" does not lie along an edge of region {}", issue.region.value());
                case IssueKind::OffEveryRegion:
                    return " does not lie along an edge of any region";
                case IssueKind::SharedEdge:
                    return std::format(" lies on an edge shared by regions {} and {}; state which with region",
                                       issue.region.value(),
                                       issue.otherRegion.value());
                default:
                    return std::nullopt;
            }
        }

        //! What follows the segment's label when its condition and its dataset do not pair.
        std::optional<std::string> pairingText(const ModelCase & modelCase, const Issue & issue)
        {
            const auto & dataset{modelCase.segments[issue.segment.value()].dataset};
            switch(issue.kind)
            {
                case IssueKind::ScheduleDisagrees:
                    return std::format(": the stated schedule differs from the axis of its dataset '{}' ({}); leave "
                                       "the schedule out to run on the datasets' clock",
                                       dataset->Name,
                                       TimeSeriesLibrary::describeAxis(dataset.value()));
                case IssueKind::ConvectionInputMissing:
                    return std::format(" needs {} for its convection model", inputName(issue.input.value()));
                case IssueKind::RoleWithoutDataset:
                    return std::format(" reads {} from a dataset but binds none", roleNames(issue.roles));
                case IssueKind::RoleNotProvided:
                    return std::format(": its dataset '{}' provides no {}", dataset->Name, roleNames(issue.roles));
                default:
                    return std::nullopt;
            }
        }

        //! What follows the segment's label when its condition does not suit the case's mode.
        std::optional<std::string> modeText(const Issue & issue)
        {
            switch(issue.kind)
            {
                case IssueKind::EnclosureInTransient:
                    return " radiates into an enclosure, which a transient run does not model; state fixed "
                           "radiation or a black body";
                case IssueKind::BlackBodyFromDataset:
                    return " radiates as a black body read from a series, which a transient run holds constant; "
                           "state constants, or fixed-coefficient radiation to read a dataset";
                case IssueKind::PrescribedInSteadyState:
                    return " is prescribed, which a steady-state case cannot write; state it as Convective";
                case IssueKind::RoleInSteadyState:
                    return std::format(" reads {} from a dataset, which a steady-state case has none of; state it "
                                       "as a constant",
                                       roleNames(issue.roles));
                case IssueKind::DatasetInSteadyState:
                    return " binds a dataset, which a steady-state case does not write";
                default:
                    return std::nullopt;
            }
        }
    }   // namespace

    std::string describe(const ModelCase & modelCase, const Issue & issue)
    {
        return lbnl::extend(caseText(modelCase, issue))
          .or_else([&modelCase, &issue] {
              return lbnl::extend(placementText(issue))
                .or_else([&modelCase, &issue] { return pairingText(modelCase, issue); })
                .or_else([&issue] { return modeText(issue); })
                .transform([&modelCase, &issue](const std::string & tail) {
                    return segmentLabel(modelCase, issue.segment.value()) + tail;
                })
                .raw();
          })
          .raw()
          .value_or(std::string{});
    }

    std::vector<Issue> issues(const ModelCase & modelCase)
    {
        std::vector<Issue> found;
        checkRegions(modelCase, found);
        checkMaterialNames(modelCase, found);
        checkSegments(modelCase, found);
        checkConvectionInputs(modelCase, found);
        checkMode(modelCase, found);
        return found;
    }

    std::vector<std::string> describeIssues(const ModelCase & modelCase)
    {
        return lbnl::transform_to_vector(issues(modelCase),
                                         [&modelCase](const Issue & issue) { return describe(modelCase, issue); });
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
