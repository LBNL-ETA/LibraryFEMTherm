#include <gtest/gtest.h>

#include "Gases/Gases.hxx"
#include "Gases/Serializers.hxx"

#include "MockNodeAdapter.hxx"

class TestGases : public testing::Test
{
protected:
    static void addConductivitySection(Helper::MockNode & parentNode)
    {
        auto & conductivityNode = addChildNode(parentNode, "Conductivity");
        addChildNode(conductivityNode, "A", "0.002287");
        addChildNode(conductivityNode, "B", "7.76e-05");
        addChildNode(conductivityNode, "C", "0");
    }

    static void addViscositySection(Helper::MockNode & parentNode)
    {
        auto & viscosityNode = addChildNode(parentNode, "Viscosity");
        addChildNode(viscosityNode, "A", "3.723e-06");
        addChildNode(viscosityNode, "B", "4.94e-08");
        addChildNode(viscosityNode, "C", "0");
    }

    static void addSpecificHeatSection(Helper::MockNode & parentNode)
    {
        auto & specificHeatNode = addChildNode(parentNode, "SpecificHeat");
        addChildNode(specificHeatNode, "A", "1002.737");
        addChildNode(specificHeatNode, "B", "0.012324");
        addChildNode(specificHeatNode, "C", "0");
    }
};

