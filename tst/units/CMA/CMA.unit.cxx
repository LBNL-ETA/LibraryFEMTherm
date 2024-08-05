#include "gtest/gtest.h"

#include "MockNodeAdapter.hxx"
#include "CMA/Serializers.hxx"
#include "CMA/Definitions.hxx"

/////////////////////////////////////////
///  Helper functions for serialization
/////////////////////////////////////////

Helper::MockNode createEnvironmentNode()
{
    Helper::MockNode node{"Environment"};
    addChildNode(node, "Tin", "25.5");
    addChildNode(node, "Tout", "-14.12");
    return node;
}

Helper::MockNode createGlazingConductanceNode()
{
    Helper::MockNode node{"GlazingConductance"};
    addChildNode(node, "Low", "0.498817");
    addChildNode(node, "High", "5.880546");
    return node;
}

Helper::MockNode createSpacerConductivityNode()
{
    Helper::MockNode node{"SpacerConductivity"};
    addChildNode(node, "Low", "0.02");
    addChildNode(node, "High", "30");
    return node;
}

struct OptionsKey
{
    std::string glazing;
    std::string spacer;
};

struct LayerValues
{
    std::string hcin;
    std::string hcout;
    std::string emissivityIn;
    std::string emissivityOut;
};

Helper::MockNode createOptionsNode(const OptionsKey & key, const LayerValues & values)
{
    Helper::MockNode node{"Options"};
    addChildNode(node, "Glazing", key.glazing);
    addChildNode(node, "Spacer", key.spacer);
    addChildNode(node, "hcin", values.hcin);
    addChildNode(node, "hcout", values.hcout);
    addChildNode(node, "EmissivityIn", values.emissivityIn);
    addChildNode(node, "EmissivityOut", values.emissivityOut);
    return node;
}

Helper::MockNode createSingleLayer(std::string_view thickness, std::string_view conductivity)
{
    Helper::MockNode node{"SingleLayer"};
    addChildNode(node, "Thickness", thickness);
    addChildNode(node, "Conductivity", conductivity);
    addChildNode(node, "Options", createOptionsNode({"Low", "Low"}, {"4.415185", "27", "0.027", "0.85"}));
    addChildNode(node, "Options", createOptionsNode({"Low", "High"}, {"5.415185", "28", "0.028", "0.86"}));
    addChildNode(node, "Options", createOptionsNode({"High", "Low"}, {"6.529954", "29", "0.85", "0.87"}));
    addChildNode(node, "Options", createOptionsNode({"High", "High"}, {"7.529954", "30", "0.86", "0.88"}));

    return node;
}

Helper::MockNode createDoubleLayer(std::string_view thickness1,
                           std::string_view conductivity1,
                           std::string_view thickness2,
                           std::string_view conductivity2)
{
    Helper::MockNode node{"DoubleLayer"};
    addChildNode(node, "Thickness1", thickness1);
    addChildNode(node, "Conductivity1", conductivity1);
    addChildNode(node, "Thickness2", thickness2);
    addChildNode(node, "Conductivity2", conductivity2);
    addChildNode(node, "Options", createOptionsNode({"Low", "Low"}, {"2.854252", "37", "0.84", "0.94"}));
    addChildNode(node, "Options", createOptionsNode({"Low", "High"}, {"3.854252", "38", "0.86", "0.95"}));
    addChildNode(node, "Options", createOptionsNode({"High", "Low"}, {"4.866122", "39", "0.87", "0.96"}));
    addChildNode(node, "Options", createOptionsNode({"High", "High"}, {"5.866122", "40", "0.88", "0.97"}));

    return node;
}

Helper::MockNode createCMAData()
{
    Helper::MockNode node{"CMAData"};
    addChildNode(node, "Environment", createEnvironmentNode());
    addChildNode(node, "GlazingConductance", createGlazingConductanceNode());
    addChildNode(node, "SpacerConductivity", createSpacerConductivityNode());
    addChildNode(node, "SingleLayer", createSingleLayer("0.004", "1.004"));
    addChildNode(node, "DoubleLayer", createDoubleLayer("0.005", "1.005", "0.002", "1.002"));
    return node;
}

class TestCMA : public testing::Test
{};

