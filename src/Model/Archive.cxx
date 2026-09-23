#include <filesystem>
#include <format>
#include <set>

#include "BoundaryConditions/Definitions.hxx"
#include "LibraryUtilities/StableUuid.hxx"
#include "THMZ/Model/DB.hxx"
#include "THMZ/ZipModule/ZipModule.hxx"
#include "TimeSeriesData/DB.hxx"
#include "TimeSeriesData/Tags.hxx"

#include "Materials/FromValues.hxx"

#include "Archive.hxx"

namespace ThermFile::Build
{
    // The vocabulary this namespace turns into a file. Using-declarations rather than a
    // using-directive: Model::Point and Model::Boundary must win over ThermFile's own.
    using Model::Adiabatic;
    using Model::Boundary;
    using Model::completed;
    using Model::Convective;
    using Model::ModelCase;
    using Model::Point;
    using Model::Prescribed;
    using Model::Schedule;
    using Model::issues;
    using Model::recordName;
    using Model::segmentColor;
    using Model::segmentRegion;
    using Model::kindColor;

    namespace
    {
        constexpr double mmPerM{1000.0};

        //! The "use the material's emissivity" marker THERM writes on a boundary.
        constexpr double globalEmissivityMarker{-1.0};

        const std::string emptyGases{"<Gases>\n\t<Version>1</Version>\n</Gases>\n"};
        const std::string emptyMesh{"<Mesh>\n\t<Version>1</Version>\n</Mesh>\n"};

        std::string joined(const std::vector<std::string> & lines)
        {
            std::string text;
            for(const auto & line : lines)
            {
                if(!text.empty())
                {
                    text += '\n';
                }
                text += line;
            }
            return text;
        }

        // --------------------------------------------------------------------------------
        // Boundary-condition records and datasets
        // --------------------------------------------------------------------------------

        BCLibrary::Source fromSeries(const TimeSeriesLibrary::SeriesRole role)
        {
            return BCLibrary::FromTimeSeries{role};
        }

        BCLibrary::PrescribedState prescribedRecord(const bool withHumidity)
        {
            BCLibrary::PrescribedState state;
            state.temperature = fromSeries(TimeSeriesLibrary::SeriesRole::PrescribedTemperature);
            if(withHumidity)
            {
                state.relativeHumidity = fromSeries(TimeSeriesLibrary::SeriesRole::PrescribedHumidity);
            }
            return state;
        }

        BCLibrary::SurfaceExchange convectiveRecord()
        {
            BCLibrary::Convection convection;
            convection.model = BCLibrary::ConvectionModel::Fixed_Convection_Coefficient;
            convection.airTemperature = fromSeries(TimeSeriesLibrary::SeriesRole::AirTemperature);
            convection.filmCoefficient = fromSeries(TimeSeriesLibrary::SeriesRole::ConvectiveCoefficient);
            BCLibrary::SurfaceExchange exchange;
            exchange.relativeHumidity = fromSeries(TimeSeriesLibrary::SeriesRole::RelativeHumidity);
            exchange.convection = convection;
            return exchange;
        }

