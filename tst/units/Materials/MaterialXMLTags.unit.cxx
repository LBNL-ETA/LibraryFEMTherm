#include <gtest/gtest.h>

#include "Materials/Materials.hxx"
#include "Materials/Tags.hxx"

class TestMaterialXMLTags : public testing::Test
{
protected:

    void SetUp() override
    {}
};

TEST_F(TestMaterialXMLTags, SolidTag)
{
    const auto result{MaterialsLibrary::solidString()};
    const auto expected{"Solid"};

    EXPECT_EQ(expected, result);
}

TEST_F(TestMaterialXMLTags, CavityTag)
{
    const auto result{MaterialsLibrary::cavityString()};
    const auto expected{"Cavity"};

    EXPECT_EQ(expected, result);
}

TEST_F(TestMaterialXMLTags, OpticalTag)
{
    const auto result{MaterialsLibrary::opticalString()};
    const auto expected{"Optical"};

    EXPECT_EQ(expected, result);
}

TEST_F(TestMaterialXMLTags, EnclosureRadiationTag)
{
    const auto result{MaterialsLibrary::enclosureString()};
    const auto expected{"RadiationEnclosure"};

    EXPECT_EQ(expected, result);
}

TEST_F(TestMaterialXMLTags, MaterialRougnessToTag)
{
    const auto result{MaterialsLibrary::materialRoughnessToString(MaterialsLibrary::MaterialRoughness::MediumRough)};

    const auto expected{"Medium Rough"};

    EXPECT_EQ(expected, result);
}

TEST_F(TestMaterialXMLTags, MaterialRougnessFromTagExists)
{
    const auto result{MaterialsLibrary::materialRoughnessFromString("Very Smooth")};

    const auto expected{MaterialsLibrary::MaterialRoughness::VerySmooth};

    EXPECT_EQ(expected, result);
}

TEST_F(TestMaterialXMLTags, MaterialRougnessFromTagDoNotExists)
{
    const auto result{MaterialsLibrary::materialRoughnessFromString("Some random text")};

    const auto expected{MaterialsLibrary::MaterialRoughness::VeryRough};

    EXPECT_EQ(expected, result);
}

TEST_F(TestMaterialXMLTags, CavityStandardToTag)
{
    const auto result{MaterialsLibrary::cavityStandardToString(MaterialsLibrary::CavityStandard::NFRCWithUserDimensions)};

    const auto expected{"NFRCWithUserDimensions"};

    EXPECT_EQ(expected, result);
}

TEST_F(TestMaterialXMLTags, CavityStandardFromTagExists)
{
    const auto result{MaterialsLibrary::cavityStandardFromString("ISO15099Ventilated")};

    const auto expected{MaterialsLibrary::CavityStandard::ISO15099Ventilated};

    EXPECT_EQ(expected, result);
}

TEST_F(TestMaterialXMLTags, CavityStandardFromTagDoNotExists)
{
    const auto result{MaterialsLibrary::cavityStandardFromString("Some random text")};

    const auto expected{MaterialsLibrary::CavityStandard::NFRC};

    EXPECT_EQ(expected, result);
}

TEST_F(TestMaterialXMLTags, FrontSide)
{
    const auto result{MaterialsLibrary::frontSideString()};
    const auto expected{"Front"};

    EXPECT_EQ(expected, result);
}

TEST_F(TestMaterialXMLTags, BackSide)
{
    const auto result{MaterialsLibrary::backSideString()};
    const auto expected{"Back"};

    EXPECT_EQ(expected, result);
}

TEST_F(TestMaterialXMLTags, DirectString)
{
    const auto result{MaterialsLibrary::directString()};
    const auto expected{"Direct"};

    EXPECT_EQ(expected, result);
}

TEST_F(TestMaterialXMLTags, DiffuseString)
{
    const auto result{MaterialsLibrary::diffuseString()};
    const auto expected{"Diffuse"};

    EXPECT_EQ(expected, result);
}

TEST_F(TestMaterialXMLTags, SolarString)
{
    const auto result{MaterialsLibrary::solarString()};
    const auto expected{"Solar"};

    EXPECT_EQ(expected, result);
}

TEST_F(TestMaterialXMLTags, VisibleString)
{
    const auto result{MaterialsLibrary::visibleString()};
    const auto expected{"Visible"};

    EXPECT_EQ(expected, result);
}