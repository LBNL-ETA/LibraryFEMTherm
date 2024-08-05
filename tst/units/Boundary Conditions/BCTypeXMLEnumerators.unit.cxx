#include <gtest/gtest.h>

#include "BCTransient/Types.hxx"
#include "BCTransient/TypesTags.hxx"
#include "LibraryUtilities/FileManipulation.hxx"

#include "bctype1.hxx"

class TestBCTypeXMLEnumerators : public testing::Test
{
protected:

    void SetUp() override
    {}
};

TEST_F(TestBCTypeXMLEnumerators, BoundaryConditionType)
{
    BCTypesLibrary::StringTags tags;
    const auto stringTransient{tags.stringBCType(BCTypesLibrary::BCType::Transient)};
    EXPECT_EQ(stringTransient, "Transient");

    const auto stringSteadyState{tags.stringBCType(BCTypesLibrary::BCType::SteadyState)};
    EXPECT_EQ(stringSteadyState, "SteadyState");

    const auto vectorBCType{tags.stringsBCType()};
    EXPECT_EQ(vectorBCType.size(), 2u);
    EXPECT_EQ(vectorBCType[0], "Transient");
    EXPECT_EQ(vectorBCType[1], "SteadyState");
}

TEST_F(TestBCTypeXMLEnumerators, BoundaryConditionModel)
{
    const auto stringNeumann{BCTypesLibrary::bcModelToString(BCTypesLibrary::BCModel::Neumann)};
    EXPECT_EQ(stringNeumann, "Neumann");

    const auto stringDirichlet{BCTypesLibrary::bcModelToString(BCTypesLibrary::BCModel::Dirichlet)};
    EXPECT_EQ(stringDirichlet, "Dirichlet");

    BCTypesLibrary::StringTags tags;
    const auto vectorBCModel{tags.stringsBCModel()};
    EXPECT_EQ(vectorBCModel.size(), 2u);
    EXPECT_EQ(vectorBCModel[0], "Neumann");
    EXPECT_EQ(vectorBCModel[1], "Dirichlet");
}

TEST_F(TestBCTypeXMLEnumerators, BoundaryConditionConvectionModel)
{
    const auto stringASHRAEEInside{BCTypesLibrary::convectionModelToString(BCTypesLibrary::ConvectionModel::ASHRAE_NFRC_Inside)};
    EXPECT_EQ(stringASHRAEEInside, "ASHRAE/NFRC Inside");

    const auto stringASHRAEEOutside{BCTypesLibrary::convectionModelToString(BCTypesLibrary::ConvectionModel::ASHRAE_NFRC_Outside)};
    EXPECT_EQ(stringASHRAEEOutside, "ASHRAE/NFRC Outside");

    const auto stringFixedFilm{
      BCTypesLibrary::convectionModelToString(BCTypesLibrary::ConvectionModel::Fixed_Convection_Coefficient)};
    EXPECT_EQ(stringFixedFilm, "Fixed Convection Coefficient");

    const auto stringWUFI{BCTypesLibrary::convectionModelToString(BCTypesLibrary::ConvectionModel::TARP)};
    EXPECT_EQ(stringWUFI, "TARP");

    const auto stringYazdanianKlems{BCTypesLibrary::convectionModelToString(BCTypesLibrary::ConvectionModel::Yazdanian_Klems)};
    EXPECT_EQ(stringYazdanianKlems, "Yazdanian Klems");

    const auto stringKimura{BCTypesLibrary::convectionModelToString(BCTypesLibrary::ConvectionModel::Kimura)};
    EXPECT_EQ(stringKimura, "Kimura");

    BCTypesLibrary::StringTags tags;

    const auto vectorConvectionModel{tags.stringsConvectionModel()};
    EXPECT_EQ(vectorConvectionModel.size(), 7u);
    EXPECT_EQ(vectorConvectionModel[0], "ASHRAE/NFRC Inside");
    EXPECT_EQ(vectorConvectionModel[1], "ASHRAE/NFRC Outside");
    EXPECT_EQ(vectorConvectionModel[2], "Fixed Convection Coefficient");
    EXPECT_EQ(vectorConvectionModel[3], "TARP");
    EXPECT_EQ(vectorConvectionModel[4], "Kimura");
    EXPECT_EQ(vectorConvectionModel[5], "Yazdanian Klems");    
    EXPECT_EQ(vectorConvectionModel[6], "Montazeri");    
}

