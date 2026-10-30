#include <algorithm>
#include <filesystem>
#include <format>

#include <lbnl/algorithm.hxx>

#include "BoundaryConditions/Definitions.hxx"
#include "LibraryUtilities/StableUuid.hxx"
#include "THMZ/Model/DB.hxx"
#include "THMZ/ZipModule/ZipModule.hxx"
#include "TimeSeriesData/DB.hxx"

#include "Materials/FromValues.hxx"

#include "Archive.hxx"

namespace ThermFile::Build
{
    // The vocabulary this namespace turns into a file. Using-declarations rather than a
    // using-directive: Model::Point and Model::Boundary must win over ThermFile's own.
    using Model::Adiabatic;
    using Model::Boundary;
    using Model::completed;
    using Model::isSteadyState;
    using Model::ModelCase;
    using Model::Point;
    using Model::RecordData;
    using Model::Schedule;
    using Model::issues;
    using Model::segmentColor;
    using Model::segmentRegion;
    using Model::kindColor;
    using TimeSeriesLibrary::Series;
    using TimeSeriesLibrary::SeriesRole;
    using TimeSeriesLibrary::TimeSeriesData;

    namespace
    {
        constexpr double mmPerM{1000.0};

        //! THERM's surface emissivity for a material that states none.
        constexpr double defaultEmissivity{0.9};

        //! The emissivity a boundary carries: the infrared emissivity of the material it
        //! borders, as THERM resolves "use the material's emissivity" when it saves -- the
        //! solver reads the stored value, not the flag. THERM's default when the material
        //! states none.
        double surfaceEmissivity(const MaterialsLibrary::Material & material)
        {
            const auto & optical{material.data.optical};
            if(optical.has_value() && optical->integrated.has_value()
               && optical->integrated->IR.EmissivityFront.has_value())
            {
                return optical->integrated->IR.EmissivityFront.value();
            }
            return defaultEmissivity;
        }

        const std::string emptyGases{"<Gases>\n\t<Version>1</Version>\n</Gases>\n"};
        const std::string emptyMesh{"<Mesh>\n\t<Version>1</Version>\n</Mesh>\n"};

        // --------------------------------------------------------------------------------
        // Records and datasets per segment
        // --------------------------------------------------------------------------------

        //! THERM's own record, as it ships: protected, owned by no project.
        BCLibrary::BoundaryCondition adiabaticRecord()
        {
            return BCLibrary::BoundaryCondition{.UUID = std::string{adiabaticRecordUuid},
                                                .Name = "Adiabatic",
                                                .Protected = true,
                                                .Color = kindColor(Adiabatic{}),
                                                .data = BCLibrary::NoExchange{}};
        }

        bool hasName(const std::vector<BCLibrary::BoundaryCondition> & records, const std::string & name)
        {
            return std::ranges::any_of(records, [&name](const auto & record) { return record.Name == name; });
        }

        //! `base` kept apart from the names already taken by a record of different
        //! content: "outside", "outside (2)", ...
        std::string uniqueRecordName(const std::string & base, const std::vector<BCLibrary::BoundaryCondition> & records)
        {
            std::string name{base};
            for(std::size_t suffix = 2U; hasName(records, name); ++suffix)
            {
                name = std::format("{} ({})", base, suffix);
            }
            return name;
        }

        //! A record of the project's own for a lowered condition, under the name and colour
        //! of what states it.
        BCLibrary::BoundaryCondition newRecord(const ModelCase & modelCase,
                                               const std::string & base,
                                               const std::string & color,
                                               const RecordData & data,
                                               const std::vector<BCLibrary::BoundaryCondition> & records)
        {
            const auto name{uniqueRecordName(base, records)};
            return BCLibrary::BoundaryCondition{.UUID = caseUuid(modelCase, "record/" + name),
                                                .Name = name,
                                                .Protected = false,
                                                .Color = color,
                                                .data = data,
                                                .ProjectName = modelCase.id};
        }

