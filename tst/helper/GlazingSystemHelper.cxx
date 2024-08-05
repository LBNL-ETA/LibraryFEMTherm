#include <gtest/gtest.h>

#include <utility>

#include "GlazingSystemHelper.hxx"

#include "MockNodeAdapter.hxx"

namespace Helper
{
    void expect_near(const ThermFile::EnvironmentCoefficients & expected,
                     const ThermFile::EnvironmentCoefficients & actual,
                     double tolerance)
    {
        EXPECT_NEAR(expected.h, actual.h, tolerance);
        EXPECT_NEAR(expected.hc, actual.hc, tolerance);
        EXPECT_NEAR(expected.hr, actual.hr, tolerance);
        EXPECT_NEAR(expected.temperature, actual.temperature, tolerance);
    }

    void expect_near(const ThermFile::GlazingSystemProperties & expected,
                     const ThermFile::GlazingSystemProperties & actual,
                     double tolerance)
    {
        EXPECT_EQ(expected.environmentalCondition, actual.environmentalCondition);
        EXPECT_NEAR(expected.uValue, actual.uValue, tolerance);
        EXPECT_EQ(expected.coefficients.size(), actual.coefficients.size());
        for(const auto & [key, value] : expected.coefficients)
        {
            const auto it = actual.coefficients.find(key);
            ASSERT_NE(it, actual.coefficients.end());
            expect_near(value, it->second, tolerance);
        }
        EXPECT_NEAR(expected.solar, actual.solar, tolerance);
    }

    void expect_near(const ThermFile::IGUSurface & expected, const ThermFile::IGUSurface & actual, double tolerance)
    {
        EXPECT_NEAR(expected.temperature, actual.temperature, tolerance);
        EXPECT_NEAR(expected.emissivity, actual.emissivity, tolerance);
    }

    void expect_near(const ThermFile::IGULayer & expected, const ThermFile::IGULayer & actual, double tolerance)
    {
        EXPECT_EQ(expected.ID, actual.ID);
        EXPECT_EQ(expected.name, actual.name);
        EXPECT_NEAR(expected.thickness, actual.thickness, tolerance);
        EXPECT_NEAR(expected.conductivity, actual.conductivity, tolerance);
        expect_near(expected.surface1, actual.surface1, tolerance);
        expect_near(expected.surface2, actual.surface2, tolerance);
    }

    void expect_near(const ThermFile::IGUGasCoefficients & expected,
                     const ThermFile::IGUGasCoefficients & actual,
                     double tolerance)
    {
        EXPECT_NEAR(expected.A, actual.A, tolerance);
        EXPECT_NEAR(expected.B, actual.B, tolerance);
        EXPECT_NEAR(expected.C, actual.C, tolerance);
    }

    void expect_near(const ThermFile::IGUGasProperties & expected,
                     const ThermFile::IGUGasProperties & actual,
                     double tolerance)
    {
        EXPECT_EQ(expected.name, actual.name);
        EXPECT_NEAR(expected.molecularWeight, actual.molecularWeight, tolerance);
        expect_near(expected.conductivity, actual.conductivity, tolerance);
        expect_near(expected.viscosity, actual.viscosity, tolerance);
        expect_near(expected.specificHeat, actual.specificHeat, tolerance);
    }

    void expect_near(const ThermFile::IGUPureGas & expected, const ThermFile::IGUPureGas & actual, double tolerance)
    {
        EXPECT_NEAR(expected.fraction, actual.fraction, tolerance);
        expect_near(expected.properties, actual.properties, tolerance);
    }

    void expect_near(const ThermFile::IGUGas & expected, const ThermFile::IGUGas & actual, double tolerance)
    {
        EXPECT_EQ(expected.ID, actual.ID);
        EXPECT_EQ(expected.name, actual.name);
        EXPECT_NEAR(expected.thickness, actual.thickness, tolerance);
        EXPECT_NEAR(expected.conductivity, actual.conductivity, tolerance);
        ASSERT_EQ(expected.components.size(), actual.components.size());
        for(size_t i = 0; i < expected.components.size(); ++i)
        {
            expect_near(expected.components[i], actual.components[i], tolerance);
        }
    }

