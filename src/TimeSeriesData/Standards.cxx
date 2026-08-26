#include <algorithm>
#include <cmath>

#include "ContentHash.hxx"
#include "Standards.hxx"

namespace TimeSeriesLibrary::Standards
{
    namespace
    {
        //! Every constant of every standard, in one place so a corrected breakpoint is a
        //! one-line change. No number is buried in code.
        struct Ramp
        {
            double lowOutdoor;
            double highOutdoor;
            double lowIndoor;
            double highIndoor;
        };

        // EN 15026 / DIN 4108: indoor temperature rises with the outdoor daily mean
        // between two breakpoints and is held flat outside them; indoor humidity does the
        // same over a wider outdoor range. High moisture load shifts humidity up.
        constexpr Ramp enTemperature{10.0, 20.0, 20.0, 22.0};
        constexpr Ramp enHumidity{-10.0, 20.0, 0.30, 0.60};
        constexpr double enHighLoadOffset{0.10};

        // ISO 13788: indoor air held at a setpoint; its vapour content is the outdoor
        // content plus an excess that falls linearly to zero as the outdoor daily mean
        // rises. The excess is the standard's humidity class.
        constexpr double isoIndoorTemperature{20.0};
        constexpr Ramp isoExcessRange{0.0, 20.0, 0.0, 0.0};   // indoor values injected per load
        constexpr double isoExcessNormal{0.004};              // kg/m3 at 0 degC
        constexpr double isoExcessHigh{0.006};

        // ASHRAE 160: heating and cooling setpoints with a linear band between; indoor
        // humidity follows the outdoor daily mean over the same band.
        constexpr Ramp ashraeTemperature{18.3, 21.1, 21.1, 23.9};
        constexpr Ramp ashraeHumidity{-10.0, 20.0, 0.30, 0.55};
        constexpr double ashraeHighLoadOffset{0.10};

        constexpr double humidityFloor{0.05};
        constexpr double humidityCeiling{0.98};
        constexpr double defaultOutdoorHumidity{0.7};

        //! Linear between two breakpoints, flat outside them.
        [[nodiscard]] double ramp(const double value, const Ramp & shape)
        {
            if(value <= shape.lowOutdoor)
            {
                return shape.lowIndoor;
            }
            if(value >= shape.highOutdoor)
            {
                return shape.highIndoor;
            }
            const double span{shape.highOutdoor - shape.lowOutdoor};
            return shape.lowIndoor + (value - shape.lowOutdoor) * (shape.highIndoor - shape.lowIndoor) / span;
        }

        //! Magnus form, Pa. Valid over the range building physics cares about.
        [[nodiscard]] double saturationPressure(const double temperature)
        {
            return 610.94 * std::exp(17.625 * temperature / (temperature + 243.04));
        }

        //! kg/m3 of water vapour in air, from the ideal gas law for the partial pressure.
        [[nodiscard]] double vapourDensity(const double temperature, const double humidity)
        {
            return humidity * saturationPressure(temperature) / (461.5 * (temperature + 273.15));
        }

        //! The inverse: what relative humidity a vapour density represents.
        [[nodiscard]] double relativeHumidityOf(const double temperature, const double density)
        {
            return density * 461.5 * (temperature + 273.15) / saturationPressure(temperature);
        }

        [[nodiscard]] double clampHumidity(const double value)
        {
            return std::clamp(value, humidityFloor, humidityCeiling);
        }

        struct IndoorState
        {
            double temperature;
            double humidity;
        };

        [[nodiscard]] IndoorState indoorState(const Standard standard,
                                              const MoistureLoad load,
                                              const double outdoorMean,
                                              const double outdoorHumidity)
        {
            switch(standard)
            {
                case Standard::EN_15026_DIN_4108:
                {
                    double humidity{ramp(outdoorMean, enHumidity)};
                    if(load == MoistureLoad::High)
                    {
                        humidity += enHighLoadOffset;
                    }
                    return {ramp(outdoorMean, enTemperature), clampHumidity(humidity)};
                }
                case Standard::ISO_13788:
                {
                    // The one rule that genuinely reads the outdoor humidity: indoor
                    // vapour is the outdoor vapour plus the class excess.
                    const double classExcess{load == MoistureLoad::High ? isoExcessHigh : isoExcessNormal};
                    const Ramp excessShape{isoExcessRange.lowOutdoor, isoExcessRange.highOutdoor, classExcess, 0.0};
                    const double excess{ramp(outdoorMean, excessShape)};
                    const double density{vapourDensity(outdoorMean, outdoorHumidity) + excess};
                    return {isoIndoorTemperature,
                            clampHumidity(relativeHumidityOf(isoIndoorTemperature, density))};
                }
                case Standard::ASHRAE_160:
                {
                    double humidity{ramp(outdoorMean, ashraeHumidity)};
                    if(load == MoistureLoad::High)
                    {
                        humidity += ashraeHighLoadOffset;
                    }
                    return {ramp(outdoorMean, ashraeTemperature), clampHumidity(humidity)};
                }
            }
            return {isoIndoorTemperature, defaultOutdoorHumidity};
        }

