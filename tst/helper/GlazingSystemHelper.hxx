#pragma once

#include "THMZ/GlazingSystem/GlazingSystem.hxx"

namespace Helper
{
    struct MockNode;

    void expect_near(const ThermFile::EnvironmentCoefficients & expected,
                     const ThermFile::EnvironmentCoefficients & actual,
                     double tolerance);

    void expect_near(const ThermFile::GlazingSystemProperties & expected,
                     const ThermFile::GlazingSystemProperties & actual,
                     double tolerance);

    void expect_near(const ThermFile::IGUSurface & expected, const ThermFile::IGUSurface & actual, double tolerance);

    void expect_near(const ThermFile::IGULayer & expected, const ThermFile::IGULayer & actual, double tolerance);

    void expect_near(const ThermFile::IGUGasCoefficients & expected,
                     const ThermFile::IGUGasCoefficients & actual,
                     double tolerance);

    void expect_near(const ThermFile::IGUGasProperties & expected,
                     const ThermFile::IGUGasProperties & actual,
                     double tolerance);

    void expect_near(const ThermFile::IGUPureGas & expected, const ThermFile::IGUPureGas & actual, double tolerance);

    void expect_near(const ThermFile::IGUGas & expected, const ThermFile::IGUGas & actual, double tolerance);

    void expect_near(const ThermFile::ShadeProperties & expected,
                     const ThermFile::ShadeProperties & actual,
                     double tolerance);

    void
      expect_near(const ThermFile::IGUShadeLayer & expected, const ThermFile::IGUShadeLayer & actual, double tolerance);

    void
      expect_near(const ThermFile::GlazingSystem & expected, const ThermFile::GlazingSystem & actual, double tolerance);

    //////////////////////////////////////////////////////////////////////////////////////////
    //// Helper functions to fill the data
    //////////////////////////////////////////////////////////////////////////////////////////

    struct MockEnvironmentCoefficients
    {
        std::string h;
        std::string hc;
        std::string hr;
        std::string temperature;

        MockEnvironmentCoefficients(std::string h, std::string hc, std::string hr, std::string temperature);
    };

    MockNode generateEnvironmentCoefficients(const MockEnvironmentCoefficients & environmentCoefficients);

    struct MockGlazingSystemProperties
    {
        std::string environmentalCondition;
        std::string boundaryConditionStyle;
        std::string uValue;
        std::map<std::string, MockEnvironmentCoefficients, std::less<>> coefficients;
        std::string solar;

        MockGlazingSystemProperties(std::string environmentalCondition,
                                    std::string boundaryConditionStyle,
                                    std::string uValue,
                                    std::map<std::string, MockEnvironmentCoefficients, std::less<>> coefficients,
                                    std::string solar);
    };

    MockNode generateGlazingSystemProperties(const MockGlazingSystemProperties & glazingSystemProperties);

    struct MockIGUSurface
    {
        std::string temperature;
        std::string emissivity;

        MockIGUSurface(std::string temperature, std::string emissivity);
    };

    MockNode generateIGUSurface(const MockIGUSurface & iguSurface);

    struct MockIGULayer
    {
        std::string ID;
        std::string name;
        std::string thickness;
        std::string conductivity;
        MockIGUSurface surface1;
        MockIGUSurface surface2;

        MockIGULayer(std::string ID,
                     std::string name,
                     std::string thickness,
                     std::string conductivity,
                     MockIGUSurface surface1,
                     MockIGUSurface surface2);
    };

    MockNode generateIGULayer(const MockIGULayer & iguLayer);

    struct MockIGUGasCoefficients
    {
        std::string A;
        std::string B;
        std::string C;

        MockIGUGasCoefficients(std::string A, std::string B, std::string C);
    };

    MockNode generateIGUGasCoefficients(const MockIGUGasCoefficients & iguGasCoefficients);

    struct MockIGUGasProperties
    {
        std::string name;
        std::string molecularWeight;
        MockIGUGasCoefficients conductivity;
        MockIGUGasCoefficients viscosity;
        MockIGUGasCoefficients specificHeat;

        MockIGUGasProperties(std::string name,
                             std::string molecularWeight,
                             MockIGUGasCoefficients conductivity,
                             MockIGUGasCoefficients viscosity,
                             MockIGUGasCoefficients specificHeat);
    };

    MockNode generateIGUGasProperties(const MockIGUGasProperties & iguGasProperties);

    struct MockIGUPureGas
    {
        std::string fraction;
        MockIGUGasProperties properties;

        MockIGUPureGas(std::string fraction, MockIGUGasProperties properties);
    };

    MockNode generateIGUPureGas(const MockIGUPureGas & iguPureGas);

    struct MockIGUGas
    {
        std::string ID;
        std::string name;
        std::string thickness;
        std::string conductivity;
        std::vector<MockIGUPureGas> components;

        MockIGUGas(std::string ID,
                   std::string name,
                   std::string thickness,
                   std::string conductivity,
                   std::vector<MockIGUPureGas> components);
    };

    MockNode generateIGUGas(const MockIGUGas & iguGas);

    struct MockShadeProperties
    {
        std::string dBottom;
        std::string dTop;
        std::string dLeft;
        std::string dRight;
        std::string gapKeff;

        MockShadeProperties(
          std::string dBottom, std::string dTop, std::string dLeft, std::string dRight, std::string gapKeff);
    };

    MockNode generateShadeProperties(const MockShadeProperties & shadeProperties);

    struct MockIGUShadeLayer
    {
        std::string position;
        std::string emissivityRatio;
        std::string hcRatio;
        std::string thickness;
        std::string gapThickness;
        std::string modeledHeight;
        MockShadeProperties properties;

        MockIGUShadeLayer(std::string position,
                          std::string emissivityRatio,
                          std::string hcRatio,
                          std::string thickness,
                          std::string gapThickness,
                          std::string modeledHeight,
                          MockShadeProperties properties);
    };

    MockNode generateIGUShadeLayer(const MockIGUShadeLayer & iguShadeLayer);

    struct MockGlazingSystem
    {
        std::string ID;
        std::string index;
        std::string name;
        std::vector<MockGlazingSystemProperties> properties;
        std::vector<MockIGULayer> layers;
        std::vector<MockIGUGas> gases;
        std::optional<MockIGUShadeLayer> shadeLayer;
        std::string actualHeight;
        std::string orientation;
        std::string isNominalThickness;
        std::string interiorBC;
        std::string exteriorBC;
        std::string optionFlag;
        std::string source;
        std::string siteLineHeight;
        std::string eogHeight;
        std::string sightLineToEdge;
        std::string isDeflected;
        std::string edgeGlassTag;

        MockGlazingSystem(std::string ID,
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
                          std::string edgeGlassTag);
    };

    MockNode generateGlazingSystem(const MockGlazingSystem & glazingSystem);
}   // namespace Helper