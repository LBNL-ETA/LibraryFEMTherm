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
    ASSERT_FALSE(exposureType.assemblyType.has_value());
}

TEST_F(TestFilePropertiesUtilities, CreateExposureTypeOpaqueFacade)
{
    auto exposureType = createExposureType(ThermFile::ModelPurpose::OpaqueFacade);
    ASSERT_EQ(std::get<ThermFile::OpaqueCrossSectionType>(exposureType.crossSection),
              ThermFile::OpaqueCrossSectionType::SillPlate);
    ASSERT_TRUE(exposureType.assemblyType.has_value());
    ASSERT_EQ(exposureType.assemblyType.value(), ThermFile::OpaqueAssemblyType::Wall);
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

TEST_F(TestFilePropertiesUtilities, CreateModelExposureWindow)
{
    auto modelExposure = createModelExposure(ThermFile::ModelPurpose::Window_TransparentFacade);

    ASSERT_EQ(std::get<ThermFile::WindowCrossSectionType>(modelExposure.exposureType.crossSection),
              ThermFile::WindowCrossSectionType::Sill);
    ASSERT_FALSE(modelExposure.exposureType.assemblyType.has_value());
}

TEST_F(TestFilePropertiesUtilities, CreateModelExposureOpaque)
{
    auto modelExposure = createModelExposure(ThermFile::ModelPurpose::OpaqueFacade);

    ASSERT_EQ(std::get<ThermFile::OpaqueCrossSectionType>(modelExposure.exposureType.crossSection),
              ThermFile::OpaqueCrossSectionType::SillPlate);
    ASSERT_TRUE(modelExposure.exposureType.assemblyType.has_value());
    ASSERT_EQ(modelExposure.exposureType.assemblyType.value(), ThermFile::OpaqueAssemblyType::Wall);
}

TEST_F(TestFilePropertiesUtilities, CreateModelExposureOther)
{
    auto modelExposure = createModelExposure(ThermFile::ModelPurpose::Other);

    ASSERT_EQ(std::get<ThermFile::OtherCrossSectionType>(modelExposure.exposureType.crossSection),
              ThermFile::OtherCrossSectionType::GeneralCrossSection);
    ASSERT_FALSE(modelExposure.exposureType.assemblyType.has_value());
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