        //! The record a condition lowers to: THERM's Adiabatic for no exchange, the record
        //! of the same content already in `records`, or a new one appended to them under
        //! `base` and `color`.
        BCLibrary::BoundaryCondition recordFor(const ModelCase & modelCase,
                                               const Boundary & boundary,
                                               const std::string & base,
                                               const std::string & color,
                                               std::vector<BCLibrary::BoundaryCondition> & records)
        {
            const auto data{Model::recordData(boundary)};
            if(std::holds_alternative<BCLibrary::NoExchange>(data))
            {
                return adiabaticRecord();
            }
            const auto shared{lbnl::find_element(records, [&data](const auto & record) { return record.data == data; })};
            if(!shared.has_value())
            {
                records.push_back(newRecord(modelCase, base, color, data, records));
            }
            return shared.value_or(records.back());
        }

        //! The record of every segment of the completed case: segments whose conditions
        //! lower to the same data share the record of the first of them.
        std::vector<BCLibrary::BoundaryCondition> recordPerSegment(const ModelCase & modelCase)
        {
            std::vector<BCLibrary::BoundaryCondition> records;
            std::vector<BCLibrary::BoundaryCondition> perSegment;
            for(std::size_t index = 0U; index < modelCase.segments.size(); ++index)
            {
                const auto & segment{modelCase.segments[index]};
                perSegment.push_back(recordFor(modelCase,
                                               segment.kind,
                                               segment.name.value_or(std::format("segment {}", index + 1U)),
                                               segmentColor(segment),
                                               records));
            }
            return perSegment;
        }

        //! A face held under another record while the start is solved: the segment and the
        //! record of the condition swapped in.
        struct StartRecord
        {
            std::size_t segment{0U};
            BCLibrary::BoundaryCondition record{};
        };

        //! The records of a solved start's conditions, named "<face> at start" and shared
        //! with the segment records of the same content: one entry per segment the start
        //! names, in segment order. `records` is every record the segments use, appended to.
        std::vector<StartRecord> startRecords(const ModelCase & modelCase,
                                              std::vector<BCLibrary::BoundaryCondition> & records)
        {
            std::vector<StartRecord> found;
            const auto * conditions{Model::startConditions(modelCase.initial)};
            if(conditions == nullptr)
            {
                return found;
            }
            for(std::size_t index = 0U; index < modelCase.segments.size(); ++index)
            {
                const auto & name{modelCase.segments[index].name};
                if(!name.has_value())
                {
                    continue;
                }
                const auto condition{conditions->find(name.value())};
                if(condition == conditions->end())
                {
                    continue;
                }
                found.push_back(StartRecord{
                  .segment = index,
                  .record = recordFor(
                    modelCase, condition->second, name.value() + " at start", kindColor(condition->second), records)});
            }
            return found;
        }

        //! Every distinct record, by UUID, in the order first used.
        std::vector<BCLibrary::BoundaryCondition>
          distinctRecords(const std::vector<BCLibrary::BoundaryCondition> & records)
        {
            std::vector<BCLibrary::BoundaryCondition> distinct;
            for(const auto & record : records)
            {
                if(!std::ranges::any_of(distinct, [&record](const auto & known) { return known.UUID == record.UUID; }))
                {
                    distinct.push_back(record);
                }
            }
            return distinct;
        }

        //! The clock of a model in which no segment binds a dataset: the start temperature
        //! on every step, under a role the Adiabatic record ignores.
        TimeSeriesData clockDataset(const ModelCase & modelCase)
        {
            return TimeSeriesData{
              .UUID = caseUuid(modelCase, "dataset/clock"),
              .Name = "clock",
              .ProjectName = modelCase.id,
              .Protected = false,
              .axis = Model::axisOf(modelCase.schedule),
              .series = {Series{.role = SeriesRole::AirTemperature,
                                .values = std::vector<double>(Model::nSteps(modelCase.schedule),
                                                              Model::initialValues(modelCase.initial).temperature)}}};
        }

