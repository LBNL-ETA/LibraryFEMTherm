#include <array>
#include <charconv>
#include <format>
#include <sstream>
#include <system_error>
#include <utility>

#include "THMZ/ZipModule/ZipModule.hxx"

#include "TransientResults.hxx"

namespace ThermFile::TransientResults
{
    namespace
    {
        using Failure = lbnl::Unexpected<std::string>;

        //! Numbers of one comma-separated line, parsed in place: the flux files are the
        //! largest entries in the archive (two columns per node). A trailing '\r' from a
        //! Windows line ending is ignored. Empty on any cell that is not a number.
        std::vector<double> parseRow(const std::string & line)
        {
            std::vector<double> row;
            const char * cursor{line.data()};
            const char * const end{line.data() + line.size()};
            while(cursor < end && *cursor != '\r')
            {
                double value{0.0};
                const auto [next, errc]{std::from_chars(cursor, end, value)};
                if(errc != std::errc{})
                {
                    return {};
                }
                row.push_back(value);
                cursor = next;
                if(cursor < end && *cursor == ',')
                {
                    ++cursor;
                }
            }
            return row;
        }

        //! The data rows of a CSV, header skipped, blank lines skipped.
        std::vector<std::vector<double>> parseRows(const std::string & content)
        {
            std::vector<std::vector<double>> rows;
            std::istringstream stream(content);
            std::string line;
            std::getline(stream, line);   // header
            while(std::getline(stream, line))
            {
                auto row{parseRow(line)};
                if(!row.empty())
                {
                    rows.push_back(std::move(row));
                }
            }
            return rows;
        }

        constexpr std::size_t leadingColumns{2U};   // frame number, solve error

        struct NodalCsv
        {
            std::vector<std::size_t> frames;
            NodalSeries series;
        };

        lbnl::ExpectedExt<NodalCsv, std::string>
          parseNodal(const std::string & content, const std::size_t nodeCount, const std::string & name)
        {
            NodalCsv parsed;
            for(auto & row : parseRows(content))
            {
                if(row.size() != leadingColumns + nodeCount)
                {
                    return Failure{std::format("{}: a row has {} node columns, the geometry has {} nodes",
                                               name,
                                               row.size() >= leadingColumns ? row.size() - leadingColumns : 0U,
                                               nodeCount)};
                }
                parsed.frames.push_back(static_cast<std::size_t>(row[0]));
                parsed.series.solveError.push_back(row[1]);
                row.erase(row.begin(), row.begin() + static_cast<std::ptrdiff_t>(leadingColumns));
                parsed.series.values.push_back(std::move(row));
            }
            return parsed;
        }

        lbnl::ExpectedExt<std::vector<std::vector<FluxVector>>, std::string>
          parseFlux(const std::string & content, const std::size_t nodeCount, const std::string & name)
        {
            std::vector<std::vector<FluxVector>> frames;
            for(const auto & row : parseRows(content))
            {
                if(row.size() != 2U * nodeCount)
                {
                    return Failure{std::format("{}: a row has {} values, the geometry has {} nodes",
                                               name,
                                               row.size(),
                                               nodeCount)};
                }
                std::vector<FluxVector> flux;
                flux.reserve(nodeCount);
                for(std::size_t index = 0U; index < row.size(); index += 2U)
                {
                    flux.push_back(FluxVector{row[index], row[index + 1U]});
                }
                frames.push_back(std::move(flux));
            }
            return frames;
        }

        //! The geometry's nodes in column order, ID k + 1 at position k; fails on a gap.
        lbnl::ExpectedExt<std::vector<Node>, std::string>
          columnOrderedNodes(const GeometryLibrary::InputGeometryDataRecord & geometry)
        {
            std::vector<Node> nodes(geometry.nodes.size());
            std::vector<bool> seen(geometry.nodes.size(), false);
            for(const auto & node : geometry.nodes)
            {
                if(node.ID == 0U || node.ID > nodes.size() || seen[node.ID - 1U])
                {
                    return Failure{std::format("Geometry: node IDs are not 1..{} without gaps (found {})",
                                               nodes.size(),
                                               node.ID)};
                }
                seen[node.ID - 1U] = true;
                nodes[node.ID - 1U] = Node{node.ID, node.x, node.y};
            }
            return nodes;
        }

