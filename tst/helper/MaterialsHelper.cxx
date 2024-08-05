#include <gtest/gtest.h>

#include "MaterialsHelper.hxx"

#include "MockNodeAdapter.hxx"
#include "HelperUtility.hxx"

namespace Helper
{
    //////////////////////////////////////////////////////////////////////////////////////////
    //// Helper functions to test the data
    //////////////////////////////////////////////////////////////////////////////////////////

    void assertVectorOfPoints(const std::vector<std::pair<double, double>> & expectedValues,
                              const std::optional<std::vector<MaterialsLibrary::point>> & optionalPoints,
                              double tolerance)
    {
        ASSERT_TRUE(optionalPoints.has_value());
        ASSERT_EQ(expectedValues.size(), optionalPoints->size());
        for(size_t i = 0; i < expectedValues.size(); ++i)
        {
            EXPECT_NEAR(expectedValues[i].first, optionalPoints.value()[i].x, tolerance);
            EXPECT_NEAR(expectedValues[i].second, optionalPoints.value()[i].y, tolerance);
        }
    }

    void assertOpticalProperties(const MaterialsLibrary::OpticalProperties & correct,
                                 const MaterialsLibrary::OpticalProperties & calculated,
                                 double tolerance)
    {
        EXPECT_NEAR(correct.Transmittance, calculated.Transmittance, tolerance);
        EXPECT_NEAR(correct.Reflectance, calculated.Reflectance, tolerance);
    }

    void assertOpticalSide(const MaterialsLibrary::OpticalSide & correct,
                           const MaterialsLibrary::OpticalSide & calculated,
                           double tolerance)
    {
        assertOpticalProperties(correct.Front, calculated.Front, tolerance);
        assertOpticalProperties(correct.Back, calculated.Back, tolerance);
    }

    void assertOpticalRange(const MaterialsLibrary::OpticalType & correct,
                            const std::optional<MaterialsLibrary::OpticalType> & calculated,
                            double tolerance)
    {
        ASSERT_TRUE(calculated.has_value());
        assertOpticalSide(correct.Direct, calculated->Direct, tolerance);
        assertOpticalSide(correct.Diffuse, calculated->Diffuse, tolerance);
    }

