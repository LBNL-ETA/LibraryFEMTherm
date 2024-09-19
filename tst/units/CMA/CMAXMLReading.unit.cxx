#include <filesystem>
#include <gtest/gtest.h>

#include "CMA/CMA.hxx"
#include "CMA/DB.hxx"
#include "LibraryUtilities/FileManipulation.hxx"

#include "cma1.hxx"

class TestCMAXMLReading : public testing::Test
{
protected:
    void SetUp() override
    {}
};

void compareLayerOptions(const CMALibrary::LayerOptions & correctMap,
                         const CMALibrary::LayerOptions & loadedMap,
                         double tolerance)
{
    EXPECT_EQ(correctMap.size(), loadedMap.size());

    for(const auto & [key, correctValues] : correctMap)
    {
        auto it = loadedMap.find(key);
        EXPECT_NE(it, loadedMap.end());

        const CMALibrary::LayerValues & loadedValues = it->second;

        EXPECT_NEAR(correctValues.hcin, loadedValues.hcin, tolerance);
        EXPECT_NEAR(correctValues.hcout, loadedValues.hcout, tolerance);
        EXPECT_NEAR(correctValues.emissivityIn, loadedValues.emissivityIn, tolerance);
        EXPECT_NEAR(correctValues.emissivityOut, loadedValues.emissivityOut, tolerance);
    }
}

TEST_F(TestCMAXMLReading, CMAData)
{
    using CMALibrary::Option;
    constexpr auto tolerance{1e-6};
    const std::string fileContent{TestCMA::testDatabase()};
    const std::string fileName{"TestRead.xml"};

    File::createFileFromString(fileName, fileContent);

    auto optData{CMALibrary::loadCMADataFromFile(fileName)};

    ASSERT_TRUE(optData.has_value());

    CMALibrary::CMAData data{optData.value()};

    EXPECT_NEAR(25.5, data.environment.Tin, tolerance);
    EXPECT_NEAR(-14.12, data.environment.Tout, tolerance);

    EXPECT_NEAR(0.234191, data.glazingConductance.at(Option::Low), tolerance);
    EXPECT_NEAR(7.32155, data.glazingConductance.at(Option::High), tolerance);

    EXPECT_NEAR(0.02, data.spacerConductivity.at(Option::Low), tolerance);
    EXPECT_NEAR(30.0, data.spacerConductivity.at(Option::High), tolerance);

    EXPECT_NEAR(0.004, data.singleLayer.thickness, tolerance);
    EXPECT_NEAR(1.004, data.singleLayer.conductivity, tolerance);

    CMALibrary::LayerOptions correctSingleLayerOptionsMap = {{{Option::Low,  Option::Low},  {4.415185, 27, 0.027, 0.85}},
                                                             {{Option::Low,  Option::High}, {5.415185, 28, 0.028, 0.86}},
                                                             {{Option::High, Option::Low},  {6.529954, 29, 0.85,  0.87}},
                                                             {{Option::High, Option::High}, {7.529954, 30, 0.86,  0.88}}};

    compareLayerOptions(correctSingleLayerOptionsMap, data.singleLayer.layerOptions, tolerance);

    EXPECT_NEAR(0.005, data.doubleLayer.thickness1, tolerance);
    EXPECT_NEAR(0.002, data.doubleLayer.thickness2, tolerance);
    EXPECT_NEAR(1.005, data.doubleLayer.conductivity1, tolerance);
    EXPECT_NEAR(1.002, data.doubleLayer.conductivity2, tolerance);

    CMALibrary::LayerOptions correctDoubleLayerOptionsMap{{{Option::Low,  Option::Low},  {2.854252, 37, 0.85, 0.94}},
                                                          {{Option::Low,  Option::High}, {3.854252, 38, 0.86, 0.95}},
                                                          {{Option::High, Option::Low},  {4.866122, 39, 0.87, 0.96}},
                                                          {{Option::High, Option::High}, {5.866122, 40, 0.88, 0.97}}};

    compareLayerOptions(correctDoubleLayerOptionsMap, data.doubleLayer.layerOptions, tolerance);

    std::filesystem::remove(fileName);
}