        //! One mean per day, held across that day's steps: the interior responds to the
        //! day, not to the hour.
        [[nodiscard]] std::vector<double> dailyMean(const std::vector<double> & values,
                                                    const std::size_t perDay)
        {
            if(perDay <= 1U)
            {
                return values;
            }
            std::vector<double> means;
            means.reserve(values.size());
            for(std::size_t start = 0; start < values.size(); start += perDay)
            {
                const std::size_t blockEnd{std::min(start + perDay, values.size())};
                double sum{0.0};
                for(std::size_t index = start; index < blockEnd; ++index)
                {
                    sum += values[index];
                }
                const double mean{sum / static_cast<double>(blockEnd - start)};
                means.insert(means.end(), blockEnd - start, mean);
            }
            return means;
        }

        //! "Sydney-Bankstown.AP exterior" -> "Sydney-Bankstown.AP interior (...)": a
        //! trailing " exterior" on the source name is replaced rather than repeated.
        [[nodiscard]] std::string interiorName(const std::string & exteriorName,
                                               const Standard standard,
                                               const MoistureLoad load)
        {
            static const std::string exteriorSuffix{" exterior"};
            std::string base{exteriorName};
            if(base.size() > exteriorSuffix.size()
               && base.ends_with(exteriorSuffix))
            {
                base.erase(base.size() - exteriorSuffix.size());
            }
            return base + " interior (" + standardToString(standard) + ", " + moistureLoadToString(load) + ")";
        }
    }   // namespace

    std::string standardToString(const Standard standard)
    {
        switch(standard)
        {
            case Standard::EN_15026_DIN_4108:
                return "EN 15026 / DIN 4108";
            case Standard::ISO_13788:
                return "ISO 13788";
            case Standard::ASHRAE_160:
                return "ASHRAE 160";
        }
        return {};
    }

    std::string moistureLoadToString(const MoistureLoad load)
    {
        return load == MoistureLoad::High ? "high" : "normal";
    }

    lbnl::ExpectedExt<TimeSeriesData, std::string> generateInterior(const TimeSeriesData & exterior,
                                                                    const Standard standard,
                                                                    const MoistureLoad load,
                                                                    const std::size_t stepsPerDay)
    {
        const auto temperatures{valuesForRole(exterior, ChannelRole::AirTemperature)};
        if(!temperatures.has_value() || temperatures->empty())
        {
            return lbnl::Unexpected{std::string{"The exterior dataset carries no air temperature."}};
        }
        auto humidities{valuesForRole(exterior, ChannelRole::RelativeHumidity)
                          .value_or(std::vector<double>(temperatures->size(), defaultOutdoorHumidity))};
        humidities.resize(temperatures->size(), defaultOutdoorHumidity);

        const auto temperatureMeans{dailyMean(temperatures.value(), stepsPerDay)};
        const auto humidityMeans{dailyMean(humidities, stepsPerDay)};

        std::vector<double> indoorTemperature;
        std::vector<double> indoorHumidity;
        indoorTemperature.reserve(temperatureMeans.size());
        indoorHumidity.reserve(temperatureMeans.size());
        for(std::size_t index = 0; index < temperatureMeans.size(); ++index)
        {
            const auto state{indoorState(standard, load, temperatureMeans[index], humidityMeans[index])};
            indoorTemperature.push_back(state.temperature);
            indoorHumidity.push_back(state.humidity);
        }

        TimeSeriesData result;
        result.Name = interiorName(exterior.Name, standard, load);
        result.channels = {Channel{ChannelRole::AirTemperature, std::move(indoorTemperature)},
                           Channel{ChannelRole::RelativeHumidity, std::move(indoorHumidity)}};
        result.UUID = contentUuid(result);
        return result;
    }
}   // namespace TimeSeriesLibrary::Standards
