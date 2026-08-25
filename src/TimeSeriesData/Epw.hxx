#pragma once

#include <string>
#include <vector>

#include <lbnl/expected.hxx>

#include "TimeSeriesData.hxx"

namespace TimeSeriesLibrary::Epw
{
    //! EnergyPlus weather import, following the interactive mockup's epw.py.
    //!
    //! Five roles come out of a weather file: AirTemperature (dry bulb),
    //! RelativeHumidity (per cent in the file, a fraction here), WindSpeed,
    //! WindDirection, and RadiantTemperature - the last derived from horizontal
    //! infrared intensity, which is allowed because the sky is the sky no matter
    //! which way a surface faces. Solar irradiance cannot join it: what a surface
    //! receives depends on tilt, azimuth and solar position, so it must be computed
    //! per surface at run time.
    //!
    //! Missing readings (the field's sentinel value) carry the last good reading
    //! forward; a field with no good reading at all produces no channel. Fields the
    //! role vocabulary cannot express are reported by name so the omission is
    //! visible rather than silent.

    struct Location
    {
        std::string city;
        std::string region;
        std::string country;
        std::string wmo;
        double latitude{0};
        double longitude{0};
        double timeZone{0};
        double elevation{0};
    };

    struct ImportResult
    {
        TimeSeriesData data;
        Location location;
        //! Fields the file carried that no channel role can express, as
        //! "Label [unit]" strings for display.
        std::vector<std::string> unmappedColumns;
    };

    //! Sky temperature [degC] from horizontal infrared intensity [W/m2] by
    //! Stefan-Boltzmann. Surface-independent, which is what makes it storable as a
    //! channel when surface solar irradiance is not.
    [[nodiscard]] std::vector<double> skyTemperature(const std::vector<double> & horizontalInfrared);

    //! The imported dataset's UUID is the deterministic content hash.
    [[nodiscard]] lbnl::ExpectedExt<ImportResult, std::string>
      readFromString(const std::string & content, const std::string & datasetName);

    //! Reads the file and imports it under its stem as the dataset name.
    [[nodiscard]] lbnl::ExpectedExt<ImportResult, std::string> read(const std::string & fileName);
}   // namespace TimeSeriesLibrary::Epw