    void
      expect_near(const ThermFile::IGUShadeLayer & expected, const ThermFile::IGUShadeLayer & actual, double tolerance)
    {
        EXPECT_EQ(expected.position, actual.position);
        EXPECT_NEAR(expected.emissivityRatio, actual.emissivityRatio, tolerance);
        EXPECT_NEAR(expected.hcRatio, actual.hcRatio, tolerance);
        EXPECT_NEAR(expected.thickness, actual.thickness, tolerance);
        EXPECT_NEAR(expected.gapThickness, actual.gapThickness, tolerance);
        EXPECT_NEAR(expected.modeledHeight, actual.modeledHeight, tolerance);
        expect_near(expected.properties, actual.properties, tolerance);
    }

    void
      expect_near(const ThermFile::GlazingSystem & expected, const ThermFile::GlazingSystem & actual, double tolerance)
    {
        EXPECT_EQ(expected.ID, actual.ID);
        EXPECT_EQ(expected.index, actual.index);
        EXPECT_EQ(expected.name, actual.name);
        ASSERT_EQ(expected.properties.size(), actual.properties.size());
        for(size_t i = 0; i < expected.properties.size(); ++i)
        {
            expect_near(expected.properties[i], actual.properties[i], tolerance);
        }
        ASSERT_EQ(expected.layers.size(), actual.layers.size());
        for(size_t i = 0; i < expected.layers.size(); ++i)
        {
            expect_near(expected.layers[i], actual.layers[i], tolerance);
        }
        ASSERT_EQ(expected.gases.size(), actual.gases.size());
        for(size_t i = 0; i < expected.gases.size(); ++i)
        {
            expect_near(expected.gases[i], actual.gases[i], tolerance);
        }
        if(expected.shadeLayer)
        {
            ASSERT_TRUE(actual.shadeLayer);
            expect_near(*expected.shadeLayer, *actual.shadeLayer, tolerance);
        }
        else
        {
            ASSERT_FALSE(actual.shadeLayer);
        }
        EXPECT_NEAR(expected.actualHeight, actual.actualHeight, tolerance);
        EXPECT_EQ(expected.orientation, actual.orientation);
        EXPECT_EQ(expected.isNominalThickness, actual.isNominalThickness);
        EXPECT_EQ(expected.interiorBC, actual.interiorBC);
        EXPECT_EQ(expected.exteriorBC, actual.exteriorBC);
        EXPECT_EQ(expected.optionFlag, actual.optionFlag);
        EXPECT_EQ(expected.source, actual.source);
        EXPECT_EQ(expected.siteLineHeight, actual.siteLineHeight);
        EXPECT_EQ(expected.eogHeight, actual.eogHeight);
        EXPECT_EQ(expected.sightLineToShadeEdge, actual.sightLineToShadeEdge);
        EXPECT_EQ(expected.isDeflected, actual.isDeflected);
        EXPECT_EQ(expected.edgeGlassTag, actual.edgeGlassTag);
    }

    MockEnvironmentCoefficients::MockEnvironmentCoefficients(
      std::string h, std::string hc, std::string hr, std::string temperature) :
        h{std::move(h)},
        hc{std::move(hc)},
        hr{std::move(hr)},
        temperature{std::move(temperature)}
    {}

    MockNode generateEnvironmentCoefficients(const MockEnvironmentCoefficients & environmentCoefficients)
    {
        MockNode node{"EnvironmentCoefficients"};
        addChildNode(node, "H", environmentCoefficients.h);
        addChildNode(node, "Hc", environmentCoefficients.hc);
        addChildNode(node, "Hr", environmentCoefficients.hr);
        addChildNode(node, "Temperature", environmentCoefficients.temperature);
        return node;
    }

