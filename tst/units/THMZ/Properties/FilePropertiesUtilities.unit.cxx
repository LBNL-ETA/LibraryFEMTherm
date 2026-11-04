#include "gtest/gtest.h"

#include "THMZ/Properties/Properties.hxx"
#include "THMZ/Properties/Utilities.hxx"

class TestFilePropertiesUtilities : public testing::Test
{};

TEST_F(TestFilePropertiesUtilities, CreateExposureTypeWindowTransparentFacade)
{
    auto exposureType = createExposureType(ThermFile::ModelPurpose::Window_TransparentFacade);
    ASSERT_EQ(std::get<ThermFile::WindowCrossSectionType>(exposureType.crossSection),
              ThermFile::WindowCrossSectionType::Sill);
    ASSERT_TRUE(exposureType.assemblyType.has_value());
    ASSERT_TRUE(exposureType.assemblyType.value() == ThermFile::AssemblyType{ThermFile::WindowAssemblyType::SingleLite});
}

TEST_F(TestFilePropertiesUtilities, CreateExposureTypeOpaqueFacade)
{
    auto exposureType = createExposureType(ThermFile::ModelPurpose::OpaqueFacade);
    ASSERT_EQ(std::get<ThermFile::OpaqueCrossSectionType>(exposureType.crossSection),
              ThermFile::OpaqueCrossSectionType::SillPlate);
    ASSERT_TRUE(exposureType.assemblyType.has_value());
    ASSERT_TRUE(exposureType.assemblyType.value() == ThermFile::AssemblyType{ThermFile::OpaqueAssemblyType::Wall});
}

TEST_F(TestFilePropertiesUtilities, CreateExposureTypeOther)
{
    auto exposureType = createExposureType(ThermFile::ModelPurpose::Other);
    ASSERT_EQ(std::get<ThermFile::OtherCrossSectionType>(exposureType.crossSection),
              ThermFile::OtherCrossSectionType::GeneralCrossSection);
    ASSERT_FALSE(exposureType.assemblyType.has_value());
}

// Test exception is thrown for unsupported ModelPurpose
TEST_F(TestFilePropertiesUtilities, CreateExposureTypeUnsupported)
{
    ASSERT_THROW((void)createExposureType(static_cast<ThermFile::ModelPurpose>(999)), std::logic_error);
}

TEST_F(TestFilePropertiesUtilities, CreateModelDetailsWindow)
{
    auto modelDetails = createModelDetails(ThermFile::ModelPurpose::Window_TransparentFacade);

    ASSERT_EQ(std::get<ThermFile::WindowCrossSectionType>(modelDetails.exposureType.crossSection),
              ThermFile::WindowCrossSectionType::Sill);
    ASSERT_TRUE(modelDetails.exposureType.assemblyType.has_value());
    ASSERT_TRUE(modelDetails.exposureType.assemblyType.value()
                == ThermFile::AssemblyType{ThermFile::WindowAssemblyType::SingleLite});
    EXPECT_DOUBLE_EQ(modelDetails.building.width, 20.0);
    EXPECT_DOUBLE_EQ(modelDetails.building.height, 5.0);
}

TEST_F(TestFilePropertiesUtilities, CreateModelDetailsOpaque)
{
    auto modelDetails = createModelDetails(ThermFile::ModelPurpose::OpaqueFacade);

    ASSERT_EQ(std::get<ThermFile::OpaqueCrossSectionType>(modelDetails.exposureType.crossSection),
              ThermFile::OpaqueCrossSectionType::SillPlate);
    ASSERT_TRUE(modelDetails.exposureType.assemblyType.has_value());
    ASSERT_TRUE(modelDetails.exposureType.assemblyType.value()
                == ThermFile::AssemblyType{ThermFile::OpaqueAssemblyType::Wall});
}

TEST_F(TestFilePropertiesUtilities, CreateModelDetailsOther)
{
    auto modelDetails = createModelDetails(ThermFile::ModelPurpose::Other);

    ASSERT_EQ(std::get<ThermFile::OtherCrossSectionType>(modelDetails.exposureType.crossSection),
              ThermFile::OtherCrossSectionType::GeneralCrossSection);
    ASSERT_FALSE(modelDetails.exposureType.assemblyType.has_value());
}

TEST_F(TestFilePropertiesUtilities, GetMeshParameterLimitsQuadTreeMesher)
{
    auto limits = getMeshParameterLimits(ThermFile::MesherType::QuadTreeMesher);
    ASSERT_EQ(limits.minimum, 3u);
    ASSERT_EQ(limits.maximum, 12u);
}

TEST_F(TestFilePropertiesUtilities, GetMeshParameterLimitsSimmetrixVersion2022)
{
    auto limits = getMeshParameterLimits(ThermFile::MesherType::Simmetrix_Version_2022);
    ASSERT_EQ(limits.minimum, 3u);
    ASSERT_EQ(limits.maximum, 100u);
}

TEST_F(TestFilePropertiesUtilities, GetIterationLimits)
{
    auto limits = ThermFile::getIterationLimits();
    ASSERT_EQ(limits.minimum, 1u);
    ASSERT_EQ(limits.maximum, 30000u);
}