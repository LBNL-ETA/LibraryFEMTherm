#include <algorithm>
#include <cstdint>
#include <format>
#include <iterator>
#include <set>

#include "LibraryUtilities/StableUuid.hxx"

#include "FromValues.hxx"

namespace MaterialsLibrary
{
    namespace
    {
        //! Temperatures bracketing the two-point k(T) table, °C.
        constexpr double conductivityTemperatureLow{-50.0};
        constexpr double conductivityTemperatureHigh{150.0};

        //! The temperature THERM writes on a constant k(T) single-point table, °C.
        constexpr double constantTableTemperature{10.0};

        //! Emissivity THERM writes on every material record.
        constexpr double infraredEmissivity{0.9};

        std::vector<point> points(const Curve & curve)
        {
            std::vector<point> converted;
            converted.reserve(curve.size());
            for(const auto & [xValue, yValue] : curve)
            {
                converted.emplace_back(xValue, yValue);
            }
            return converted;
        }

        //! k(w) as THERM's table: one point when constant, two when linear in w.
        Curve conductivityMoistureTable(const double kDry, const double slope, const double wMax)
        {
            if(slope == 0.0)
            {
                return {{0.0, kDry}};
            }
            return {{0.0, kDry}, {wMax, kDry + slope * wMax}};
        }

        //! k(T) as THERM's table: one point when constant, two when linear in T.
        Curve conductivityTemperatureTable(const double kDry, const double beta)
        {
            if(beta == 0.0)
            {
                return {{constantTableTemperature, kDry}};
            }
            Curve table;
            for(const double temperature : {conductivityTemperatureLow, conductivityTemperatureHigh})
            {
                table.emplace_back(temperature, kDry * (1.0 + beta * temperature));
            }
            return table;
        }

        //! The liquid table THERM stores twice, from the one stated curve; a zero table
        //! when there is none.
        std::vector<point> liquidTable(const Curve & liquidTransportCurve)
        {
            return points(liquidTransportCurve.empty() ? Curve{{0.0, 0.0}} : liquidTransportCurve);
        }

        Optical optical()
        {
            Infrared infrared;
            infrared.Transmittance = 0.0;
            infrared.EmissivityFront = infraredEmissivity;
            infrared.EmissivityBack = infraredEmissivity;
            Integrated integrated;
            integrated.IR = infrared;
            Optical result;
            result.integrated = integrated;
            return result;
        }
    }   // namespace

    std::string materialUuid(const std::string_view name)
    {
        return LibraryCommon::stableUuid("material", name);
    }

    std::string materialColor(const std::string_view name)
    {
        LibraryCommon::Fnv1a hasher{LibraryCommon::fnvOffsetBasis};
        hasher.consume("color/");
        hasher.consume(name);
        const std::uint64_t digest{hasher.value()};
        const auto channel{[digest](const unsigned shift) {
            return 64U + static_cast<unsigned>((digest >> shift) & 0xFFU) % 160U;
        }};
        return std::format("0x{:02X}{:02X}{:02X}", channel(0U), channel(8U), channel(16U));
    }

    double maxWaterContent(const Curve & sorptionCurve)
    {
        double maximum{0.0};
        for(const auto & [humidity, water] : sorptionCurve)
        {
            maximum = std::max(maximum, water);
        }
        return maximum;
    }

    double waterContent(const Curve & sorptionCurve, const double humidity)
    {
        if(sorptionCurve.empty())
        {
            return 0.0;
        }
        if(humidity <= sorptionCurve.front().first)
        {
            return sorptionCurve.front().second;
        }
        if(humidity >= sorptionCurve.back().first)
        {
            return sorptionCurve.back().second;
        }
        const auto upper{std::ranges::upper_bound(
          sorptionCurve, humidity, {}, [](const auto & sample) { return sample.first; })};
        const auto lower{std::prev(upper)};
        const double fraction{(humidity - lower->first) / (upper->first - lower->first)};
        return lower->second + fraction * (upper->second - lower->second);
    }

    Curve resistanceFactorByWaterContent(const Curve & muCurve, const Curve & sorptionCurve)
    {
        Curve table;
        table.reserve(muCurve.size());
        for(const auto & [humidity, factor] : muCurve)
        {
            table.emplace_back(waterContent(sorptionCurve, humidity), factor);
        }
        return table;
    }

    Material fromValues(const std::string_view name,
                                        const double diffusionResistanceFactor,
                                        const Curve & sorptionCurve,
                                        const Curve & liquidTransportCurve,
                                        const Curve & muCurve,
                                        const double density,
                                        const double heatCapacity,
                                        const double thermalConductivity,
                                        const double thermalConductivityBeta,
                                        const double thermalConductivityMoistureSlope,
                                        const double porosity,
                                        const std::optional<std::string> & color)
    {
        HygroThermal hygro;
        hygro.MaterialInformation = "NA";
        hygro.BulkDensity = density;
        hygro.Porosity = porosity;
        hygro.SpecificHeatCapacityDry = heatCapacity;
        hygro.ThermalConductivityDry = thermalConductivity;
        hygro.WaterVaporDiffusionResistanceFactor = diffusionResistanceFactor;
        hygro.MoistureStorageFunction = points(sorptionCurve);
        hygro.LiquidTransportationCoefficientSuction = liquidTable(liquidTransportCurve);
        hygro.LiquidTransportationCoefficientRedistribution = liquidTable(liquidTransportCurve);
        hygro.ThermalConductivityMoistureDependent = points(conductivityMoistureTable(
          thermalConductivity, thermalConductivityMoistureSlope, maxWaterContent(sorptionCurve)));
        hygro.ThermalConductivityTemperatureDependent =
          points(conductivityTemperatureTable(thermalConductivity, thermalConductivityBeta));
        if(!muCurve.empty())
        {
            hygro.WaterVaporDiffusionResistanceFactorMoistureDependent =
              points(resistanceFactorByWaterContent(muCurve, sorptionCurve));
        }

        Solid solid;
        solid.hygroThermal = hygro;
        solid.optical = optical();

        Material record;
        record.UUID = materialUuid(name);
        record.Name = std::string{name};
        record.Protected = false;
        record.Color = color.value_or(materialColor(name));
        record.data = solid;
        return record;
    }

    DB materialsDatabase(const std::vector<Material> & records)
    {
        DB database;
        std::set<std::string> seen;
        for(const auto & record : records)
        {
            if(seen.insert(record.UUID).second)
            {
                database.add(record);
            }
        }
        return database;
    }
}   // namespace MaterialsLibrary
