#pragma once

#include <string>
#include <optional>

namespace ThermFile
{
    struct Projection;
    struct SteadyStateUFactors;
    struct SteadyStateResultsCase;
    struct SteadyStateResults;
}   // namespace ThermFile

namespace Helper
{
    struct MockNode;

    void expect_near(const ThermFile::Projection & expected, const ThermFile::Projection & actual, double tolerance);

    void expect_near(const ThermFile::SteadyStateUFactors & expected,
                     const ThermFile::SteadyStateUFactors & actual,
                     double tolerance);

    void expect_near(const ThermFile::SteadyStateResultsCase & expected,
                     const ThermFile::SteadyStateResultsCase & actual,
                     double tolerance);

    void expect_near(const ThermFile::SteadyStateResults & expected,
                     const ThermFile::SteadyStateResults & actual,
                     double tolerance);

    struct MockProjectionNode
    {
        std::string lengthType;
        std::optional<std::string> length;
        std::optional<std::string> uValue;
        std::optional<std::string> projectionAngle;

        [[maybe_unused]] MockProjectionNode(std::string lengthType,
                                            std::optional<std::string> length,
                                            std::optional<std::string> uValue,
                                            std::optional<std::string> projectionAngle);
    };

    Helper::MockNode generateMockProjectionNode(const MockProjectionNode & projectionNode);

    struct MockUFactorsNode
    {
        std::string tag;
        std::string deltaT;
        std::string heatFlow;
        std::string defaultDisplayType;
        std::vector<MockProjectionNode> projections;

        [[maybe_unused]] MockUFactorsNode(std::string tag,
                                          std::string deltaT,
                                          std::string heatFlow,
                                          std::string defaultDisplayType,
                                          std::vector<MockProjectionNode> projections);
    };

    Helper::MockNode generateMockUFactorsNode(const MockUFactorsNode & uFactorsNode);

    struct MockNodeCase
    {
        std::string modelType;
        std::optional<std::string> simulationError;
        std::vector<MockUFactorsNode> steadyStateUFactors;

        [[maybe_unused]] MockNodeCase(std::string modelType,
                                      std::optional<std::string> simulationError,
                                      std::vector<MockUFactorsNode> steadyStateUFactors);
    };

    Helper::MockNode generateMockNodeCase(const MockNodeCase & nodeCase);

    struct MockResultsNode
    {
        std::string version;
        std::vector<MockNodeCase> cases;

        [[maybe_unused]] explicit MockResultsNode(std::string version, std::vector<MockNodeCase> cases);
    };

    Helper::MockNode generateMockResultsNode(const MockResultsNode & resultsNode);

}   // namespace Helper