    MockGlazingSystemProperties::MockGlazingSystemProperties(
      std::string environmentalCondition,
      std::string boundaryConditionStyle,
      std::string uValue,
      std::map<std::string, MockEnvironmentCoefficients, std::less<>> coefficients,
      std::string solar) :
        environmentalCondition{std::move(environmentalCondition)},
        boundaryConditionStyle{std::move(boundaryConditionStyle)},
        uValue{std::move(uValue)},
        coefficients{std::move(coefficients)},
        solar{std::move(solar)}
    {}

    MockNode generateGlazingSystemProperties(const MockGlazingSystemProperties & glazingSystemProperties)
    {
        MockNode node{"GlazingSystemProperties"};
        addChildNode(node, "EnvironmentalCondition", glazingSystemProperties.environmentalCondition);
        addChildNode(node, "BoundaryConditionStyle", glazingSystemProperties.boundaryConditionStyle);
        addChildNode(node, "U-value", glazingSystemProperties.uValue);
        auto & coefficientsNode = addChildNode(node, "Coefficients");
        for(const auto & [key, value] : glazingSystemProperties.coefficients)
        {
            addChildNode(coefficientsNode, key, generateEnvironmentCoefficients(value));
        }
        addChildNode(node, "Solar", glazingSystemProperties.solar);
        return node;
    }

    MockIGUSurface::MockIGUSurface(std::string temperature, std::string emissivity) :
        temperature{std::move(temperature)}, emissivity{std::move(emissivity)}
    {}

    MockNode generateIGUSurface(const MockIGUSurface & iguSurface)
    {
        MockNode node{"Surface"};
        addChildNode(node, "Temperature", iguSurface.temperature);
        addChildNode(node, "Emissivity", iguSurface.emissivity);
        return node;
    }

    MockIGULayer::MockIGULayer(std::string ID,
                               std::string name,
                               std::string thickness,
                               std::string conductivity,
                               MockIGUSurface surface1,
                               MockIGUSurface surface2) :
        ID{std::move(ID)},
        name{std::move(name)},
        thickness{std::move(thickness)},
        conductivity{std::move(conductivity)},
        surface1{std::move(surface1)},
        surface2{std::move(surface2)}
    {}

    MockNode generateIGULayer(const MockIGULayer & iguLayer)
    {
        MockNode node{"IGULayer"};
        addChildNode(node, "ID", iguLayer.ID);
        addChildNode(node, "Name", iguLayer.name);
        addChildNode(node, "Thickness", iguLayer.thickness);
        addChildNode(node, "Conductivity", iguLayer.conductivity);
        addChildNode(node, "Surface1", generateIGUSurface(iguLayer.surface1));
        addChildNode(node, "Surface2", generateIGUSurface(iguLayer.surface2));
        return node;
    }

    MockIGUGasCoefficients::MockIGUGasCoefficients(std::string A, std::string B, std::string C) :
        A{std::move(A)}, B{std::move(B)}, C{std::move(C)}
    {}

    MockNode generateIGUGasCoefficients(const MockIGUGasCoefficients & iguGasCoefficients)
    {
        MockNode node{"IGUGasCoefficients"};
        addChildNode(node, "A", iguGasCoefficients.A);
        addChildNode(node, "B", iguGasCoefficients.B);
        addChildNode(node, "C", iguGasCoefficients.C);
        return node;
    }

    MockIGUGasProperties::MockIGUGasProperties(std::string name,
                                               std::string molecularWeight,
                                               MockIGUGasCoefficients conductivity,
                                               MockIGUGasCoefficients viscosity,
                                               MockIGUGasCoefficients specificHeat) :
        name{std::move(name)},
        molecularWeight{std::move(molecularWeight)},
        conductivity{std::move(conductivity)},
        viscosity{std::move(viscosity)},
        specificHeat{std::move(specificHeat)}
    {}

    MockNode generateIGUGasProperties(const MockIGUGasProperties & iguGasProperties)
    {
        MockNode node{"IGUGasProperties"};
        addChildNode(node, "Name", iguGasProperties.name);
        addChildNode(node, "MolecularWeight", iguGasProperties.molecularWeight);
        addChildNode(node, "Conductivity", generateIGUGasCoefficients(iguGasProperties.conductivity));
        addChildNode(node, "Viscosity", generateIGUGasCoefficients(iguGasProperties.viscosity));
        addChildNode(node, "SpecificHeat", generateIGUGasCoefficients(iguGasProperties.specificHeat));
        return node;
    }