TEST_F(TestGases, DeserializeCoefficients)
{
    auto mockData = []() {
        Helper::MockNode node{"Coefficients"};

        addChildNode(node, "A", "0.002285");
        addChildNode(node, "B", "0.00005149");
        addChildNode(node, "C", "0");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    GasesLibrary::Coefficients coefficients;
    adapter >> coefficients;

    GasesLibrary::Coefficients correct{0.002285, 0.00005149, 0};

    constexpr auto tolerance{1e-6};
    EXPECT_NEAR(correct.A, coefficients.A, tolerance);
    EXPECT_NEAR(correct.B, coefficients.B, tolerance);
    EXPECT_NEAR(correct.C, coefficients.C, tolerance);
}

TEST_F(TestGases, SerializeCoefficients)
{
    GasesLibrary::Coefficients element{0.002285, 0.00005149, 0};

    Helper::MockNode elementNode("Coefficients");
    Helper::MockNodeAdapter adapter{&elementNode};

    adapter << element;

    auto correctNodes = []() {
        Helper::MockNode node{"Coefficients"};

        addChildNode(node, "A", "0.002285");
        addChildNode(node, "B", "5.149e-05");
        addChildNode(node, "C", "0");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestGases, DeserializeProperties)
{
    auto mockData = [&]() {
        Helper::MockNode node{"Properties"};

        addChildNode(node, "MolecularWeight", "28.97");
        addChildNode(node, "SpecificHeatRatio", "1.4");

        addConductivitySection(node);
        addViscositySection(node);
        addSpecificHeatSection(node);

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    GasesLibrary::GasProperties properties;
    adapter >> properties;

    GasesLibrary::GasProperties correct{
      28.97, 1.4, {0.0022873, 0.0000776, 0}, {0.000003723, 4.94E-08, 0}, {1002.737, 0.012324, 0}};

    constexpr auto tolerance{1e-6};
    EXPECT_NEAR(correct.MolecularWeight, properties.MolecularWeight, tolerance);
    EXPECT_NEAR(correct.SpecificHeatRatio, properties.SpecificHeatRatio, tolerance);

    EXPECT_NEAR(correct.Conductivity.A, properties.Conductivity.A, tolerance);
    EXPECT_NEAR(correct.Conductivity.B, properties.Conductivity.B, tolerance);
    EXPECT_NEAR(correct.Conductivity.C, properties.Conductivity.C, tolerance);

    EXPECT_NEAR(correct.Viscosity.A, properties.Viscosity.A, tolerance);
    EXPECT_NEAR(correct.Viscosity.B, properties.Viscosity.B, tolerance);
    EXPECT_NEAR(correct.Viscosity.C, properties.Viscosity.C, tolerance);

    EXPECT_NEAR(correct.SpecificHeat.A, properties.SpecificHeat.A, tolerance);
    EXPECT_NEAR(correct.SpecificHeat.B, properties.SpecificHeat.B, tolerance);
    EXPECT_NEAR(correct.SpecificHeat.C, properties.SpecificHeat.C, tolerance);
}

TEST_F(TestGases, SerializeProperties)
{
    GasesLibrary::GasProperties element{
      28.97, 1.4, {0.0022873, 0.0000776, 0}, {0.000003723, 4.94E-08, 0}, {1002.737, 0.012324, 0}};

    Helper::MockNode elementNode("Properties");
    Helper::MockNodeAdapter adapter{&elementNode};

    adapter << element;

    auto correctNodes = []() {
        Helper::MockNode node{"Properties"};

        addChildNode(node, "MolecularWeight", "28.97");
        addChildNode(node, "SpecificHeatRatio", "1.4");

        addConductivitySection(node);
        addViscositySection(node);
        addSpecificHeatSection(node);

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestGases, DeserializePureGas)
{
    auto mockData = [&]() {
        Helper::MockNode node{"PureGas"};

        addChildNode(node, "UUID", "8d33196f-f052-46e6-8353-bccb9a779f9c");
        addChildNode(node, "Name", "Air");
        addChildNode(node, "Protected", "true");

        auto & propertiesNode = addChildNode(node, "Properties");

        addChildNode(propertiesNode, "MolecularWeight", "28.970000");
        addChildNode(propertiesNode, "SpecificHeatRatio", "1.400000");

        addConductivitySection(propertiesNode);
        addViscositySection(propertiesNode);
        addSpecificHeatSection(propertiesNode);

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    GasesLibrary::PureGas element;
    adapter >> element;

    GasesLibrary::PureGas correct{
      "8d33196f-f052-46e6-8353-bccb9a779f9c",
      "Air",
      true,
      {28.97, 1.4, {0.0022873, 0.0000776, 0}, {0.000003723, 4.94E-08, 0}, {1002.737, 0.012324, 0}}};

    constexpr auto tolerance{1e-6};
    EXPECT_EQ(correct.UUID, element.UUID);
    EXPECT_EQ(correct.Name, element.Name);
    EXPECT_EQ(correct.Protected, element.Protected);

    EXPECT_NEAR(correct.Properties.MolecularWeight, element.Properties.MolecularWeight, tolerance);
    EXPECT_NEAR(correct.Properties.SpecificHeatRatio, element.Properties.SpecificHeatRatio, tolerance);

    EXPECT_NEAR(correct.Properties.Conductivity.A, element.Properties.Conductivity.A, tolerance);
    EXPECT_NEAR(correct.Properties.Conductivity.B, element.Properties.Conductivity.B, tolerance);
    EXPECT_NEAR(correct.Properties.Conductivity.C, element.Properties.Conductivity.C, tolerance);

    EXPECT_NEAR(correct.Properties.Viscosity.A, element.Properties.Viscosity.A, tolerance);
    EXPECT_NEAR(correct.Properties.Viscosity.B, element.Properties.Viscosity.B, tolerance);
    EXPECT_NEAR(correct.Properties.Viscosity.C, element.Properties.Viscosity.C, tolerance);

    EXPECT_NEAR(correct.Properties.SpecificHeat.A, element.Properties.SpecificHeat.A, tolerance);
    EXPECT_NEAR(correct.Properties.SpecificHeat.B, element.Properties.SpecificHeat.B, tolerance);
    EXPECT_NEAR(correct.Properties.SpecificHeat.C, element.Properties.SpecificHeat.C, tolerance);
}

TEST_F(TestGases, SerializePureGas)
{
    GasesLibrary::PureGas element{
      "8d33196f-f052-46e6-8353-bccb9a779f9c",
      "Air",
      true,
      {28.97, 1.4, {0.0022873, 0.0000776, 0}, {0.000003723, 4.94E-08, 0}, {1002.737, 0.012324, 0}}};

    Helper::MockNode elementNode("PureGas");
    Helper::MockNodeAdapter adapter{&elementNode};

    adapter << element;

    auto correctNodes = [&]() {
        Helper::MockNode node{"PureGas"};

        addChildNode(node, "UUID", "8d33196f-f052-46e6-8353-bccb9a779f9c");
        addChildNode(node, "Name", "Air");
        addChildNode(node, "Protected", "true");

        auto & propertiesNode = addChildNode(node, "Properties");

        addChildNode(propertiesNode, "MolecularWeight", "28.97");
        addChildNode(propertiesNode, "SpecificHeatRatio", "1.4");

        addConductivitySection(propertiesNode);
        addViscositySection(propertiesNode);
        addSpecificHeatSection(propertiesNode);

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}

TEST_F(TestGases, DeserializeGasMixture)
{
    auto mockData = [&]() {
        Helper::MockNode node{"Gas"};

        addChildNode(node, "UUID", "37881919-7cdf-4428-bf5c-10702bc78415");
        addChildNode(node, "Name", "Air 10%/Argon 90%");
        addChildNode(node, "Protected", "true");

        auto & componentsNode = addChildNode(node, "Components");

        auto & component1Node = addChildNode(componentsNode, "Component");
        addChildNode(component1Node, "Fraction", "0.1");
        addChildNode(component1Node, "PureGas", "Air");

        auto & component2Node = addChildNode(componentsNode, "Component");
        addChildNode(component2Node, "Fraction", "0.9");
        addChildNode(component2Node, "PureGas", "Argon");

        return node;
    };

    auto elementNode(mockData());
    const Helper::MockNodeAdapter adapter{&elementNode};

    GasesLibrary::Gas element;
    adapter >> element;

    GasesLibrary::Gas correct{
      "37881919-7cdf-4428-bf5c-10702bc78415", "Air 10%/Argon 90%", true, {{0.1, "Air"}, {0.9, "Argon"}}};

    constexpr auto tolerance{1e-6};
    EXPECT_EQ(correct.UUID, element.UUID);
    EXPECT_EQ(correct.Name, element.Name);
    EXPECT_EQ(correct.Protected, element.Protected);
    for(size_t i = 0; i < correct.Components.size(); ++i)
    {
        EXPECT_NEAR(correct.Components[i].Fraction, element.Components[i].Fraction, tolerance);
        EXPECT_EQ(correct.Components[i].PureGasName, element.Components[i].PureGasName);
    }
}

TEST_F(TestGases, SerializeGasMixture)
{
    GasesLibrary::Gas element{
      "37881919-7cdf-4428-bf5c-10702bc78415", "Air 10%/Argon 90%", true, {{0.1, "Air"}, {0.9, "Argon"}}};

    Helper::MockNode elementNode("Gas");
    Helper::MockNodeAdapter adapter{&elementNode};

    adapter << element;

    auto correctNodes = [&]() {
        Helper::MockNode node{"Gas"};

        addChildNode(node, "UUID", "37881919-7cdf-4428-bf5c-10702bc78415");
        addChildNode(node, "Name", "Air 10%/Argon 90%");
        addChildNode(node, "Protected", "true");

        auto & componentsNode = addChildNode(node, "Components");

        auto & component1Node = addChildNode(componentsNode, "Component");
        addChildNode(component1Node, "Fraction", "0.1");
        addChildNode(component1Node, "PureGas", "Air");

        auto & component2Node = addChildNode(componentsNode, "Component");
        addChildNode(component2Node, "Fraction", "0.9");
        addChildNode(component2Node, "PureGas", "Argon");

        return node;
    };

    EXPECT_TRUE(Helper::compareNodes(adapter.getNode(), correctNodes()));
}