        TimeSeriesLibrary::TimeSeriesData dataset(const ModelCase & modelCase,
                                                  const std::string & key,
                                                  const std::vector<std::pair<std::string, double>> & values)
        {
            TimeSeriesLibrary::TimeSeriesData data;
            data.UUID = caseUuid(modelCase, "dataset/" + key);
            // THERM displays project-scoped records as "project:Name", so the name itself
            // carries only the segment; the case id is the record's project name.
            data.Name = key;
            data.ProjectName = modelCase.id;
            data.Protected = false;
            data.axis.stepSeconds = modelCase.schedule.dtime;
            for(const auto & [role, value] : values)
            {
                data.series.push_back(TimeSeriesLibrary::Series{TimeSeriesLibrary::seriesRoleFromString(role),
                                                                std::vector<double>(modelCase.schedule.nSteps, value)});
            }
            return data;
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
                                     const std::optional<std::string> & datasetUuid)
        {
            const auto & segment{modelCase.segments[index]};
            ThermFile::Boundary result;
            result.ID = index + 1U;
            result.uuid = caseUuid(modelCase, std::format("segment/{}", index));
            result.name = recordName(segment.kind);
            result.bcUUID = recordUuid(result.name);
            result.timeSeriesUUID = datasetUuid;
            result.isBlocking = true;
            result.neighborPolygonUUID = polygonUuid;
            result.startPoint = pagePoint(segment.start);
            result.endPoint = pagePoint(segment.end);
            result.surfaceType = SurfaceType::BoundaryCondition;
            result.color = segmentColor(segment);
            result.status = 1;
            result.thermalEmissionProperties.emissivity = globalEmissivityMarker;
            result.thermalEmissionProperties.useGlobalEmissivity = true;
            return result;
        }