        //! The dataset of every segment of the completed case: the one it binds, as given;
        //! the clock on the first segment when none binds one; nothing for a steady-state
        //! case.
        std::vector<std::optional<TimeSeriesData>> datasetPerSegment(const ModelCase & modelCase)
        {
            if(isSteadyState(modelCase))
            {
                return std::vector<std::optional<TimeSeriesData>>(modelCase.segments.size());
            }
            auto perSegment{lbnl::transform_to_vector(modelCase.segments,
                                                      [](const Model::Segment & segment) { return segment.dataset; })};
            const auto bound{[](const std::optional<TimeSeriesData> & dataset) { return dataset.has_value(); }};
            if(!perSegment.empty() && std::ranges::none_of(perSegment, bound))
            {
                perSegment.front() = clockDataset(modelCase);
            }
            return perSegment;
        }

        // --------------------------------------------------------------------------------
        // The model
        // --------------------------------------------------------------------------------

        //! Case metres to THERM page millimetres, offset by the (cosmetic) drawing origin.
        ThermFile::Point pagePoint(const Point & metres)
        {
            return ThermFile::Point{drawingOriginMm.x + metres.x * mmPerM, drawingOriginMm.y + metres.y * mmPerM};
        }

        Polygon polygon(const ModelCase & modelCase, const std::size_t index)
        {
            const auto & region{modelCase.regions[index]};
            const auto & material{region.material};
            Polygon result;
            result.uuid = caseUuid(modelCase, std::format("region/{}", index));
            result.ID = static_cast<int>(index) + 1;
            result.materialUUID = material.UUID;
            result.materialName = material.Name;
            result.polygonType = PolygonType::Material;
            for(const auto & corner : region.points)
            {
                result.points.push_back(pagePoint(corner));
            }
            return result;
        }

        ThermFile::Boundary boundary(const ModelCase & modelCase,
                                     const std::size_t index,
                                     const std::string & polygonUuid,
                                     const BCLibrary::BoundaryCondition & record,
                                     const std::optional<std::string> & datasetUuid)
        {
            const auto & segment{modelCase.segments[index]};
            ThermFile::Boundary result;
            result.ID = index + 1U;
            result.uuid = caseUuid(modelCase, std::format("segment/{}", index));
            result.name = record.Name;
            result.bcUUID = record.UUID;
            result.fluxTag = segment.uFactorTag.value_or(std::string{});
            result.timeSeriesUUID = datasetUuid;
            result.isBlocking = true;
            result.neighborPolygonUUID = polygonUuid;
            result.startPoint = pagePoint(segment.start);
            result.endPoint = pagePoint(segment.end);
            result.surfaceType = SurfaceType::BoundaryCondition;
            result.color = segmentColor(segment);
            result.status = 1;
            result.thermalEmissionProperties.emissivity =
              surfaceEmissivity(modelCase.regions[segmentRegion(modelCase, index).value()].material);
            result.thermalEmissionProperties.useGlobalEmissivity = true;
            return result;
        }

        void applyPhysics(const Model::Physics & physics, CalculationOptions & options)
        {
            options.simulateThermal = physics.thermal;
            options.simulateMoisture = physics.moisture;

            auto & switches{options.modelingOptions};
            switches.excludeWaterLiquidTransportation = !physics.liquidTransport;
            switches.excludeHeatOfEvaporation = !physics.heatOfEvaporation;
            switches.excludeCapillaryConduction = !physics.capillaryConduction;
            switches.excludeVaporDiffusionConduction = !physics.vaporDiffusionConduction;
            switches.excludeLatentHeatOfFusion = !physics.latentHeatOfFusion;
            switches.excludeThermalConductivityMoistureDependent = !physics.conductivityDependent;
            switches.excludeThermalConductivityTemperatureDependent = !physics.conductivityDependent;
        }

        UniformStart fileStart(const Model::Uniform & start)
        {
            return UniformStart{.temperature = start.temperature, .humidity = start.humidity};
        }

        std::vector<StartCondition> fileStartConditions(const std::vector<StartRecord> & starts)
        {
            return lbnl::transform_to_vector(starts, [](const StartRecord & start) {
                return StartCondition{.segment = start.segment, .bcUUID = start.record.UUID};
            });
        }

