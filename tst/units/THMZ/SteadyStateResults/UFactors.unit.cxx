#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "THMZ/SteadyStateResults/DB.hxx"
#include "THMZ/SteadyStateResults/UFactors.hxx"

namespace
{
    using ThermFile::Projection;
    using ThermFile::UValueDimensionType;

    [[nodiscard]] ThermFile::SteadyStateUFactors frame()
    {
        return {.tag = "Frame",
                .deltaT = 21.0,
                .heatFlux = 9.0,
                .defaultDisplayType = UValueDimensionType::XProjected,
                .projections = {{.lengthType = UValueDimensionType::TotalLength, .length = 120.0, .uFactor = 1.5},
                                {.lengthType = UValueDimensionType::XProjected, .length = 100.0, .uFactor = 1.8},
                                {.lengthType = UValueDimensionType::CustomRotationProjected,
                                 .length = 5e-15,
                                 .uFactor = 1e16,
                                 .projectionAngle = 0.0},
                                {.lengthType = UValueDimensionType::Custom}}};
    }
}   // namespace

TEST(UFactors, ProjectionOfALengthType)
{
    const auto projection{ThermFile::projectionOf(frame(), UValueDimensionType::TotalLength)};
    ASSERT_TRUE(projection.has_value());
    EXPECT_DOUBLE_EQ(projection->uFactor.value(), 1.5);
    EXPECT_FALSE(ThermFile::projectionOf(frame(), UValueDimensionType::YProjected).has_value());
}

TEST(UFactors, TheDisplayedProjectionIsTheDefaultDisplayType)
{
    const auto projection{ThermFile::displayedProjection(frame())};
    ASSERT_TRUE(projection.has_value());
    EXPECT_EQ(projection->lengthType, UValueDimensionType::XProjected);
    EXPECT_DOUBLE_EQ(ThermFile::displayedUFactor(frame()).value(), 1.8);
}

TEST(UFactors, AZeroAngleRotationIsDegenerateAndNotReportable)
{
    const auto rotation{ThermFile::projectionOf(frame(), UValueDimensionType::CustomRotationProjected).value()};
    EXPECT_TRUE(ThermFile::isDegenerate(rotation));
    EXPECT_FALSE(ThermFile::isReportable(rotation));

    auto shown{frame()};
    shown.defaultDisplayType = UValueDimensionType::CustomRotationProjected;
    EXPECT_FALSE(ThermFile::displayedUFactor(shown).has_value());
}

TEST(UFactors, AnEmptyProjectionIsNeitherDegenerateNorReportable)
{
    const auto custom{ThermFile::projectionOf(frame(), UValueDimensionType::Custom).value()};
    EXPECT_FALSE(ThermFile::isDegenerate(custom));
    EXPECT_FALSE(ThermFile::isReportable(custom));
}

TEST(UFactors, EveryTagOfTheSillShowsAUFactor)
{
    std::filesystem::path path{TEST_DATA_DIR};
    path /= "products";
    path /= "sample-sill.thmz";
    const auto results{ThermFile::loadSteadyStateResultsFromZipFile(path.string())};
    ASSERT_TRUE(results.has_value());
    ASSERT_FALSE(results->cases.empty());
    for(const auto & tag : results->cases.front().steadyStateUFactors)
    {
        EXPECT_TRUE(ThermFile::displayedUFactor(tag).has_value()) << tag.tag;
    }
}
