#include <filesystem>
#include <gtest/gtest.h>

#include <THMZ/Model/DB.hxx>

TEST(CRUDeserialization, LoadFromTHMZFile)
{
    SCOPED_TRACE("Begin Test: Load sample-sill.thmz and verify CRU properties");

    std::filesystem::path productPath(TEST_DATA_DIR);
    productPath /= "products";
    productPath /= "sample-sill.thmz";

    const auto model = ThermFile::loadThermModelFromZipFile(productPath.string());
    ASSERT_TRUE(model.has_value()) << "Failed to load THMZ model from " << productPath.string();

    const auto & glazingSystems = model.value().glazingSystems;
    ASSERT_FALSE(glazingSystems.empty()) << "Model contains no glazing systems";

    const auto & gases = glazingSystems[0].gases;
    ASSERT_FALSE(gases.empty()) << "Glazing system contains no gases";

    const auto & firstGas = gases[0];
    ASSERT_TRUE(firstGas.crProperties.has_value()) << "First gas has no CRProperties";

    const auto & crp = firstGas.crProperties.value();

    EXPECT_TRUE(crp.calcUnsealedCR);
    EXPECT_FALSE(crp.useCustomRelativeHumidityRatio);
    EXPECT_TRUE(crp.useUnsealedTowardInterior);
    EXPECT_DOUBLE_EQ(0.0, crp.customRelativeHumidityRatio);
    EXPECT_DOUBLE_EQ(0.5, crp.bcRelativeHumidity);
    EXPECT_DOUBLE_EQ(21.0, crp.bcTemperature);
    EXPECT_FALSE(crp.bcName.empty()) << "BC name should not be empty";
}