        //! The case's start as the file states it: the uniform values, or the start solved
        //! first with its conditions as segment index and record.
        InitialConditions fileInitialConditions(const ModelCase & modelCase, const std::vector<StartRecord> & starts)
        {
            return std::visit(
              [&starts](const auto & start) -> InitialConditions {
                  using Start = std::decay_t<decltype(start)>;
                  if constexpr(std::is_same_v<Start, Model::Uniform>)
                  {
                      return InitialConditions{fileStart(start)};
                  }
                  else if constexpr(std::is_same_v<Start, Model::FromSteadyState>)
                  {
                      return InitialConditions{SteadyStart{
                        .conditions = fileStartConditions(starts),
                        .start = start.start.has_value() ? std::optional{fileStart(start.start.value())} : std::nullopt}};
                  }
                  else
                  {
                      return InitialConditions{TransientStart{.conditions = fileStartConditions(starts),
                                                              .stepLengths = start.schedule.stepLengths,
                                                              .start = fileStart(start.start)}};
                  }
              },
              modelCase.initial);
        }

        //! A transient case runs no estimator: the estimator is a steady-state tool. A
        //! steady-state case writes its mesh control, which is what drives the estimator.
        //! Both write their start.
        void applyMode(const ModelCase & modelCase, const std::vector<StartRecord> & starts, CalculationOptions & options)
        {
            options.calculationMode = modelCase.mode;
            options.initialConditions = fileInitialConditions(modelCase, starts);
            if(isSteadyState(modelCase))
            {
                options.meshControl = modelCase.meshControl;
                return;
            }
            options.meshControl.runErrorEstimator = false;
        }

        void applyCalculationOptions(const ModelCase & modelCase, const std::vector<StartRecord> & starts, ThermModel & model)
        {
            auto & options{model.properties.calculationOptions};
            options.simulationEngine = SimulationEngine::HygroThermFEM;
            applyPhysics(modelCase.physics, options);
            applyMode(modelCase, starts, options);

            options.engineParameters.convergenceTolerance = modelCase.numerics.convergenceTolerance;
            options.engineParameters.relaxationParameter = modelCase.numerics.relaxation;
            options.heatTransferModelingOptions.maximumIterations =
              static_cast<std::size_t>(modelCase.numerics.maxIterations);
        }

        std::pair<std::string, std::string> datasetEntry(const TimeSeriesLibrary::TimeSeriesData & data)
        {
            TimeSeriesLibrary::DB library;
            library.add(data);
            return {ThermZip::timeSeriesEntryName(data.UUID), library.saveToString()};
        }
    }   // namespace

    std::string caseUuid(const ModelCase & modelCase, const std::string_view key)
    {
        return LibraryCommon::stableUuid(modelCase.id, key);
    }

    std::vector<BCLibrary::BoundaryCondition> segmentRecords(const ModelCase & stated)
    {
        const auto modelCase{completed(stated)};
        auto records{recordPerSegment(modelCase)};
        auto pool{distinctRecords(records)};
        std::erase_if(pool, [](const auto & record) { return record.UUID == adiabaticRecordUuid; });
        for(const auto & start : startRecords(modelCase, pool))
        {
            records.push_back(start.record);
        }
        return distinctRecords(records);
    }

    std::vector<Binding> segmentBindings(const ModelCase & stated)
    {
        const auto modelCase{completed(stated)};
        const auto records{recordPerSegment(modelCase)};
        const auto datasets{datasetPerSegment(modelCase)};
        std::vector<Binding> bindings;
        for(std::size_t index = 0U; index < records.size(); ++index)
        {
            bindings.push_back(Binding{.recordUuid = records[index].UUID,
                                       .datasetUuid = datasets[index].has_value()
                                                        ? std::optional{datasets[index]->UUID}
                                                        : std::nullopt});
        }
        return bindings;
    }

    std::map<std::size_t, TimeSeriesData> boundaryDatasets(const ModelCase & stated)
    {
        const auto modelCase{completed(stated)};
        const auto datasets{datasetPerSegment(modelCase)};
        std::map<std::size_t, TimeSeriesData> found;
        for(std::size_t index = 0U; index < datasets.size(); ++index)
        {
            if(datasets[index].has_value())
            {
                found.emplace(index, datasets[index].value());
            }
        }
        return found;
    }

