#include "gtest/gtest.h"

#include "THMZ/SteadyStateResults/Results.hxx"
#include "THMZ/SteadyStateResults/Serializers.hxx"

#include "MockNodeAdapter.hxx"
#include "THMXSteadyStateResultsHelper.hxx"

class TestSteadyStateResults : public testing::Test
{};

TEST_F(TestSteadyStateResults, ProjectionDeserialization)
{
    auto elementNode(Helper::generateMockProjectionNode({"Projected X", "20.0", "0.5", std::nullopt}));
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::Projection projection;
    adapter >> projection;

    ThermFile::Projection correctProjection{ThermFile::UValueDimensionType::XProjected, 20.0, 0.5};
    Helper::expect_near(correctProjection, projection, 1e-6);
}

TEST_F(TestSteadyStateResults, ProjectionSerialization)
{
    ThermFile::Projection projection{ThermFile::UValueDimensionType::YProjected, 32.7, 0.65};

    Helper::MockNode node{"Projection"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << projection;

    EXPECT_TRUE(Helper::compareNodes(
      adapter.getNode(), Helper::generateMockProjectionNode({"Projected Y", "32.7", "0.65", std::nullopt})));
}

TEST_F(TestSteadyStateResults, UFactorsNodeDeserialization)
{
    auto elementNode = Helper::generateMockUFactorsNode({"Edge",
                                                         "39.0",
                                                         "23.94",
                                                         "Projected Y",
                                                         {{"Projected X", "20.0", "0.5", std::nullopt},
                                                          {"Projected Y", "32.7", "0.65", std::nullopt},
                                                          {"Total Length", "40.0", "0.75", std::nullopt}}});
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::SteadyStateUFactors result;
    adapter >> result;

    ThermFile::SteadyStateUFactors correctResult{"Edge",
                                                 39.0,
                                                 23.94,
                                                 ThermFile::UValueDimensionType::YProjected,
                                                 {{ThermFile::UValueDimensionType::XProjected, 20.0, 0.5},
                                                  {ThermFile::UValueDimensionType::YProjected, 32.7, 0.65},
                                                  {ThermFile::UValueDimensionType::TotalLength, 40.0, 0.75}}};
    Helper::expect_near(correctResult, result, 1e-6);
}

TEST_F(TestSteadyStateResults, UFactorsNodeSerialization)
{
    ThermFile::SteadyStateUFactors result{"Edge",
                                          39.0,
                                          23.94,
                                          ThermFile::UValueDimensionType::YProjected,
                                          {{ThermFile::UValueDimensionType::XProjected, 20.0, 0.5},
                                           {ThermFile::UValueDimensionType::YProjected, 32.7, 0.65},
                                           {ThermFile::UValueDimensionType::TotalLength, 40.0, 0.75}}};

    Helper::MockNode node{"UFactors"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << result;

    auto correctNodes = Helper::generateMockUFactorsNode({"Edge",
                                                          "39",
                                                          "23.94",
                                                          "Projected Y",
                                                          {{"Projected X", "20", "0.5", std::nullopt},
                                                           {"Projected Y", "32.7", "0.65", std::nullopt},
                                                           {"Total Length", "40", "0.75", std::nullopt}}});

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes));
}

TEST_F(TestSteadyStateResults, CaseDeserialization)
{
    auto elementNode =
      Helper::generateMockNodeCase({"U-factor",
                                    std::nullopt,   // No simulation error
                                    {{"Edge",
                                      "39.0",
                                      "23.94",
                                      "Projected Y",
                                      {{"Projected X", "20.0", "0.5", std::nullopt},
                                       {"Projected Y", "32.7", "0.65", std::nullopt},
                                       {"Total Length", "40.0", "0.75", std::nullopt}}},
                                     {"Frame",
                                      "45.2",
                                      "18.35",
                                      "Projected Y",
                                      {{"Projected Y", "18.5", "0.55", std::nullopt},
                                       {"Projected X", "30.2", "0.60", std::nullopt},
                                       {"Custom Rotation", "42.5", "0.80", std::nullopt}}}}});   // Modified values
    const Helper::MockNodeAdapter adapter{&elementNode};

    ThermFile::SteadyStateResultsCase result;
    adapter >> result;

    ThermFile::SteadyStateResultsCase correctResult{
      ThermFile::RunType::UFactor,
      std::nullopt,   // No simulation error
      std::nullopt,   // No glazing case
      std::nullopt,   // No spacer case
      {{
         "Edge",
         39.0,
         23.94,
         ThermFile::UValueDimensionType::YProjected,
         {{ThermFile::UValueDimensionType::XProjected, 20.0, 0.5},
          {ThermFile::UValueDimensionType::YProjected, 32.7, 0.65},
          {ThermFile::UValueDimensionType::TotalLength, 40.0, 0.75}},
       },
       {
         "Frame",
         45.2,
         18.35,
         ThermFile::UValueDimensionType::YProjected,
         {{ThermFile::UValueDimensionType::YProjected, 18.5, 0.55},
          {ThermFile::UValueDimensionType::XProjected, 30.2, 0.60},
          {ThermFile::UValueDimensionType::CustomRotationProjected, 42.5, 0.80}},
       }}};

    Helper::expect_near(correctResult, result, 1e-6);
}

