#include <algorithm>
#include <gtest/gtest.h>

#include "THMZ/CMA/CMA.hxx"
#include "THMZ/CMA/Serializers.hxx"

#include "MockNodeAdapter.hxx"
#include "Utility.hxx"
#include "HelperUtility.hxx"

class TestThermFileCMA : public testing::Test
{};

namespace
{
    Helper::MockNode &
      insertPositionNode(Helper::MockNode & parent, const std::string & value, const std::string & uuid)
    {
        Helper::MockNode & positionNode = Helper::addChildNode(parent, "Position");
        addChildNode(positionNode, "Value", value);
        addChildNode(positionNode, "UUID", uuid);
        return parent;
    }

    Helper::MockNode & insertCaseNode(Helper::MockNode & parent,
                                      const std::string & glazingOption,
                                      const std::string & spacerOption,
                                      const std::string & gapConductance,
                                      const std::string & spacerConductivity)
    {
        Helper::MockNode & caseNode = Helper::addChildNode(parent, "Case");
        addChildNode(caseNode, "GlazingOption", glazingOption);
        addChildNode(caseNode, "SpacerOption", spacerOption);
        addChildNode(caseNode, "GapConductance", gapConductance);
        addChildNode(caseNode, "SpacerConductivity", spacerConductivity);
        return parent;
    }

    struct LayerInput
    {
        std::string glazingOption;
        std::string spacerOption;
        std::string hcin;
        std::string hcout;
        std::string emissivityIn;
        std::string emissivityOut;
    };

    Helper::MockNode & insertLayerOption(Helper::MockNode & parent, const LayerInput & input)
    {
        Helper::MockNode & optionNode = Helper::addChildNode(parent, "Option");
        addChildNode(optionNode, "Glazing", input.glazingOption);
        addChildNode(optionNode, "Spacer", input.spacerOption);
        addChildNode(optionNode, "hcin", input.hcin);
        addChildNode(optionNode, "hcout", input.hcout);
        addChildNode(optionNode, "EmissivityIn", input.emissivityIn);
        addChildNode(optionNode, "EmissivityOut", input.emissivityOut);
        return parent;
    }

    Helper::MockNode & insertSingleLayer(Helper::MockNode & parent,
                                         const std::string & thickness,
                                         const std::string & conductivity,
                                         const std::vector<LayerInput> & options)
    {
        Helper::MockNode & singleLayerNode = Helper::addChildNode(parent, "SingleLayer");
        addChildNode(singleLayerNode, "Thickness", thickness);
        addChildNode(singleLayerNode, "Conductivity", conductivity);

        std::for_each(
          begin(options), end(options), [&](const LayerInput & option) { insertLayerOption(singleLayerNode, option); });

        return parent;
    }

    void expectBestWorstValuesNear(const ThermFile::CMAGapSpacer & correct,
                                   const ThermFile::CMAGapSpacer & actual,
                                   const double tolerance)
    {
        EXPECT_EQ(static_cast<int>(correct.glazingCase), static_cast<int>(actual.glazingCase));
        EXPECT_EQ(static_cast<int>(correct.spacerCase), static_cast<int>(actual.spacerCase));
        EXPECT_NEAR(correct.gapConductance, actual.gapConductance, tolerance);
        EXPECT_NEAR(correct.spacerConductivity, actual.spacerConductivity, tolerance);
    }

    void compareSingleLayer(const std::variant<ThermFile::CMASingleLayer, ThermFile::CMADoubleLayer> & variant,
                            const ThermFile::CMASingleLayer & correctSingleLayer,
                            const double tolerance = 1e-6)
    {
        ASSERT_TRUE(std::holds_alternative<ThermFile::CMASingleLayer>(variant));
        const auto & singleLayer = std::get<ThermFile::CMASingleLayer>(variant);

        ASSERT_NEAR(singleLayer.thickness, correctSingleLayer.thickness, tolerance);
        ASSERT_NEAR(singleLayer.conductivity, correctSingleLayer.conductivity, tolerance);

        ASSERT_EQ(singleLayer.option.size(), correctSingleLayer.option.size());
        for(size_t i = 0; i < singleLayer.option.size(); ++i)
        {
            ASSERT_EQ(static_cast<int>(singleLayer.option[i].glazingCase),
                      static_cast<int>(correctSingleLayer.option[i].glazingCase));
            ASSERT_EQ(static_cast<int>(singleLayer.option[i].spacerCase),
                      static_cast<int>(correctSingleLayer.option[i].spacerCase));
            ASSERT_NEAR(singleLayer.option[i].hcin, correctSingleLayer.option[i].hcin, tolerance);
            ASSERT_NEAR(singleLayer.option[i].hcout, correctSingleLayer.option[i].hcout, tolerance);
            ASSERT_NEAR(singleLayer.option[i].emissivityIn, correctSingleLayer.option[i].emissivityIn, tolerance);
            ASSERT_NEAR(singleLayer.option[i].emissivityOut, correctSingleLayer.option[i].emissivityOut, tolerance);
        }
    }
}   // namespace