TEST_F(TestCMA, EnvironmentDeserialization)
{
    // Note that the MockNodeAdapter is used as variable while createEnvironmentNode() is returning MockNode
    auto environmentNode{createEnvironmentNode()};
    const Helper::MockNodeAdapter adapter{&environmentNode};
    CMALibrary::Environment deserializedEnv;
    adapter >> deserializedEnv;

    static auto tolerance{1e-6};
    EXPECT_NEAR(25.5, deserializedEnv.Tin, tolerance);
    EXPECT_NEAR(-14.12, deserializedEnv.Tout, tolerance);
}

TEST_F(TestCMA, EnvironmentSerialization)
{
    CMALibrary::Environment environment;
    environment.Tin = 25.5;
    environment.Tout = -14.12;

    // Serialize the environment
    Helper::MockNode environmentNode{"Environment"};
    Helper::MockNodeAdapter adapter{&environmentNode};
    adapter << environment;

    // Compare the serialized node structure with the expected structure
    EXPECT_TRUE(compareNodes(adapter.getNode(), environmentNode));
}


TEST_F(TestCMA, GlazingConductanceDeserialization)
{
    // Note that the MockNodeAdapter is used as variable while createEnvironmentNode() is returning MockNode
    auto conductanceNode{createGlazingConductanceNode()};
    const Helper::MockNodeAdapter adapter{&conductanceNode};
    std::map<CMALibrary::Option, double> deserializedConductance;
    adapter >> deserializedConductance;

    static auto tolerance{1e-6};
    EXPECT_NEAR(0.498817, deserializedConductance.at(CMALibrary::Option::Low), tolerance);
    EXPECT_NEAR(5.880546, deserializedConductance.at(CMALibrary::Option::High), tolerance);
}

TEST_F(TestCMA, SpacerConductivityDeserialization)
{
    // Note that the MockNodeAdapter is used as variable while createEnvironmentNode() is returning MockNode
    auto conductivityNode{createSpacerConductivityNode()};
    const Helper::MockNodeAdapter adapter{&conductivityNode};
    std::map<CMALibrary::Option, double> deserializedConductivity;
    adapter >> deserializedConductivity;

    static auto tolerance{1e-6};
    EXPECT_NEAR(0.02, deserializedConductivity.at(CMALibrary::Option::Low), tolerance);
    EXPECT_NEAR(30, deserializedConductivity.at(CMALibrary::Option::High), tolerance);
}

TEST_F(TestCMA, SingleLayerDeserialization)
{
    // Note that the MockNodeAdapter is used as variable while createEnvironmentNode() is returning MockNode
    auto singleLayerNode{createSingleLayer("0.004", "1.004")};
    const Helper::MockNodeAdapter adapter{&singleLayerNode};
    CMALibrary::SingleLayer deserializedValues;
    adapter >> deserializedValues;

    static auto tolerance{1e-6};
    EXPECT_NEAR(0.004, deserializedValues.thickness, tolerance);
    EXPECT_NEAR(1.004, deserializedValues.conductivity, tolerance);

    using CMALibrary::Option;

    const auto & options{deserializedValues.layerOptions};

    EXPECT_NEAR(4.415185, options.at({Option::Low, Option::Low}).hcin, tolerance);
    EXPECT_NEAR(27.00000, options.at({Option::Low, Option::Low}).hcout, tolerance);
    EXPECT_NEAR(0.027000, options.at({Option::Low, Option::Low}).emissivityIn, tolerance);
    EXPECT_NEAR(0.850000, options.at({Option::Low, Option::Low}).emissivityOut, tolerance);

    EXPECT_NEAR(5.415185, options.at({Option::Low, Option::High}).hcin, tolerance);
    EXPECT_NEAR(28.00000, options.at({Option::Low, Option::High}).hcout, tolerance);
    EXPECT_NEAR(0.028000, options.at({Option::Low, Option::High}).emissivityIn, tolerance);
    EXPECT_NEAR(0.860000, options.at({Option::Low, Option::High}).emissivityOut, tolerance);

    EXPECT_NEAR(6.529954, options.at({Option::High, Option::Low}).hcin, tolerance);
    EXPECT_NEAR(29.00000, options.at({Option::High, Option::Low}).hcout, tolerance);
    EXPECT_NEAR(0.850000, options.at({Option::High, Option::Low}).emissivityIn, tolerance);
    EXPECT_NEAR(0.870000, options.at({Option::High, Option::Low}).emissivityOut, tolerance);

    EXPECT_NEAR(7.529954, options.at({Option::High, Option::High}).hcin, tolerance);
    EXPECT_NEAR(30.00000, options.at({Option::High, Option::High}).hcout, tolerance);
    EXPECT_NEAR(0.860000, options.at({Option::High, Option::High}).emissivityIn, tolerance);
    EXPECT_NEAR(0.880000, options.at({Option::High, Option::High}).emissivityOut, tolerance);
}