        void applyCalculationOptions(const ModelCase & modelCase, ThermModel & model)
        {
            auto & options{model.properties.calculationOptions};
            options.simulationEngine = SimulationEngine::HygroThermFEM;
            options.calculationMode = CalculationMode::cmTransient;
            options.simulateThermal = modelCase.physics.thermal;
            options.simulateMoisture = modelCase.physics.moisture;

            auto & switches{options.modelingOptions};
            const auto & physics{modelCase.physics};
            switches.excludeWaterLiquidTransportation = !physics.liquidTransport;
            switches.excludeHeatOfEvaporation = !physics.heatOfEvaporation;
            switches.excludeCapillaryConduction = !physics.capillaryConduction;
            switches.excludeVaporDiffusionConduction = !physics.vaporDiffusionConduction;
            switches.excludeLatentHeatOfFusion = !physics.latentHeatOfFusion;
            switches.excludeThermalConductivityMoistureDependent = !physics.conductivityDependent;
            switches.excludeThermalConductivityTemperatureDependent = !physics.conductivityDependent;

            options.transientCalculationMethodology = TransientCalculationMethodology::icConstantEverywhere;
            options.constantInitialConditionsTransient.temperature = modelCase.initial.temperature;
            options.constantInitialConditionsTransient.humidity = modelCase.initial.humidity;

            options.engineParameters.convergenceTolerance = modelCase.numerics.convergenceTolerance;
            options.engineParameters.relaxationParameter = modelCase.numerics.relaxation;
            options.heatTransferModelingOptions.maximumIterations =
              static_cast<std::size_t>(modelCase.numerics.maxIterations);
            // The mesh is handed in by the caller's mesher; the estimator is a steady-state tool.
            options.meshControl.runErrorEstimator = false;
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

    std::string recordUuid(const std::string_view name)
    {
        if(name == recordName(Adiabatic{}))
        {
            return std::string{adiabaticRecordUuid};
        }
        return LibraryCommon::stableUuid("boundary-condition", name);
    }

    BCLibrary::BoundaryCondition boundaryRecord(const Boundary & boundary, const std::string_view project)
    {
        const auto name{recordName(boundary)};
        BCLibrary::BoundaryCondition record;
        record.UUID = recordUuid(name);
        record.Name = name;
        record.Color = kindColor(boundary);
        if(std::holds_alternative<Adiabatic>(boundary))
        {
            // THERM's own record, as it ships: protected, owned by no project.
            record.Protected = true;
            record.data = BCLibrary::NoExchange{};
            return record;
        }
        record.ProjectName = std::string{project};
        record.Protected = false;
        if(const auto * prescribed{std::get_if<Prescribed>(&boundary)})
        {
            record.data = prescribedRecord(prescribed->humidity.has_value());
        }
        else
        {
            record.data = convectiveRecord();
        }
        return record;
    }

    std::vector<std::pair<std::string, double>> seriesValues(const Boundary & boundary)
    {
        std::vector<std::pair<std::string, double>> values;
        if(const auto * prescribed{std::get_if<Prescribed>(&boundary)})
        {
            values.emplace_back("PrescribedTemperature", prescribed->temperature);
            if(prescribed->humidity.has_value())
            {
                values.emplace_back("PrescribedHumidity", prescribed->humidity.value());
            }
        }
        else if(const auto * convective{std::get_if<Convective>(&boundary)})
        {
            values.emplace_back("AirTemperature", convective->airTemperature);
            values.emplace_back("ConvectiveCoefficient", convective->filmCoefficient);
            values.emplace_back("RelativeHumidity", convective->humidity);
        }
        return values;
    }

    std::map<std::size_t, TimeSeriesLibrary::TimeSeriesData> boundaryDatasets(const ModelCase & stated)
    {
        const auto modelCase{completed(stated)};
        std::map<std::size_t, TimeSeriesLibrary::TimeSeriesData> datasets;
        for(std::size_t index = 0U; index < modelCase.segments.size(); ++index)
        {
            const auto values{seriesValues(modelCase.segments[index].kind)};
            if(!values.empty())
            {
                datasets.emplace(index, dataset(modelCase, std::format("segment {}", index + 1U), values));
            }
        }
        if(datasets.empty())
        {
            datasets.emplace(0U, dataset(modelCase, "clock", {{"AirTemperature", modelCase.initial.temperature}}));
        }
        return datasets;
    }

    ThermModel model(const ModelCase & stated)
    {
        const auto modelCase{completed(stated)};
        const auto datasets{boundaryDatasets(modelCase)};
        ThermModel result;
        result.calculationReady = true;
        result.preferences.settings.origin = drawingOriginMm;
        result.glazingOrigin = drawingOriginMm;
        result.properties.general.fileName = modelCase.id;
        result.properties.general.title = modelCase.title.empty() ? modelCase.id : modelCase.title;
        applyCalculationOptions(modelCase, result);

        for(std::size_t index = 0U; index < modelCase.regions.size(); ++index)
        {
            result.polygons.push_back(polygon(modelCase, index));
        }
        for(std::size_t index = 0U; index < modelCase.segments.size(); ++index)
        {
            const auto & segment{modelCase.segments[index]};
            const auto bound{datasets.find(index)};
            result.boundaryConditions.push_back(
              boundary(modelCase,
                       index,
                       result.polygons[segmentRegion(modelCase, index).value()].uuid,
                       bound == datasets.end() ? std::nullopt : std::optional<std::string>{bound->second.UUID}));
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
        std::set<std::string> seen;
        for(const auto & segment : modelCase.segments)
        {
            if(seen.insert(recordName(segment.kind)).second)
            {
                records.add(boundaryRecord(segment.kind, modelCase.id));
            }
        }

        std::vector<TimeSeriesLibrary::TimeSeriesData> datasets;
        for(const auto & [index, data] : boundaryDatasets(modelCase))
        {
            datasets.push_back(data);
        }
        return Libraries{.materials = MaterialsLibrary::materialsDatabase(used),
                         .boundaryConditions = std::move(records),
                         .datasets = std::move(datasets)};
    }

    lbnl::ExpectedExt<std::map<std::string, std::string>, std::string> archiveEntries(const ModelCase & modelCase)
    {
        if(const auto found{issues(modelCase)}; !found.empty())
        {
            return lbnl::Unexpected<std::string>{joined(found)};
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

    lbnl::ExpectedExt<std::string, std::string> archive(const ModelCase & modelCase, const std::string & path)
    {
        const auto entries{archiveEntries(modelCase)};
        if(!entries.has_value())
        {
            return lbnl::Unexpected<std::string>{entries.error()};
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

    Schedule scheduleOf(const std::vector<TimeSeriesLibrary::TimeSeriesData> & datasets)
    {
        const auto & first{datasets.front()};
        return Schedule{.dtime = first.axis.stepSeconds, .nSteps = TimeSeriesLibrary::steps(first)};
    }
}   // namespace ThermFile::Build