    MockIGUPureGas::MockIGUPureGas(std::string fraction, MockIGUGasProperties properties) :
        fraction{std::move(fraction)}, properties{std::move(properties)}
    {}

    MockNode generateIGUPureGas(const MockIGUPureGas & iguPureGas)
    {
        MockNode node{"IGUPureGas"};
        addChildNode(node, "Fraction", iguPureGas.fraction);
        addChildNode(node, "Properties", generateIGUGasProperties(iguPureGas.properties));
        return node;
    }

    MockIGUGas::MockIGUGas(std::string ID,
                           std::string name,
                           std::string thickness,
                           std::string conductivity,
                           std::vector<MockIGUPureGas> components) :
        ID{std::move(ID)},
        name{std::move(name)},
        thickness{std::move(thickness)},
        conductivity{std::move(conductivity)},
        components{std::move(components)}
    {}

    MockNode generateIGUGas(const MockIGUGas & iguGas)
    {
        MockNode node{"IGUGas"};
        addChildNode(node, "ID", iguGas.ID);
        addChildNode(node, "Name", iguGas.name);
        addChildNode(node, "Thickness", iguGas.thickness);
        addChildNode(node, "Conductivity", iguGas.conductivity);
        auto & componentsNode = addChildNode(node, "Components");
        for(const auto & component : iguGas.components)
        {
            addChildNode(componentsNode, "Component", generateIGUPureGas(component));
        }
        return node;
    }

    MockShadeProperties::MockShadeProperties(
      std::string dBottom, std::string dTop, std::string dLeft, std::string dRight, std::string gapKeff) :
        dBottom{std::move(dBottom)},
        dTop{std::move(dTop)},
        dLeft{std::move(dLeft)},
        dRight{std::move(dRight)},
        gapKeff{std::move(gapKeff)}
    {}

    MockNode generateShadeProperties(const MockShadeProperties & shadeProperties)
    {
        MockNode node{"ShadeProperties"};
        addChildNode(node, "DBottom", shadeProperties.dBottom);
        addChildNode(node, "DTop", shadeProperties.dTop);
        addChildNode(node, "DLeft", shadeProperties.dLeft);
        addChildNode(node, "DRight", shadeProperties.dRight);
        addChildNode(node, "GapKeff", shadeProperties.gapKeff);
        return node;
    }

    void expect_near(const ThermFile::ShadeProperties & expected,
                     const ThermFile::ShadeProperties & actual,
                     double tolerance)
    {
        EXPECT_NEAR(expected.dBottom, actual.dBottom, tolerance);
        EXPECT_NEAR(expected.dTop, actual.dTop, tolerance);
        EXPECT_NEAR(expected.dLeft, actual.dLeft, tolerance);
        EXPECT_NEAR(expected.dRight, actual.dRight, tolerance);
        EXPECT_NEAR(expected.gapKeff, actual.gapKeff, tolerance);
    }

    MockIGUShadeLayer::MockIGUShadeLayer(std::string position,
                                         std::string emissivityRatio,
                                         std::string hcRatio,
                                         std::string thickness,
                                         std::string gapThickness,
                                         std::string modeledHeight,
                                         MockShadeProperties properties) :
        position{std::move(position)},
        emissivityRatio{std::move(emissivityRatio)},
        hcRatio{std::move(hcRatio)},
        thickness{std::move(thickness)},
        gapThickness{std::move(gapThickness)},
        modeledHeight{std::move(modeledHeight)},
        properties{std::move(properties)}
    {}

    MockNode generateIGUShadeLayer(const MockIGUShadeLayer & iguShadeLayer)
    {
        MockNode node{"IGUShadeLayer"};
        addChildNode(node, "Position", iguShadeLayer.position);
        addChildNode(node, "EmissivityRatio", iguShadeLayer.emissivityRatio);
        addChildNode(node, "HcRatio", iguShadeLayer.hcRatio);
        addChildNode(node, "Thickness", iguShadeLayer.thickness);
        addChildNode(node, "GapThickness", iguShadeLayer.gapThickness);
        addChildNode(node, "ModeledHeight", iguShadeLayer.modeledHeight);
        addChildNode(node, "Properties", generateShadeProperties(iguShadeLayer.properties));

        return node;
    }

