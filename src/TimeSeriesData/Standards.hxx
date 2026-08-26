#pragma once

#include <string>

#include <lbnl/expected.hxx>

#include "TimeSeriesData.hxx"

namespace TimeSeriesLibrary::Standards
{
    //! Interior conditions generated from an exterior climate, following the interactive
    //! mockup's standards.py. The standards all answer the same question - what is
    //! happening indoors, given what is happening outdoors - and all of them answer it
    //! from the outdoor DAILY MEAN rather than the hourly value, because a building's
    //! interior does not follow the weather hour by hour. The interior inherits the
    //! exterior's step count exactly; there is no generation length to choose.
    //!
    //! CONSTANTS NEED CHECKING: the piecewise shapes match the reference tools, but the
    //! breakpoints are written from working knowledge, not transcribed from the standard
    //! texts. Every number lives in one table in the implementation file.

    enum class Standard
    {
        EN_15026_DIN_4108,
        ISO_13788,
        ASHRAE_160
    };

    enum class MoistureLoad
    {
        Normal,
        High
    };

    [[nodiscard]] std::string standardToString(Standard standard);
    [[nodiscard]] std::string moistureLoadToString(MoistureLoad load);

    //! Derives indoor AirTemperature and RelativeHumidity from the exterior dataset, one
    //! value per exterior step. The exterior must carry AirTemperature; a missing
    //! RelativeHumidity is taken as 0.7 (only ISO 13788 reads it). The result's UUID is
    //! the deterministic content hash and its name follows the library convention:
    //! "<exterior> interior (<standard>, <load>)", with a trailing " exterior" on the
    //! source name replaced rather than repeated.
    [[nodiscard]] lbnl::ExpectedExt<TimeSeriesData, std::string>
      generateInterior(const TimeSeriesData & exterior,
                       Standard standard,
                       MoistureLoad load,
                       std::size_t stepsPerDay = 24U);
}   // namespace TimeSeriesLibrary::Standards
