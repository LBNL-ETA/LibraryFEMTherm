#include <gtest/gtest.h>

#include "THMXSteadyStateResultsHelper.hxx"

#include "THMZ/SteadyStateResults/Results.hxx"
#include "MockNodeAdapter.hxx"

namespace Helper
{
    void expect_near(const ThermFile::Projection & expected, const ThermFile::Projection & actual, double tolerance)
    {
        EXPECT_EQ(expected.lengthType, actual.lengthType);
        EXPECT_EQ(expected.length.has_value(), actual.length.has_value());
        if(expected.length.has_value() && actual.length.has_value())
        {
            EXPECT_NEAR(expected.length.value(), actual.length.value(), tolerance);
        }
        EXPECT_EQ(expected.uFactor.has_value(), actual.uFactor.has_value());
        if(expected.uFactor.has_value() && actual.uFactor.has_value())
        {
            EXPECT_NEAR(expected.uFactor.value(), actual.uFactor.value(), tolerance);
        }
        EXPECT_EQ(expected.projectionAngle.has_value(), actual.projectionAngle.has_value());
        if(expected.projectionAngle.has_value() && actual.projectionAngle.has_value())
        {
            EXPECT_NEAR(expected.projectionAngle.value(), actual.projectionAngle.value(), tolerance);
        }
    }

    void expect_near(const ThermFile::SteadyStateUFactors & expected,
                     const ThermFile::SteadyStateUFactors & actual,
                     double tolerance)
    {
        EXPECT_EQ(expected.tag, actual.tag);
        EXPECT_NEAR(expected.deltaT, actual.deltaT, tolerance);
        ASSERT_EQ(expected.projections.size(), actual.projections.size());
        for(size_t i = 0; i < expected.projections.size(); ++i)
        {
            expect_near(expected.projections[i], actual.projections[i], tolerance);
        }
    }

    void expect_near(const ThermFile::SteadyStateResultsCase & expected,
                     const ThermFile::SteadyStateResultsCase & actual,
                     double tolerance)
    {
        EXPECT_EQ(expected.modelType, actual.modelType);
        EXPECT_EQ(expected.simulationError.has_value(), actual.simulationError.has_value());
        if(expected.simulationError.has_value() && actual.simulationError.has_value())
        {
            EXPECT_NEAR(expected.simulationError.value(), actual.simulationError.value(), tolerance);
        }
        ASSERT_EQ(expected.steadyStateUFactors.size(), actual.steadyStateUFactors.size());
        for(size_t i = 0; i < expected.steadyStateUFactors.size(); ++i)
        {
            expect_near(expected.steadyStateUFactors[i], actual.steadyStateUFactors[i], tolerance);
        }
    }

    void expect_near(const ThermFile::SteadyStateResults & expected,
                     const ThermFile::SteadyStateResults & actual,
                     double tolerance)
    {
        ASSERT_EQ(expected.cases.size(), actual.cases.size());
        for(size_t i = 0; i < expected.cases.size(); ++i)
        {
            expect_near(expected.cases[i], actual.cases[i], tolerance);
        }
    }

    Helper::MockProjectionNode::MockProjectionNode(std::string lengthType,
                                                   std::optional<std::string> length,
                                                   std::optional<std::string> uValue,
                                                   std::optional<std::string> projectionAngle) :
        lengthType{std::move(lengthType)},
        length{std::move(length)},
        uValue{std::move(uValue)},
        projectionAngle{std::move(projectionAngle)}

    {}

    Helper::MockNode generateMockProjectionNode(const MockProjectionNode & projectionNode)
    {
        Helper::MockNode node{"Projection"};
        addChildNode(node, "Length-type", projectionNode.lengthType);
        if(projectionNode.length)
        {
            addChildNode(node, "Length", *projectionNode.length);
        }
        if(projectionNode.uValue)
        {
            addChildNode(node, "U-factor", *projectionNode.uValue);
        }
        if(projectionNode.projectionAngle)
        {
            addChildNode(node, "ProjectionAngle", *projectionNode.projectionAngle);
        }

        return node;
    }

    MockUFactorsNode::MockUFactorsNode(std::string tag,
                                       std::string deltaT,
                                       std::string heatFlow,
                                       std::string defaultDisplayType,
                                       std::vector<MockProjectionNode> projections) :
        tag{std::move(tag)},
        deltaT{std::move(deltaT)},
        heatFlow{std::move(heatFlow)},
        defaultDisplayType{std::move(defaultDisplayType)},
        projections{std::move(projections)}
    {}

    Helper::MockNode generateMockUFactorsNode(const MockUFactorsNode & uFactorsNode)
    {
        Helper::MockNode node{"UFactors"};
        addChildNode(node, "Tag", uFactorsNode.tag);
        addChildNode(node, "DeltaT", uFactorsNode.deltaT);
        addChildNode(node, "HeatFlux", uFactorsNode.heatFlow);
        addChildNode(node, "DefaultDisplayType", uFactorsNode.defaultDisplayType);

        for(const auto & projection : uFactorsNode.projections)
        {
            addChildNode(node, "Projection", generateMockProjectionNode(projection));
        }

        return node;
    }

    MockNodeCase::MockNodeCase(std::string modelType,
                               std::optional<std::string> simulationError,
                               std::vector<MockUFactorsNode> steadyStateUFactors) :
        modelType{std::move(modelType)},
        simulationError{std::move(simulationError)},
        steadyStateUFactors{std::move(steadyStateUFactors)}
    {}

    Helper::MockNode generateMockNodeCase(const MockNodeCase & nodeCase)
    {
        Helper::MockNode node{"Case"};
        addChildNode(node, "ModelType", nodeCase.modelType);
        if(nodeCase.simulationError)
        {
            addChildNode(node, "SimulationError", *nodeCase.simulationError);
        }

        for(const auto & uFactors : nodeCase.steadyStateUFactors)
        {
            addChildNode(node, "U-factors", generateMockUFactorsNode(uFactors));
        }

        return node;
    }

    MockResultsNode::MockResultsNode(std::string version, std::vector<MockNodeCase> cases) :
        version{std::move(version)}, cases{std::move(cases)}
    {}

    Helper::MockNode generateMockResultsNode(const MockResultsNode & resultsNode)
    {
        Helper::MockNode node{"Results"};

        addChildNode(node, "Version", resultsNode.version);

        for(const auto & nodeCase : resultsNode.cases)
        {
            addChildNode(node, "Case", generateMockNodeCase(nodeCase));
        }

        return node;
    }
}   // namespace Helper