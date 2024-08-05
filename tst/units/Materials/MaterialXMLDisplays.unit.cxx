#include <gtest/gtest.h>

#include "Materials/Materials.hxx"
#include "Materials/Tags.hxx"

class TestMaterialXMLDisplays : public testing::Test
{
protected:

    void SetUp() override
    {}
};

TEST_F(TestMaterialXMLDisplays, CavityToDisplayString)
{
    const auto result{MaterialsLibrary::cavityStandardToDisplayString(MaterialsLibrary::CavityStandard::CENVentilated)};

    const auto expected{"CEN (slightly ventilated)"};

    EXPECT_EQ(expected, result);
}

TEST_F(TestMaterialXMLDisplays, DisplayStringToCavity)
{
    const auto result{MaterialsLibrary::cavityStandardFromDisplayString("NFRC with user dimensions")};

    const auto expected{MaterialsLibrary::CavityStandard::NFRCWithUserDimensions};

    EXPECT_EQ(expected, result);
}

TEST_F(TestMaterialXMLDisplays, DisplayStringToCavityDoNotExists)
{
    const auto result{MaterialsLibrary::cavityStandardFromDisplayString("Some random string")};
    const auto expected{MaterialsLibrary::CavityStandard::NFRC};

    EXPECT_EQ(expected, result);
}