    MockGlazingSystem::MockGlazingSystem(std::string ID,
                                         std::string index,
                                         std::string name,
                                         std::vector<MockGlazingSystemProperties> properties,
                                         std::vector<MockIGULayer> layers,
                                         std::vector<MockIGUGas> gases,
                                         std::optional<MockIGUShadeLayer> shadeLayer,
                                         std::string actualHeight,
                                         std::string orientation,
                                         std::string isNominalThickness,
                                         std::string interiorBC,
                                         std::string exteriorBC,
                                         std::string optionFlag,
                                         std::string source,
                                         std::string siteLineHeight,
                                         std::string eogHeight,
                                         std::string sightLineToEdge,
                                         std::string isDeflected,
                                         std::string edgeGlassTag) :
        ID{std::move(ID)},
        index{std::move(index)},
        name{std::move(name)},
        properties{std::move(properties)},
        layers{std::move(layers)},
        gases{std::move(gases)},
        shadeLayer{std::move(shadeLayer)},
        actualHeight{std::move(actualHeight)},
        orientation{std::move(orientation)},
        isNominalThickness{std::move(isNominalThickness)},
        interiorBC{std::move(interiorBC)},
        exteriorBC{std::move(exteriorBC)},
        optionFlag{std::move(optionFlag)},
        source{std::move(source)},
        siteLineHeight{std::move(siteLineHeight)},
        eogHeight{std::move(eogHeight)},
        sightLineToEdge{std::move(sightLineToEdge)},
        isDeflected{std::move(isDeflected)},
        edgeGlassTag{std::move(edgeGlassTag)}
    {}

    MockNode generateGlazingSystem(const MockGlazingSystem & glazingSystem)
    {
        MockNode node{"GlazingSystem"};
        addChildNode(node, "ID", glazingSystem.ID);
        addChildNode(node, "Index", glazingSystem.index);
        addChildNode(node, "Name", glazingSystem.name);
        auto & propertiesNode = addChildNode(node, "GlazingSystemProperties");
        for(const auto & property : glazingSystem.properties)
        {
            addChildNode(propertiesNode, "Property", generateGlazingSystemProperties(property));
        }
        auto & layersNode = addChildNode(node, "Layers");
        for(const auto & layer : glazingSystem.layers)
        {
            addChildNode(layersNode, "Layer", generateIGULayer(layer));
        }
        auto & gasesNode = addChildNode(node, "Gases");
        for(const auto & gas : glazingSystem.gases)
        {
            addChildNode(gasesNode, "Gas", generateIGUGas(gas));
        }
        if(glazingSystem.shadeLayer)
        {
            addChildNode(node, "ShadeLayer", generateIGUShadeLayer(*glazingSystem.shadeLayer));
        }
        addChildNode(node, "ActualHeight", glazingSystem.actualHeight);
        addChildNode(node, "Orientation", glazingSystem.orientation);
        addChildNode(node, "IsNominalThickness", glazingSystem.isNominalThickness);
        addChildNode(node, "InteriorBC", glazingSystem.interiorBC);
        addChildNode(node, "ExteriorBC", glazingSystem.exteriorBC);
        addChildNode(node, "OptionFlag", glazingSystem.optionFlag);
        addChildNode(node, "Source", glazingSystem.source);
        addChildNode(node, "SiteLineHeight", glazingSystem.siteLineHeight);
        addChildNode(node, "EOGHeight", glazingSystem.eogHeight);
        addChildNode(node, "SightLineToShadeEdge", glazingSystem.sightLineToEdge);
        addChildNode(node, "IsDeflected", glazingSystem.isDeflected);
        addChildNode(node, "EdgeGlassTag", glazingSystem.edgeGlassTag);

        return node;
    }
}   // namespace Helper