TEST_F(TestSteadyStateResults, CaseSerialization)
{
    ThermFile::SteadyStateResultsCase result{
      ThermFile::RunType::UFactor,
      std::nullopt,   // No simulation error
      std::nullopt,   // No glazing case
      std::nullopt,   // No spacer case
      {{"Edge",
        39.0,
        23.94,
        ThermFile::UValueDimensionType::YProjected,
        {{ThermFile::UValueDimensionType::XProjected, 20.0, 0.5},
         {ThermFile::UValueDimensionType::YProjected, 32.7, 0.65},
         {ThermFile::UValueDimensionType::TotalLength, 40.0, 0.75}}},
       {"Frame",
        45.2,
        18.35,
        ThermFile::UValueDimensionType::YProjected,
        {{ThermFile::UValueDimensionType::YProjected, 18.5, 0.55},
         {ThermFile::UValueDimensionType::XProjected, 30.2, 0.60},
         {ThermFile::UValueDimensionType::CustomRotationProjected, 42.5, 0.80}}}}};

    Helper::MockNode node{"Case"};
    Helper::MockNodeAdapter adapter{&node};

    adapter << result;

    auto correctNodes = Helper::generateMockNodeCase({"U-factor",
                                                      std::nullopt,   // No simulation error
                                                      {{"Edge",
                                                        "39",
                                                        "23.94",
                                                        "Projected Y",
                                                        {{"Projected X", "20", "0.5", std::nullopt},
                                                         {"Projected Y", "32.7", "0.65", std::nullopt},
                                                         {"Total Length", "40", "0.75", std::nullopt}}},
                                                       {"Frame",
                                                        "45.2",
                                                        "18.35",
                                                        "Projected Y",
                                                        {{"Projected Y", "18.5", "0.55", std::nullopt},
                                                         {"Projected X", "30.2", "0.6", std::nullopt},
                                                         {"Custom Rotation", "42.5", "0.8", std::nullopt}}}}});

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes));
}

TEST_F(TestSteadyStateResults, ResultsDeserialization)
{
    Helper::MockResultsNode mockResultsNodeData("1",
                                                {{"U-factor",
                                                  std::nullopt,   // No simulation error
                                                  {{"Edge",
                                                    "36.0",
                                                    "23.94",
                                                    "Projected Y",
                                                    {{"Projected X", "22.0", "0.55", std::nullopt},
                                                     {"Projected Y", "34.7", "0.66", std::nullopt},
                                                     {"Total Length", "41.0", "0.76", std::nullopt}}},
                                                   {"Frame",
                                                    "46.2",
                                                    "18.35",
                                                    "Projected Y",
                                                    {{"Projected X", "19.5", "0.56", std::nullopt},
                                                     {"Projected Y", "31.2", "0.61", std::nullopt},
                                                     {"Custom Rotation", "43.5", "0.81", std::nullopt}}}}},
                                                 {"Condensation Resistance",
                                                  std::nullopt,   // No simulation error
                                                  {{"Edge",
                                                    "0.5",
                                                    "1.0",
                                                    "Projected Y",
                                                    {{"Projected X", "1.0", "0.1", std::nullopt},
                                                     {"Projected Y", "2.0", "0.2", std::nullopt},
                                                     {"Total Length", "3.0", "0.3", std::nullopt}}},
                                                   {"Frame",
                                                    "0.6",
                                                    "1.1",
                                                    "Projected Y",
                                                    {{"Projected X", "1.1", "0.11", std::nullopt},
                                                     {"Projected Y", "2.1", "0.21", std::nullopt},
                                                     {"Custom Rotation", "3.1", "0.31", std::nullopt}}}}}});

    auto resultsNode = Helper::generateMockResultsNode(mockResultsNodeData);

    const Helper::MockNodeAdapter adapter{&resultsNode};

    ThermFile::SteadyStateResults results;
    adapter >> results;

    // Define the correct results manually for comparison
    ThermFile::SteadyStateResults correctResults{
      "1",
      {{ThermFile::RunType::UFactor,
        std::nullopt,   // No simulation error
        std::nullopt,   // No glazing case
        std::nullopt,   // No spacer case
        {{"Edge",
          36.0,
          23.94,
          ThermFile::UValueDimensionType::YProjected,
          {{ThermFile::UValueDimensionType::XProjected, 22.0, 0.55},
           {ThermFile::UValueDimensionType::YProjected, 34.7, 0.66},
           {ThermFile::UValueDimensionType::TotalLength, 41.0, 0.76}}},
         {"Frame",
          46.2,
          18.35,
          ThermFile::UValueDimensionType::YProjected,
          {{ThermFile::UValueDimensionType::XProjected, 19.5, 0.56},
           {ThermFile::UValueDimensionType::YProjected, 31.2, 0.61},
           {ThermFile::UValueDimensionType::CustomRotationProjected, 43.5, 0.81}}}}},
       {ThermFile::RunType::CondensationResistance,
        std::nullopt,   // No simulation error
        std::nullopt,   // No glazing case
        std::nullopt,   // No spacer case
        {{"Edge",
          0.5,
          1.0,
          ThermFile::UValueDimensionType::YProjected,
          {{ThermFile::UValueDimensionType::XProjected, 1.0, 0.1},
           {ThermFile::UValueDimensionType::YProjected, 2.0, 0.2},
           {ThermFile::UValueDimensionType::TotalLength, 3.0, 0.3}}},
         {"Frame",
          0.6,
          1.1,
          ThermFile::UValueDimensionType::YProjected,
          {{ThermFile::UValueDimensionType::XProjected, 1.1, 0.11},
           {ThermFile::UValueDimensionType::YProjected, 2.1, 0.21},
           {ThermFile::UValueDimensionType::CustomRotationProjected, 3.1, 0.31}}}}}}};

    // Use your existing or similar comparison function to verify the deserialization results
    Helper::expect_near(correctResults, results, 1e-6);
}

