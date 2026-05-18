#include "FrameCavityMigration.hxx"

#include <iterator>
#include <map>
#include <string>
#include <vector>

#include "Materials/Materials.hxx"
#include "Materials/Definitions.hxx"
#include "THMZ/Geometry/Geometry.hxx"
#include "THMZ/Geometry/Enumerators.hxx"
#include "THMZ/Properties/Enumerators.hxx"
#include "THMZ/Properties/Properties.hxx"

namespace ThermFile
{
    namespace
    {
        enum class LegacyKind
        {
            Cavity,
            RadiationEnclosure
        };

        struct LegacyEntry
        {
            LegacyKind kind{LegacyKind::Cavity};
            MaterialsLibrary::CavityStandard cavityStandard{MaterialsLibrary::CavityStandard::ISO15099};
            std::string gas;
            bool ventilated{false};
        };

        using LegacyMap = std::map<std::string, LegacyEntry>;

        bool isVentilatedVariant(MaterialsLibrary::CavityStandard standard)
        {
            return standard == MaterialsLibrary::CavityStandard::CENVentilated
                   || standard == MaterialsLibrary::CavityStandard::ISO15099Ventilated;
        }

        ThermFile::CavityStandard normalizeStandard(MaterialsLibrary::CavityStandard legacy)
        {
            switch(legacy)
            {
                case MaterialsLibrary::CavityStandard::CEN:
                case MaterialsLibrary::CavityStandard::CENVentilated:
                    return ThermFile::CavityStandard::CEN;
                case MaterialsLibrary::CavityStandard::NFRC:
                case MaterialsLibrary::CavityStandard::NFRCWithUserDimensions:
                case MaterialsLibrary::CavityStandard::ISO15099:
                case MaterialsLibrary::CavityStandard::ISO15099Ventilated:
                default:
                    return ThermFile::CavityStandard::ISO15099;
            }
        }

        LegacyMap captureLegacyMaterials(MaterialsLibrary::DB & materials)
        {
            LegacyMap capture;

            for(const auto & material : materials.getMaterials())
            {
                if(MaterialsLibrary::isCavity(material))
                {
                    const auto * cavity = MaterialsLibrary::getCavity(material);
                    LegacyEntry entry;
                    entry.kind = LegacyKind::Cavity;
                    entry.cavityStandard = cavity->cavityStandard;
                    entry.gas = cavity->GasName.empty() ? std::string{"Air"} : cavity->GasName;
                    entry.ventilated = isVentilatedVariant(cavity->cavityStandard);
                    capture.emplace(material.UUID, std::move(entry));
                }
                else if(MaterialsLibrary::isRadiationEnclosure(material))
                {
                    LegacyEntry entry;
                    entry.kind = LegacyKind::RadiationEnclosure;
                    capture.emplace(material.UUID, std::move(entry));
                }
            }

            for(const auto & [uuid, _] : capture)
            {
                materials.deleteWithUUID(uuid);
            }

            return capture;
        }

        struct PolygonTally
        {
            std::map<ThermFile::CavityStandard, int> standardCounts;
            std::map<std::string, int> gasCounts;
            std::map<bool, int> ventilatedCounts;

            bool empty() const
            {
                return standardCounts.empty() && gasCounts.empty() && ventilatedCounts.empty();
            }
        };

        PolygonTally rewritePolygons(std::vector<ThermFile::Polygon> & polygons, const LegacyMap & capture)
        {
            PolygonTally tally;

            for(auto & polygon : polygons)
            {
                const auto match = capture.find(polygon.materialUUID);
                if(match != capture.end())
                {
                    const auto & entry = match->second;
                    if(entry.kind == LegacyKind::Cavity)
                    {
                        polygon.polygonType = ThermFile::PolygonType::FrameCavity;
                        polygon.cavity = ThermFile::CavityData{entry.gas, entry.ventilated};

                        const auto normalized = normalizeStandard(entry.cavityStandard);
                        ++tally.standardCounts[normalized];
                        ++tally.gasCounts[entry.gas];
                        ++tally.ventilatedCounts[entry.ventilated];
                    }
                    else
                    {
                        polygon.polygonType = ThermFile::PolygonType::RadiationEnclosure;
                    }
                    polygon.materialUUID.clear();
                }
            }

            return tally;
        }

        template<typename T>
        T pickWinner(const std::map<T, int> & counts, T fallback)
        {
            if(counts.empty())
            {
                return fallback;
            }

            auto best = counts.begin();
            for(auto it = std::next(counts.begin()); it != counts.end(); ++it)
            {
                // Strictly-greater so ties leave the existing best (which is the
                // smallest key encountered) — for the enums and bool this means
                // ISO15099 / false win ties; for the gas-name string map, "Air"
                // wins ties alphabetically.
                if(it->second > best->second)
                {
                    best = it;
                }
            }
            return best->first;
        }

        void promoteProjectDefaults(const PolygonTally & tally, ThermFile::FrameCavityProperties & properties)
        {
            if(tally.empty())
            {
                return;
            }

            properties.standard = pickWinner(tally.standardCounts, ThermFile::CavityStandard::ISO15099);
            properties.defaultGas = pickWinner(tally.gasCounts, std::string{"Air"});
            properties.defaultVentilated = pickWinner(tally.ventilatedCounts, false);
        }
    }   // namespace

    void applyFrameCavityMigration(MaterialsLibrary::DB & materials, ThermFile::ThermModel & model)
    {
        const auto capture = captureLegacyMaterials(materials);
        const auto tally = rewritePolygons(model.polygons, capture);
        promoteProjectDefaults(tally, model.properties.calculationOptions.frameCavityProperties);
    }
}   // namespace ThermFile