TEST_F(TestThermFileCMA, CMABCContainerDeserialization)
{
    auto mockData = []() {
        Helper::MockNode node{"CMAData"};
        Helper::MockNode & positionsNode = Helper::addChildNode(node, "CMABCPositions");

        positionsNode = insertPositionNode(positionsNode, "Exterior", "3f50e7c8-9b49-4f72-8427-b9b8fbbd163d");
        positionsNode = insertPositionNode(positionsNode, "Interior", "4e3a1f79-c5cb-478e-8127-9c0a1ec27a93");
        positionsNode = insertPositionNode(positionsNode, "Exterior", "6a7e7bb9-8148-4a4d-939b-4b5315c5ad78");

        node = insertCaseNode(node, "Low", "Low", "0.13", "0.25");
        node = insertCaseNode(node, "Low", "High", "0.15", "0.27");
        node = insertCaseNode(node, "High", "Low", "0.17", "0.29");
        node = insertCaseNode(node, "High", "High", "0.19", "0.31");

        node =
          insertSingleLayer(node,
                            "0.1",
                            "0.2",
                            {{"Low", "Low", "0.3", "0.4", "0.5", "0.6"}, {"Low", "High", "0.7", "0.8", "0.9", "1.0"}});

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::CMAData cmaData;
    adapter >> cmaData;

    const ThermFile::CMABCContainer correctPositions{
      {ThermFile::CMABCPosition::Exterior, "3f50e7c8-9b49-4f72-8427-b9b8fbbd163d"},
      {ThermFile::CMABCPosition::Interior, "4e3a1f79-c5cb-478e-8127-9c0a1ec27a93"}};

    Helper::checkMapValuesEqual(correctPositions, cmaData.cmaBCPositions);

    const std::vector<ThermFile::CMAGapSpacer> correctBWValues{
      {ThermFile::CMACase::Low, ThermFile::CMACase::Low, 0.13, 0.25},
      {ThermFile::CMACase::Low, ThermFile::CMACase::High, 0.15, 0.27},
      {ThermFile::CMACase::High, ThermFile::CMACase::Low, 0.17, 0.29},
      {ThermFile::CMACase::High, ThermFile::CMACase::High, 0.19, 0.31}};

    // clang-format off
    const ThermFile::CMASingleLayer correctSingleLayer
        = {0.1, 0.2,
            {
                {ThermFile::CMACase::Low, ThermFile::CMACase::Low, 0.3, 0.4, 0.5, 0.6},
                {ThermFile::CMACase::Low, ThermFile::CMACase::High, 0.7, 0.8, 0.9, 1.0}
            }
          };
    // clang-format on

    ASSERT_EQ(correctBWValues.size(), cmaData.gapSpacerInput.size());
    for(size_t i = 0; i < correctBWValues.size(); ++i)
    {
        expectBestWorstValuesNear(correctBWValues[i], cmaData.gapSpacerInput[i], 1e-6);
    }

    compareSingleLayer(cmaData.layerInput, correctSingleLayer);
}

TEST_F(TestThermFileCMA, CMABCContainerSerialization)
{
    ThermFile::CMAData cmaData;
    cmaData.interiorTemperature = 20.0;
    cmaData.exteriorTemperature = -10.0;
    cmaData.cmaBCPositions = {{ThermFile::CMABCPosition::Exterior, "3f50e7c8-9b49-4f72-8427-b9b8fbbd163d"},
                              {ThermFile::CMABCPosition::Interior, "4e3a1f79-c5cb-478e-8127-9c0a1ec27a93"}};
    cmaData.gapSpacerInput = {{ThermFile::CMACase::Low, ThermFile::CMACase::Low, 0.13, 0.25},
                              {ThermFile::CMACase::Low, ThermFile::CMACase::High, 0.15, 0.27},
                              {ThermFile::CMACase::High, ThermFile::CMACase::Low, 0.17, 0.29},
                              {ThermFile::CMACase::High, ThermFile::CMACase::High, 0.19, 0.31}};
    // clang-format off
    ThermFile::CMASingleLayer singleLayer{
    0.1, 0.2,
        {
            {ThermFile::CMACase::Low, ThermFile::CMACase::Low, 0.3, 0.4, 0.5, 0.6},
            {ThermFile::CMACase::Low, ThermFile::CMACase::High, 0.7, 0.8, 0.9, 1.0}
        }
    };
    // clang-format on

    cmaData.layerInput = singleLayer;

    Helper::MockNode elementNode("CMAData");
    Helper::MockNodeAdapter adapter{&elementNode};
    adapter << cmaData;

    auto correctNodes = []() {
        Helper::MockNode node{"CMAData"};
        Helper::addChildNode(node, "IGUType", "Unknown");
        Helper::addChildNode(node, "InteriorTemperature", "20");
        Helper::addChildNode(node, "ExteriorTemperature", "-10");

        node = insertCaseNode(node, "Low", "Low", "0.13", "0.25");
        node = insertCaseNode(node, "Low", "High", "0.15", "0.27");
        node = insertCaseNode(node, "High", "Low", "0.17", "0.29");
        node = insertCaseNode(node, "High", "High", "0.19", "0.31");

        node = insertSingleLayer(node,
                                 "0.1",
                                 "0.2",
                                 {{"Low", "Low", "0.3", "0.4", "0.5", "0.6"},
                                  {"Low", "High", "0.7", "0.8", "0.9", "1"}});

        Helper::MockNode & positionsNode = Helper::addChildNode(node, "CMABCPositions");

        positionsNode = insertPositionNode(positionsNode, "Interior", "4e3a1f79-c5cb-478e-8127-9c0a1ec27a93");
        positionsNode = insertPositionNode(positionsNode, "Exterior", "3f50e7c8-9b49-4f72-8427-b9b8fbbd163d");

        return node;
    };

    auto test{correctNodes()};

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}