    ThermModel model(const ModelCase & stated)
    {
        const auto modelCase{completed(stated)};
        const auto records{recordPerSegment(modelCase)};
        const auto datasets{datasetPerSegment(modelCase)};
        auto pool{distinctRecords(records)};
        std::erase_if(pool, [](const auto & record) { return record.UUID == adiabaticRecordUuid; });
        const auto starts{startRecords(modelCase, pool)};
        ThermModel result;
        result.calculationReady = true;
        result.preferences.settings.origin = drawingOriginMm;
        result.glazingOrigin = drawingOriginMm;
        result.properties.general.fileName = modelCase.id;
        result.properties.general.title = modelCase.title.empty() ? modelCase.id : modelCase.title;
        applyCalculationOptions(modelCase, starts, result);

        for(std::size_t index = 0U; index < modelCase.regions.size(); ++index)
        {
            result.polygons.push_back(polygon(modelCase, index));
        }
        for(std::size_t index = 0U; index < modelCase.segments.size(); ++index)
        {
            result.boundaryConditions.push_back(
              boundary(modelCase,
                       index,
                       result.polygons[segmentRegion(modelCase, index).value()].uuid,
                       records[index],
                       datasets[index].has_value() ? std::optional{datasets[index]->UUID} : std::nullopt));
        }
        return result;
    }

    Libraries libraries(const ModelCase & stated)
    {
        const auto modelCase{completed(stated)};
        std::vector<MaterialsLibrary::Material> used;
        for(const auto & region : modelCase.regions)
        {
            used.push_back(region.material);
        }

        BCLibrary::DB records;
        for(const auto & record : segmentRecords(modelCase))
        {
            records.add(record);
        }

        std::vector<TimeSeriesData> datasets;
        for(const auto & [index, data] : boundaryDatasets(modelCase))
        {
            if(!std::ranges::any_of(datasets, [&data](const auto & known) { return known.UUID == data.UUID; }))
            {
                datasets.push_back(data);
            }
        }
        return Libraries{.materials = MaterialsLibrary::materialsDatabase(used),
                         .boundaryConditions = std::move(records),
                         .datasets = std::move(datasets)};
    }

    lbnl::ExpectedExt<std::map<std::string, std::string>, std::vector<Model::Issue>>
      thmzEntries(const ModelCase & modelCase)
    {
        if(auto found{issues(modelCase)}; !found.empty())
        {
            return lbnl::Unexpected<std::vector<Model::Issue>>{std::move(found)};
        }
        const auto thermModel{model(modelCase)};
        const auto built{libraries(modelCase)};
        const auto xml{FileParse::FileFormat::XML};
        std::map<std::string, std::string> entries{
          {ThermZip::entryNameForFormat(ThermZip::ModelFileName, xml), saveToString(thermModel)},
          {ThermZip::entryNameForFormat(ThermZip::MaterialsFileName, xml), built.materials.saveToString()},
          {ThermZip::entryNameForFormat(ThermZip::BoundaryConditionsFileName, xml),
           built.boundaryConditions.saveToString()},
          {ThermZip::entryNameForFormat(ThermZip::GasesFileName, xml), emptyGases},
          {ThermZip::entryNameForFormat(ThermZip::MeshName, xml), emptyMesh},
        };
        for(const auto & data : built.datasets)
        {
            entries.insert(datasetEntry(data));
        }
        return entries;
    }

    lbnl::ExpectedExt<std::string, std::vector<Model::Issue>> thmz(const ModelCase & modelCase, const std::string & path)
    {
        const auto entries{thmzEntries(modelCase)};
        if(!entries.has_value())
        {
            return lbnl::Unexpected<std::vector<Model::Issue>>{entries.error()};
        }
        const std::filesystem::path target{path};
        if(target.has_parent_path())
        {
            std::filesystem::create_directories(target.parent_path());
        }
        std::filesystem::remove(target);
        ThermZip::zipFiles(entries.value(), path);
        return path;
    }

    Schedule scheduleOf(const std::vector<TimeSeriesData> & datasets)
    {
        return Model::scheduleFor(datasets.front());
    }
}   // namespace ThermFile::Build