TEST_F(TestSteadyStateResults, ResultsSerialization)
{
    // Initialize a SteadyStateResults object with the data that needs to be serialized
    ThermFile::SteadyStateResults resultsToSerialize{
      "Some custom version identifier",
      {{ThermFile::RunType::UFactor,
        std::nullopt,   // No simulation error
        std::nullopt,   // No glazing case
        std::nullopt,   // No spacer case
        {{"Edge",
          36.0,
          23.94,
          ThermFile::UValueDimensionType::YProjected,
          {{ThermFile::UValueDimensionType::XProjected, 22.0, 0.55},
           {ThermFile::UValueDimensionType::YProjected, 34.7, 0.66},
           {ThermFile::UValueDimensionType::TotalLength, 41.0, 0.76}}},
         {"Frame",
          46.2,
          18.35,
          ThermFile::UValueDimensionType::YProjected,
          {{ThermFile::UValueDimensionType::XProjected, 19.5, 0.56},
           {ThermFile::UValueDimensionType::YProjected, 31.2, 0.61},
           {ThermFile::UValueDimensionType::CustomRotationProjected, 43.5, 0.81, 23}}}}},
       {ThermFile::RunType::CondensationResistance,
        std::nullopt,   // No simulation error
        std::nullopt,   // No glazing case
        std::nullopt,   // No spacer case
        {{"Edge",
          0.5,
          1.0,
          ThermFile::UValueDimensionType::YProjected,
          {{ThermFile::UValueDimensionType::XProjected, 1.0, 0.1},
           {ThermFile::UValueDimensionType::YProjected, 2.0, 0.2},
           {ThermFile::UValueDimensionType::TotalLength, 3.0, 0.3}}},
         {"Frame",
          0.6,
          1.1,
          ThermFile::UValueDimensionType::YProjected,
          {{ThermFile::UValueDimensionType::XProjected, 1.1, 0.11},
           {ThermFile::UValueDimensionType::YProjected, 2.1, 0.21},
           {ThermFile::UValueDimensionType::CustomRotationProjected, 3.1, 0.31, 48}}}}}}};

    Helper::MockNode node{"Results"};
    Helper::MockNodeAdapter adapter{&node};
    adapter << resultsToSerialize;   // Assuming this operation serializes the object

    Helper::MockResultsNode correctMockNode{"Some custom version identifier",
                                            {{"U-factor",
                                              std::nullopt,   // No simulation error
                                              {{"Edge",
                                                "36",
                                                "23.94",
                                                "Projected Y",
                                                {{"Projected X", "22", "0.55", std::nullopt},
                                                 {"Projected Y", "34.7", "0.66", std::nullopt},
                                                 {"Total Length", "41", "0.76", std::nullopt}}},
                                               {"Frame",
                                                "46.2",
                                                "18.35",
                                                "Projected Y",
                                                {{"Projected X", "19.5", "0.56", std::nullopt},
                                                 {"Projected Y", "31.2", "0.61", std::nullopt},
                                                 {"Custom Rotation", "43.5", "0.81", "23"}}}}},
                                             {"Condensation Resistance",
                                              std::nullopt,   // No simulation error
                                              {{"Edge",
                                                "0.5",
                                                "1",
                                                "Projected Y",
                                                {{"Projected X", "1", "0.1", std::nullopt},
                                                 {"Projected Y", "2", "0.2", std::nullopt},
                                                 {"Total Length", "3", "0.3", std::nullopt}}},
                                               {"Frame",
                                                "0.6",
                                                "1.1",
                                                "Projected Y",
                                                {{"Projected X", "1.1", "0.11", std::nullopt},
                                                 {"Projected Y", "2.1", "0.21", std::nullopt},
                                                 {"Custom Rotation", "3.1", "0.31", "48"}}}}}}};

    auto correctNodes = Helper::generateMockResultsNode(correctMockNode);

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes));
}