TEST_F(TestCMA, DoubleLayerDeserialization)
{
    // Note that the MockNodeAdapter is used as variable while createEnvironmentNode() is returning MockNode
    auto doubleLayerNode{createDoubleLayer("0.005", "1.005", "0.002", "1.002")};
    const Helper::MockNodeAdapter adapter{&doubleLayerNode};
    CMALibrary::DoubleLayer deserializedValues;
    adapter >> deserializedValues;

    static auto tolerance{1e-6};
    EXPECT_NEAR(0.005, deserializedValues.thickness1, tolerance);
    EXPECT_NEAR(1.005, deserializedValues.conductivity1, tolerance);
    EXPECT_NEAR(0.002, deserializedValues.thickness2, tolerance);
    EXPECT_NEAR(1.002, deserializedValues.conductivity2, tolerance);

    using CMALibrary::Option;

    const auto & options{deserializedValues.layerOptions};

    EXPECT_NEAR(2.854252, options.at({Option::Low, Option::Low}).hcin, tolerance);
    EXPECT_NEAR(37.00000, options.at({Option::Low, Option::Low}).hcout, tolerance);
    EXPECT_NEAR(0.840000, options.at({Option::Low, Option::Low}).emissivityIn, tolerance);
    EXPECT_NEAR(0.940000, options.at({Option::Low, Option::Low}).emissivityOut, tolerance);

    EXPECT_NEAR(3.854252, options.at({Option::Low, Option::High}).hcin, tolerance);
    EXPECT_NEAR(38.00000, options.at({Option::Low, Option::High}).hcout, tolerance);
    EXPECT_NEAR(0.860000, options.at({Option::Low, Option::High}).emissivityIn, tolerance);
    EXPECT_NEAR(0.950000, options.at({Option::Low, Option::High}).emissivityOut, tolerance);

    EXPECT_NEAR(4.866122, options.at({Option::High, Option::Low}).hcin, tolerance);
    EXPECT_NEAR(39.00000, options.at({Option::High, Option::Low}).hcout, tolerance);
    EXPECT_NEAR(0.870000, options.at({Option::High, Option::Low}).emissivityIn, tolerance);
    EXPECT_NEAR(0.960000, options.at({Option::High, Option::Low}).emissivityOut, tolerance);
}

