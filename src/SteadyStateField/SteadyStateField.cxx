#include <cmath>
#include <filesystem>
#include <format>
#include <limits>
#include <optional>
#include <stdexcept>
#include <unordered_map>

#include <lbnl/algorithm.hxx>

#include "THMZ/Mesh/DB.hxx"
#include "THMZ/SteadyStateMeshResults/DB.hxx"
#include "THMZ/ZipModule/ZipModule.hxx"

#include "SteadyStateField.hxx"

namespace ThermFile::SteadyStateField
{
    namespace
    {
        using Failure = lbnl::Unexpected<std::string>;

        constexpr double missing{std::numeric_limits<double>::quiet_NaN()};

        //! The first case of the run type, in a list of cases that name their run type in the
        //! given member (the mesh says modelType, the nodal results resultsType).
        template<typename CaseType>
        [[nodiscard]] std::optional<CaseType> caseOf(const std::vector<CaseType> & cases,
                                                     std::optional<RunType> CaseType::* runTypeOf,
                                                     const RunType runType)
        {
            return lbnl::find_element(cases, [&](const CaseType & item) { return item.*runTypeOf == runType; });
        }

        //! "U-factor, Condensation Resistance", or "none".
        template<typename CaseType>
        [[nodiscard]] std::string runTypesOf(const std::vector<CaseType> & cases,
                                             std::optional<RunType> CaseType::* runTypeOf)
        {
            std::string names;
            for(const auto & item : cases)
            {
                const auto name{runTypeToString((item.*runTypeOf).value_or(RunType::Unknown))};
                names += names.empty() ? name : ", " + name;
            }
            return names.empty() ? "none" : names;
        }

        template<typename CaseType>
        [[nodiscard]] lbnl::ExpectedExt<CaseType, std::string>
          requiredCase(const std::vector<CaseType> & cases,
                       std::optional<RunType> CaseType::* runTypeOf,
                       const RunType runType,
                       const std::string & section)
        {
            if(auto found{caseOf(cases, runTypeOf, runType)})
            {
                return std::move(found.value());
            }
            return Failure{std::format(
              "The {} has no {} case; it has: {}", section, runTypeToString(runType), runTypesOf(cases, runTypeOf))};
        }

        [[nodiscard]] Field emptyField(const Mesh::Case & mesh, const RunType runType)
        {
            const std::vector<double> unknown(mesh.nodes.size(), missing);
            return {.runType = runType,
                    .nodes = mesh.nodes,
                    .elements = mesh.elements,
                    .temperature = unknown,
                    .xFlux = unknown,
                    .yFlux = unknown,
                    .humidity = unknown,
                    .waterContent = unknown,
                    .iceContent = unknown};
        }

        void store(Field & field, const std::size_t position, const NodeResults & node)
        {
            field.temperature[position] = node.temperature;
            field.xFlux[position] = node.xFlux;
            field.yFlux[position] = node.yFlux;
            field.humidity[position] = node.humidity.value_or(missing);
            field.waterContent[position] = node.waterContent.value_or(missing);
            field.iceContent[position] = node.iceContent.value_or(missing);
        }

        //! The results laid onto the mesh's node order, matched on the node's label - never
        //! on list position, since neither section promises the other's order.
        [[nodiscard]] Field joined(const Mesh::Case & mesh, const CaseMeshResults & results, const RunType runType)
        {
            std::unordered_map<std::size_t, std::size_t> positionOf;
            for(std::size_t position = 0U; position < mesh.nodes.size(); ++position)
            {
                positionOf.emplace(mesh.nodes[position].index, position);
            }
            auto field{emptyField(mesh, runType)};
            for(const auto & node : results.nodes)
            {
                if(const auto found{positionOf.find(node.index)}; found != positionOf.end())
                {
                    store(field, found->second, node);
                }
            }
            return field;
        }
    }   // namespace

    std::vector<double> fluxMagnitude(const Field & field)
    {
        std::vector<double> magnitude(field.xFlux.size());
        for(std::size_t position = 0U; position < magnitude.size(); ++position)
        {
            magnitude[position] = std::hypot(field.xFlux[position], field.yFlux[position]);
        }
        return magnitude;
    }

    lbnl::ExpectedExt<Field, std::string> read(const std::map<std::string, std::string> & entries,
                                               const RunType runType)
    {
        const auto mesh{Mesh::loadMeshFromString(ThermZip::findEntry(entries, ThermZip::MeshName))};
        if(!mesh.has_value() || mesh->cases.empty())
        {
            return Failure{"The archive has no stored mesh"};
        }
        const auto results{
          loadSteadyStateMeshResultsFromString(ThermZip::findEntry(entries, ThermZip::SteadyStateMeshResultsName))};
        if(!results.has_value() || results->cases.empty())
        {
            return Failure{"The archive has no steady-state nodal results"};
        }
        const auto meshCase{requiredCase(mesh->cases, &Mesh::Case::modelType, runType, "stored mesh")};
        if(!meshCase.has_value())
        {
            return Failure{meshCase.error()};
        }
        const auto resultsCase{requiredCase(results->cases, &CaseMeshResults::resultsType, runType, "nodal results")};
        if(!resultsCase.has_value())
        {
            return Failure{resultsCase.error()};
        }
        return joined(meshCase.value(), resultsCase.value(), runType);
    }

    lbnl::ExpectedExt<Field, std::string> readFromZipFile(const std::string & zipFileName, const RunType runType)
    {
        if(!std::filesystem::exists(zipFileName))
        {
            return Failure{"No such archive: " + zipFileName};
        }
        auto names{ThermZip::entryNameCandidates(ThermZip::MeshName)};
        const auto results{ThermZip::entryNameCandidates(ThermZip::SteadyStateMeshResultsName)};
        names.insert(names.end(), results.begin(), results.end());
        try
        {
            return read(ThermZip::unzipFiles(zipFileName, names), runType);
        }
        catch(const std::runtime_error & failure)
        {
            return Failure{std::format("{}: {}", zipFileName, failure.what())};
        }
    }
}   // namespace ThermFile::SteadyStateField