TEST_F(TestBCTypeXMLEnumerators, BoundaryConditionConvectionModelFromString)
{
    const std::string stringASHRAEEInside{"ASHRAE/NFRC Inside"};
    const auto ASHRAEEInside{BCTypesLibrary::convectionModelFromString(stringASHRAEEInside)};
    EXPECT_EQ(ASHRAEEInside, BCTypesLibrary::ConvectionModel::ASHRAE_NFRC_Inside);

    const std::string stringASHRAEEOutside{"ASHRAE/NFRC Outside"};
    const auto ASHRAEEOutside{BCTypesLibrary::convectionModelFromString(stringASHRAEEOutside)};
    EXPECT_EQ(ASHRAEEOutside, BCTypesLibrary::ConvectionModel::ASHRAE_NFRC_Outside);

    const std::string stringFixedFilm{"Fixed Convection Coefficient"};
    const auto FixedFilm{BCTypesLibrary::convectionModelFromString(stringFixedFilm)};
    EXPECT_EQ(FixedFilm, BCTypesLibrary::ConvectionModel::Fixed_Convection_Coefficient);

    const std::string stringWUFI{"TARP"};
    const auto WUFI{BCTypesLibrary::convectionModelFromString(stringWUFI)};
    EXPECT_EQ(WUFI, BCTypesLibrary::ConvectionModel::TARP);

    const std::string stringYazdanianKlems{"Yazdanian Klems"};
    const auto YazdanianKlems{BCTypesLibrary::convectionModelFromString(stringYazdanianKlems)};
    EXPECT_EQ(YazdanianKlems, BCTypesLibrary::ConvectionModel::Yazdanian_Klems);

    const std::string stringKimura{"Kimura"};
    const auto Kimura{BCTypesLibrary::convectionModelFromString(stringKimura)};
    EXPECT_EQ(Kimura, BCTypesLibrary::ConvectionModel::Kimura);
}

TEST_F(TestBCTypeXMLEnumerators, BoundaryConditionRadiationModel)
{
    const auto stringAutomaticEnclosure{
      BCTypesLibrary::radiationModelToString(BCTypesLibrary::RadiationModel::Automatic_Enclosure)};
    EXPECT_EQ(stringAutomaticEnclosure, "Automatic Enclosure");

    const auto stringBlackBodyRadiation{
      BCTypesLibrary::radiationModelToString(BCTypesLibrary::RadiationModel::Black_Body_Radiation)};
    EXPECT_EQ(stringBlackBodyRadiation, "Black Body Radiation");

    const auto stringFixedCoefficient{
      BCTypesLibrary::radiationModelToString(BCTypesLibrary::RadiationModel::Fixed_Radiation_Coefficient)};
    EXPECT_EQ(stringFixedCoefficient, "Fixed Radiation Coefficient");

    BCTypesLibrary::StringTags tags;
    const auto vectorRadiationModel{tags.stringsRadiationModel()};
    EXPECT_EQ(vectorRadiationModel.size(), 3u);
    EXPECT_EQ(vectorRadiationModel[0], "Automatic Enclosure");
    EXPECT_EQ(vectorRadiationModel[1], "Black Body Radiation");
    EXPECT_EQ(vectorRadiationModel[2], "Fixed Radiation Coefficient");
}

TEST_F(TestBCTypeXMLEnumerators, Direction)
{
    const auto stringWindward{BCTypesLibrary::directionToString(BCTypesLibrary::Direction::Windward)};
    EXPECT_EQ(stringWindward, "Windward");

    const auto stringLeeward{BCTypesLibrary::directionToString(BCTypesLibrary::Direction::Leeward)};
    EXPECT_EQ(stringLeeward, "Leeward");
}

TEST_F(TestBCTypeXMLEnumerators, DirectionFromString)
{
    const std::string stringWindward{"Windward"};
    const auto Windward{BCTypesLibrary::directionFromString(stringWindward)};
    EXPECT_EQ(Windward, BCTypesLibrary::Direction::Windward);

    const std::string stringLeeward{"Leeward"};
    const auto Leeward{BCTypesLibrary::directionFromString(stringLeeward)};
    EXPECT_EQ(Leeward, BCTypesLibrary::Direction::Leeward);

    BCTypesLibrary::StringTags tags;
    const auto vectorDirection{tags.stringsDirection()};
    EXPECT_EQ(vectorDirection.size(), 2u);
    EXPECT_EQ(vectorDirection[0], "Leeward");
    EXPECT_EQ(vectorDirection[1], "Windward");    
}