        std::string entryFor(const std::string & baseName)
        {
            return ThermZip::entryNameForFormat(baseName, FileParse::FileFormat::XML);
        }

        const std::string * find(const std::map<std::string, std::string> & entries, const std::string & name)
        {
            const auto found{entries.find(name)};
            return found == entries.end() ? nullptr : &found->second;
        }

        constexpr std::array<Quantity, 4U> allQuantities{
          Quantity::Temperature, Quantity::Humidity, Quantity::WaterContent, Quantity::IceContent};
    }   // namespace

    std::string entryName(const Quantity quantity)
    {
        switch(quantity)
        {
            case Quantity::Temperature:
                return ThermZip::getTemperatureFileName();
            case Quantity::Humidity:
                return ThermZip::getHumidityFileName();
            case Quantity::WaterContent:
                return ThermZip::getWaterContentFileName();
            case Quantity::IceContent:
                return ThermZip::getIceContentFileName();
        }
        return {};
    }

    std::size_t Results::nodeCount() const
    {
        return nodes.size();
    }

    std::size_t Results::frameCount() const
    {
        return frames.size();
    }

    bool Results::has(const Quantity quantity) const
    {
        return nodal.contains(quantity);
    }

    bool hasTransientResults(const std::map<std::string, std::string> & entries)
    {
        if(find(entries, entryFor(ThermZip::getGeometryFileName())) == nullptr)
        {
            return false;
        }
        for(const auto quantity : allQuantities)
        {
            if(find(entries, entryName(quantity)) != nullptr)
            {
                return true;
            }
        }
        return false;
    }

    lbnl::ExpectedExt<Results, std::string> read(const std::map<std::string, std::string> & entries)
    {
        const auto * geometryXml{find(entries, entryFor(ThermZip::getGeometryFileName()))};
        if(geometryXml == nullptr)
        {
            return Failure{"no transient results: the archive has no " + entryFor(ThermZip::getGeometryFileName())};
        }
        Results results;
        results.geometry.loadFromString(*geometryXml);
        if(results.geometry.nodes.empty())
        {
            return Failure{"transient results geometry has no nodes"};
        }
        auto nodes{columnOrderedNodes(results.geometry)};
        if(!nodes.has_value())
        {
            return Failure{nodes.error()};
        }
        results.nodes = std::move(nodes.value());
        const auto nodeCount{results.nodes.size()};

        for(const auto quantity : allQuantities)
        {
            const auto * csv{find(entries, entryName(quantity))};
            if(csv == nullptr)
            {
                continue;
            }
            auto parsed{parseNodal(*csv, nodeCount, entryName(quantity))};
            if(!parsed.has_value())
            {
                return Failure{parsed.error()};
            }
            auto & nodalCsv{parsed.value()};
            if(results.frames.empty())
            {
                results.frames = std::move(nodalCsv.frames);
            }
            else if(results.frames != nodalCsv.frames)
            {
                return Failure{entryName(quantity) + ": frame numbering differs from the other quantities"};
            }
            results.nodal.emplace(quantity, std::move(nodalCsv.series));
        }
        if(results.nodal.empty())
        {
            return Failure{"transient results folder has no nodal quantities"};
        }
        results.firstFrameIsInitial = !results.frames.empty() && results.frames.front() == 0U;

        const std::array<std::pair<std::vector<std::vector<FluxVector>> *, std::string>, 2U> fluxEntries{
          std::pair{&results.heatFlux, ThermZip::getHeatFluxFileName()},
          std::pair{&results.waterFlux, ThermZip::getWaterFluxFileName()}};
        for(const auto & [target, name] : fluxEntries)
        {
            const auto * csv{find(entries, name)};
            if(csv == nullptr)
            {
                continue;
            }
            auto flux{parseFlux(*csv, nodeCount, name)};
            if(!flux.has_value())
            {
                return Failure{flux.error()};
            }
            *target = std::move(flux.value());
        }
        return results;
    }

    lbnl::ExpectedExt<Results, std::string> readFromZipFile(const std::string & zipFileName)
    {
        return read(ThermZip::unzipFiles(zipFileName));
    }
}   // namespace ThermFile::TransientResults
