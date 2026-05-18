#include <gtest/gtest.h>

#include "Materials/Materials.hxx"
#include "Materials/Operators.hxx"

#include "material1.hxx"

class TestMaterialsXMLOperators : public testing::Test
{
protected:
    void SetUp() override
    {}
};

TEST_F(TestMaterialsXMLOperators, EqualityOperator1)
{
    MaterialsLibrary::Material material1 =
      MaterialsLibrary::generate("7a863ad6-c537-11ea-87d0-0242ac130003");
    MaterialsLibrary::Material material2 =
      MaterialsLibrary::generate("7a863ad6-c537-11ea-87d0-0242ac130003");

    const auto bulkDensity{800.0};

    auto * solid1 = &material1.data;
    auto * solid2 = &material2.data;

    ASSERT_TRUE(solid1->hygroThermal.has_value());
    ASSERT_TRUE(solid2->hygroThermal.has_value());

    solid1->hygroThermal->BulkDensity = bulkDensity;
    solid2->hygroThermal->BulkDensity = bulkDensity;

    const auto result{*solid1 == *solid2};
    EXPECT_EQ(result, true);
}

TEST_F(TestMaterialsXMLOperators, EqualityOperator2)
{
    MaterialsLibrary::Material material1 =
      MaterialsLibrary::generate("7a863ad6-c537-11ea-87d0-0242ac130003");
    MaterialsLibrary::Material material2 =
      MaterialsLibrary::generate("7a863ad6-c537-11ea-87d0-0242ac130003");

    const auto bulkDensity{800.0};

    auto * solid1 = &material1.data;
    auto * solid2 = &material2.data;

    ASSERT_TRUE(solid1->hygroThermal.has_value());

    solid1->hygroThermal->BulkDensity = bulkDensity;

    const auto result{*solid1 == *solid2};
    EXPECT_EQ(result, false);
}

TEST_F(TestMaterialsXMLOperators, EqualityOperator3)
{
    MaterialsLibrary::Material material1 =
      MaterialsLibrary::generate("7a863ad6-c537-11ea-87d0-0242ac130003");
    MaterialsLibrary::Material material2 =
      MaterialsLibrary::generate("7a863ad6-c537-11ea-87d0-0242ac130003");

    const std::vector<MaterialsLibrary::point> someVectorPoints{{0, 0}, {1, 1}};

    auto * solid1 = &material1.data;
    auto * solid2 = &material2.data;

    ASSERT_TRUE(solid1->hygroThermal.has_value());
    ASSERT_TRUE(solid2->hygroThermal.has_value());

    solid1->hygroThermal->MoistureStorageFunction = someVectorPoints;
    solid2->hygroThermal->MoistureStorageFunction = someVectorPoints;

    const auto result{*solid1 == *solid2};
    EXPECT_EQ(result, true);
}

TEST_F(TestMaterialsXMLOperators, EqualityOperator4)
{
    MaterialsLibrary::Material material1 =
      MaterialsLibrary::generate("7a863ad6-c537-11ea-87d0-0242ac130003");
    MaterialsLibrary::Material material2 =
      MaterialsLibrary::generate("7a863ad6-c537-11ea-87d0-0242ac130003");

    const std::vector<MaterialsLibrary::point> someVectorPoints{{0, 0}, {1, 1}};

    auto * solid1 = &material1.data;
    auto * solid2 = &material2.data;

    ASSERT_TRUE(solid1->hygroThermal.has_value());
    ASSERT_TRUE(solid2->hygroThermal.has_value());

    solid1->hygroThermal->MoistureStorageFunction = someVectorPoints;

    const auto result{*solid1 == *solid2};
    EXPECT_EQ(result, false);
}

TEST_F(TestMaterialsXMLOperators, EqualityOperator5)
{
    MaterialsLibrary::Material material1 =
      MaterialsLibrary::generate("7a863ad6-c537-11ea-87d0-0242ac130003");
    MaterialsLibrary::Material material2 =
      MaterialsLibrary::generate("7a863ad6-c537-11ea-87d0-0242ac130003");

    const std::vector<MaterialsLibrary::point> someVectorPoints1{{0, 0}, {1, 1}};
    const std::vector<MaterialsLibrary::point> someVectorPoints2{{0, 0}, {2, 2}};

    auto * solid1 = &material1.data;
    auto * solid2 = &material2.data;

    ASSERT_TRUE(solid1->hygroThermal.has_value());
    ASSERT_TRUE(solid2->hygroThermal.has_value());

    solid1->hygroThermal->MoistureStorageFunction = someVectorPoints1;
    solid2->hygroThermal->MoistureStorageFunction = someVectorPoints2;

    const auto result{*solid1 == *solid2};
    EXPECT_EQ(result, false);
}