    void assertInfraredSection(const MaterialsLibrary::Infrared & correct,
                               const MaterialsLibrary::Infrared & calculated,
                               double tolerance)
    {
        assertOptionalNear(correct.Transmittance, calculated.Transmittance, tolerance);
        assertOptionalNear(correct.EmissivityFront, calculated.EmissivityFront, tolerance);
        assertOptionalNear(correct.EmissivityBack, calculated.EmissivityBack, tolerance);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //// Helper functions to fill the data
    //////////////////////////////////////////////////////////////////////////////////////////

    void addTableValueNode(Helper::MockNode & node, std::string_view childName, std::string_view x, std::string_view y)
    {
        Helper::MockNode & tableValueNode(addChildNode(node, childName));
        addChildNode(tableValueNode, "x", x);
        addChildNode(tableValueNode, "y", y);
    }

    void fillNodesWithTableValues(MockNode & parentNode,
                                  const std::string & parentName,
                                  const std::string & tableValueNodeName,
                                  const std::vector<std::pair<std::string, std::string>> & values)
    {
        // Create the parent node for the table values
        Helper::MockNode & parent = addChildNode(parentNode, parentName);

        // Iterate over the value pairs and add them as children
        for(const auto & [x, y] : values)
        {
            addTableValueNode(parent, tableValueNodeName, x, y);
        }
    }

    void fillOpticalPropertiesNode(MockNode & parentNode,
                                   std::string_view sideName,
                                   std::string_view transmittanceValue,
                                   std::string_view reflectanceValue)
    {
        // Create the side node (Front or Back)
        auto & sideNode = addChildNode(parentNode, sideName);

        // Add the Transmittance and Reflectance nodes
        addChildNode(sideNode, "Transmittance", transmittanceValue);
        addChildNode(sideNode, "Reflectance", reflectanceValue);
    }

    void fillOpticalSide(MockNode & node, std::string_view sideName, const TestPropertyList & properties)
    {
        auto & side = addChildNode(node, sideName);

        for(const auto & [sectionName, values] : properties)
        {
            fillOpticalPropertiesNode(side, sectionName, values.first, values.second);
        }
    }

    void fillOpticalRange(MockNode & node, std::string_view rangeName, const std::vector<RangeProperty> & properties)
    {
        auto & range = addChildNode(node, rangeName);
        for(const auto & [sectionName, values] : properties)
        {
            fillOpticalSide(range, sectionName, values);
        }
    }

    void fillInfraredSection(MockNode & node,
                             std::optional<std::string_view> transmittance,
                             std::optional<std::string_view> emissivityFront,
                             std::optional<std::string_view> emissivityBack)
    {
        auto & infrared = addChildNode(node, "Infrared");
        if(transmittance.has_value())
        {
            addChildNode(infrared, "Transmittance", transmittance.value());
        }

        if(emissivityFront.has_value())
        {
            addChildNode(infrared, "Emissivity-Front", emissivityFront.value());
        }

        if(emissivityBack.has_value())
        {
            addChildNode(infrared, "Emissivity-Back", emissivityBack.value());
        }
    }

    void fillSolidMaterialTest1(MockNode & node)
    {
        auto & materialNode{addChildNode(node, "Material")};
        addChildNode(materialNode, "UUID", "SomeUUID");
        addChildNode(materialNode, "Name", "SomeMaterial");
        addChildNode(materialNode, "Protected", "true");
        addChildNode(materialNode, "Color", "0xFF8080");

        auto & solidNode{addChildNode(materialNode, "Solid")};

        auto & htNode{addChildNode(solidNode, "HygroThermal")};

        addChildNode(htNode, "DefaultThickness", "0.1");
        addChildNode(htNode, "MaterialInformation", "Some material information");
        addChildNode(htNode, "BulkDensity", "800");
        addChildNode(htNode, "Porosity", "0.35");
        addChildNode(htNode, "SpecificHeatCapacityDry", "840");
        addChildNode(htNode, "ThermalConductivityDry", "0.2");
        addChildNode(htNode, "Roughness", "Medium Rough");

        fillNodesWithTableValues(htNode, "MoistureStorageFunction", "TableValue", {{"0.05", "12"}, {"0.1", "37"}});
        fillNodesWithTableValues(
          htNode, "LiquidTransportationCoefficientSuction", "TableValue", {{"35", "3.1e-10"}, {"100", "3.1e-09"}});
        fillNodesWithTableValues(
          htNode, "ThermalConductivityMoistureDependent", "TableValue", {{"0.05", "0.1"}, {"0.1", "0.2"}});
        fillNodesWithTableValues(
          htNode, "ThermalConductivityTemperatureDependent", "TableValue", {{"10", "0.18"}, {"15", "0.23"}});

        auto & opticalNode{addChildNode(solidNode, "Optical")};
        auto & integratedNode{addChildNode(opticalNode, "Integrated")};
        Helper::fillInfraredSection(integratedNode, std::nullopt, "0.8", "0.7");
        fillOpticalRange(integratedNode,
                         "Solar",
                         {{"Direct", {{"Front", {"0.75", "0.14"}}, {"Back", {"0.7", "0.28"}}}},
                          {"Diffuse", {{"Front", {"0.55", "0.33"}}, {"Back", {"0.82", "0.15"}}}}});

        fillOpticalRange(integratedNode,
                         "Visible",
                         {{"Direct", {{"Front", {"0.61", "0.47"}}, {"Back", {"0.91", "0.35"}}}},
                          {"Diffuse", {{"Front", {"0.61", "0.28"}}, {"Back", {"0.85", "0.24"}}}}});
    }
}   // namespace Helper
