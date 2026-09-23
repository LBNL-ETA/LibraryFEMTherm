#include <algorithm>
#include <cstdint>
#include <format>
#include <iterator>
#include <set>

#include "LibraryUtilities/StableUuid.hxx"

#include "Materials.hxx"

namespace ThermFile::Build
{
    using Model::Curve;
    using Model::Material;

    namespace
    {
        //! Temperatures bracketing the two-point k(T) table, °C.
        constexpr double conductivityTemperatureLow{-50.0};
        constexpr double conductivityTemperatureHigh{150.0};

        //! The temperature THERM writes on a constant k(T) single-point table, °C.
        constexpr double constantTableTemperature{10.0};

        //! Emissivity THERM writes on every material record.
        constexpr double infraredEmissivity{0.9};

        std::vector<MaterialsLibrary::point> points(const Curve & curve)
        {
            std::vector<MaterialsLibrary::point> converted;
            converted.reserve(curve.size());
            for(const auto & [xValue, yValue] : curve)
            {
                converted.emplace_back(xValue, yValue);
            }
            return converted;
        }

        //! k(w) as THERM's table: one point when constant, two when linear in w.
        Curve conductivityMoistureTable(const Material & material)
        {
            const double kDry{material.thermalConductivity};
            if(material.thermalConductivityMoistureSlope == 0.0)
            {
                return {{0.0, kDry}};
            }
            const double wMax{maxWaterContent(material)};
            return {{0.0, kDry}, {wMax, kDry + material.thermalConductivityMoistureSlope * wMax}};
        }

        //! k(T) as THERM's table: one point when constant, two when linear in T.
        Curve conductivityTemperatureTable(const Material & material)
        {
            const double kDry{material.thermalConductivity};
            if(material.thermalConductivityBeta == 0.0)
            {
                return {{constantTableTemperature, kDry}};
            }
            Curve table;
            for(const double temperature : {conductivityTemperatureLow, conductivityTemperatureHigh})
            {
                table.emplace_back(temperature, kDry * (1.0 + material.thermalConductivityBeta * temperature));
            }
            return table;
        }

        MaterialsLibrary::HygroThermal hygroThermal(const Material & material)
        {
            MaterialsLibrary::HygroThermal record;
            record.MaterialInformation = "NA";
            record.BulkDensity = material.density;
            record.Porosity = material.porosity;
            record.SpecificHeatCapacityDry = material.heatCapacity;
            record.ThermalConductivityDry = material.thermalConductivity;
            record.WaterVaporDiffusionResistanceFactor = material.diffusionResistanceFactor;
            record.MoistureStorageFunction = points(material.sorptionCurve);
            const auto liquid{points(material.liquidTransportCurve.empty() ? Curve{{0.0, 0.0}}
                                                                            : material.liquidTransportCurve)};
            record.LiquidTransportationCoefficientSuction = liquid;
            record.LiquidTransportationCoefficientRedistribution = liquid;
            record.ThermalConductivityMoistureDependent = points(conductivityMoistureTable(material));
            record.ThermalConductivityTemperatureDependent = points(conductivityTemperatureTable(material));
            const auto muTable{resistanceFactorByWaterContent(material)};
            if(!muTable.empty())
            {
                record.WaterVaporDiffusionResistanceFactorMoistureDependent = points(muTable);
            }
            return record;
        }

        MaterialsLibrary::Optical optical()
        {
            MaterialsLibrary::Infrared infrared;
            infrared.Transmittance = 0.0;
            infrared.EmissivityFront = infraredEmissivity;
            infrared.EmissivityBack = infraredEmissivity;
            MaterialsLibrary::Integrated integrated;
            integrated.IR = infrared;
            MaterialsLibrary::Optical result;
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

    double maxWaterContent(const Material & material)
    {
        double maximum{0.0};
        for(const auto & [humidity, water] : material.sorptionCurve)
        {
            maximum = std::max(maximum, water);
        }
        return maximum;
    }

    double waterContent(const Material & material, const double humidity)
    {
        const auto & curve{material.sorptionCurve};
        if(curve.empty())
        {
            return 0.0;
        }
        if(humidity <= curve.front().first)
        {
            return curve.front().second;
        }
        if(humidity >= curve.back().first)
        {
            return curve.back().second;
        }
        const auto upper{std::ranges::upper_bound(
          curve, humidity, {}, [](const auto & sample) { return sample.first; })};
        const auto lower{std::prev(upper)};
        const double fraction{(humidity - lower->first) / (upper->first - lower->first)};
        return lower->second + fraction * (upper->second - lower->second);
    }

    Curve resistanceFactorByWaterContent(const Material & material)
    {
        Curve table;
        table.reserve(material.muCurve.size());
        for(const auto & [humidity, factor] : material.muCurve)
        {
            table.emplace_back(waterContent(material, humidity), factor);
        }
        return table;
    }

    MaterialsLibrary::Material libraryMaterial(const Material & material)
    {
        MaterialsLibrary::Solid solid;
        solid.hygroThermal = hygroThermal(material);
        solid.optical = optical();

        MaterialsLibrary::Material record;
        record.UUID = materialUuid(material.name);
        record.Name = material.name;
        record.Protected = false;
        record.Color = materialColor(material.name);
        record.data = solid;
        return record;
    }

    MaterialsLibrary::DB materialsDatabase(const std::vector<Material> & materials)
    {
        MaterialsLibrary::DB database;
        std::set<std::string> seen;
        for(const auto & material : materials)
        {
            if(seen.insert(material.name).second)
            {
                database.add(libraryMaterial(material));
            }
        }
        return database;
    }
}   // namespace ThermFile::Build