TEST(TestCMAData, CMADataDeserialization)
{
    // Note that the MockNodeAdapter is used as variable while createEnvironmentNode() is returning MockNode
    auto cmaNode{createCMAData()};
    const Helper::MockNodeAdapter adapter{&cmaNode};
    CMALibrary::CMAData deserializedValues;
    adapter >> deserializedValues;

    static auto tolerance{1e-6};
    EXPECT_NEAR(25.5, deserializedValues.environment.Tin, tolerance);
    EXPECT_NEAR(-14.12, deserializedValues.environment.Tout, tolerance);

    using CMALibrary::Option;

    EXPECT_NEAR(0.498817, deserializedValues.glazingConductance.at(Option::Low), tolerance);
    EXPECT_NEAR(5.880546, deserializedValues.glazingConductance.at(Option::High), tolerance);

    EXPECT_NEAR(0.02, deserializedValues.spacerConductivity.at(Option::Low), tolerance);
    EXPECT_NEAR(30, deserializedValues.spacerConductivity.at(Option::High), tolerance);

    const auto & sLayer{deserializedValues.singleLayer};

    EXPECT_NEAR(0.004, sLayer.thickness, tolerance);
    EXPECT_NEAR(1.004, sLayer.conductivity, tolerance);

    EXPECT_NEAR(4.415185, sLayer.layerOptions.at({Option::Low, Option::Low}).hcin, tolerance);
    EXPECT_NEAR(27.00000, sLayer.layerOptions.at({Option::Low, Option::Low}).hcout, tolerance);
    EXPECT_NEAR(0.027000, sLayer.layerOptions.at({Option::Low, Option::Low}).emissivityIn, tolerance);
    EXPECT_NEAR(0.850000, sLayer.layerOptions.at({Option::Low, Option::Low}).emissivityOut, tolerance);

    EXPECT_NEAR(5.415185, sLayer.layerOptions.at({Option::Low, Option::High}).hcin, tolerance);
    EXPECT_NEAR(28.00000, sLayer.layerOptions.at({Option::Low, Option::High}).hcout, tolerance);
    EXPECT_NEAR(0.028000, sLayer.layerOptions.at({Option::Low, Option::High}).emissivityIn, tolerance);
    EXPECT_NEAR(0.860000, sLayer.layerOptions.at({Option::Low, Option::High}).emissivityOut, tolerance);

    EXPECT_NEAR(6.529954, sLayer.layerOptions.at({Option::High, Option::Low}).hcin, tolerance);
    EXPECT_NEAR(29.00000, sLayer.layerOptions.at({Option::High, Option::Low}).hcout, tolerance);
    EXPECT_NEAR(0.850000, sLayer.layerOptions.at({Option::High, Option::Low}).emissivityIn, tolerance);
    EXPECT_NEAR(0.870000, sLayer.layerOptions.at({Option::High, Option::Low}).emissivityOut, tolerance);

    EXPECT_NEAR(7.529954, sLayer.layerOptions.at({Option::High, Option::High}).hcin, tolerance);
    EXPECT_NEAR(30.00000, sLayer.layerOptions.at({Option::High, Option::High}).hcout, tolerance);
    EXPECT_NEAR(0.860000, sLayer.layerOptions.at({Option::High, Option::High}).emissivityIn, tolerance);
    EXPECT_NEAR(0.880000, sLayer.layerOptions.at({Option::High, Option::High}).emissivityOut, tolerance);

    const auto & dLayer{deserializedValues.doubleLayer};

    EXPECT_NEAR(0.005, dLayer.thickness1, tolerance);
    EXPECT_NEAR(1.005, dLayer.conductivity1, tolerance);
    EXPECT_NEAR(0.002, dLayer.thickness2, tolerance);
    EXPECT_NEAR(1.002, dLayer.conductivity2, tolerance);

    EXPECT_NEAR(2.854252, dLayer.layerOptions.at({Option::Low, Option::Low}).hcin, tolerance);
    EXPECT_NEAR(37.00000, dLayer.layerOptions.at({Option::Low, Option::Low}).hcout, tolerance);
    EXPECT_NEAR(0.840000, dLayer.layerOptions.at({Option::Low, Option::Low}).emissivityIn, tolerance);
    EXPECT_NEAR(0.940000, dLayer.layerOptions.at({Option::Low, Option::Low}).emissivityOut, tolerance);

    EXPECT_NEAR(3.854252, dLayer.layerOptions.at({Option::Low, Option::High}).hcin, tolerance);
    EXPECT_NEAR(38.00000, dLayer.layerOptions.at({Option::Low, Option::High}).hcout, tolerance);
    EXPECT_NEAR(0.860000, dLayer.layerOptions.at({Option::Low, Option::High}).emissivityIn, tolerance);
    EXPECT_NEAR(0.950000, dLayer.layerOptions.at({Option::Low, Option::High}).emissivityOut, tolerance);

    EXPECT_NEAR(4.866122, dLayer.layerOptions.at({Option::High, Option::Low}).hcin, tolerance);
    EXPECT_NEAR(39.00000, dLayer.layerOptions.at({Option::High, Option::Low}).hcout, tolerance);
    EXPECT_NEAR(0.870000, dLayer.layerOptions.at({Option::High, Option::Low}).emissivityIn, tolerance);
    EXPECT_NEAR(0.960000, dLayer.layerOptions.at({Option::High, Option::Low}).emissivityOut, tolerance);

    EXPECT_NEAR(5.866122, dLayer.layerOptions.at({Option::High, Option::High}).hcin, tolerance);
    EXPECT_NEAR(40.00000, dLayer.layerOptions.at({Option::High, Option::High}).hcout, tolerance);
    EXPECT_NEAR(0.880000, dLayer.layerOptions.at({Option::High, Option::High}).emissivityIn, tolerance);
    EXPECT_NEAR(0.970000, dLayer.layerOptions.at({Option::High, Option::High}).emissivityOut, tolerance);
}