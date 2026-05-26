#include "FrameCavity.hxx"

#include <iterator>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include <fileParse/FileDataHandler.hxx>
#include <fileParse/Optional.hxx>
#include <fileParse/Vector.hxx>

#include "THMZ/Geometry/Geometry.hxx"
#include "THMZ/Geometry/Enumerators.hxx"
#include "THMZ/Properties/Enumerators.hxx"
#include "THMZ/Properties/Properties.hxx"

#include <ranges>

namespace ThermFile::Migration::FrameCavity
{
    namespace
    {
        // --- Internal types for parsing the legacy bits of Materials.xml -------------------

        struct LegacyCavityBlock
        {
            std::string cavityStandard;
            std::string gas;
        };

        struct LegacyRadiationEnclosureBlock
        {
            // The <EmissivityDefault> field is dropped on migration -- FEM aggregation of the
            // surrounding BCs supersedes it. We only need to know the element was present.
        };

        struct LegacyMaterialEntry
        {
            std::string uuid;
            std::optional<LegacyCavityBlock> cavity;
            std::optional<LegacyRadiationEnclosureBlock> radiationEnclosure;
        };

        template<typename NodeAdapter>
        const NodeAdapter & operator>>(const NodeAdapter & node, LegacyCavityBlock & cavity)
        {
            node >> FileParse::Child{"CavityStandard", cavity.cavityStandard};
            node >> FileParse::Child{"Gas", cavity.gas};
            return node;
        }

        template<typename NodeAdapter>
        const NodeAdapter & operator>>(const NodeAdapter & node, LegacyRadiationEnclosureBlock & /*block*/)
        {
            return node;
        }

        template<typename NodeAdapter>
        const NodeAdapter & operator>>(const NodeAdapter & node, LegacyMaterialEntry & entry)
        {
            node >> FileParse::Child{"UUID", entry.uuid};
            node >> FileParse::Child{"Cavity", entry.cavity};
            node >> FileParse::Child{"RadiationEnclosure", entry.radiationEnclosure};
            return node;
        }

        // --- Normalization helpers ---------------------------------------------------------

        bool isVentilatedStandard(std::string_view legacyStandard)
        {
            return legacyStandard == "CENVentilated" || legacyStandard == "ISO15099Ventilated";
        }

        ThermFile::ConvectionModel normalizeConvectionModel(std::string_view legacyStandard)
        {
            if(legacyStandard == "CEN" || legacyStandard == "CENVentilated")
            {
                return ThermFile::ConvectionModel::EN10077;
            }
            // Everything else (ISO15099, ISO15099Ventilated, NFRC, NFRCWithUserDimensions,
            // empty/unknown) maps to ISO15099.
            return ThermFile::ConvectionModel::ISO15099;
        }

        // --- Polygon tally and majority vote -----------------------------------------------

        struct ConvectionModelTally
        {
            std::map<ThermFile::ConvectionModel, int> counts;

            bool empty() const
            {
                return counts.empty();
            }
        };

        ThermFile::ConvectionModel pickWinner(const std::map<ThermFile::ConvectionModel, int> & counts,
                                              ThermFile::ConvectionModel fallback)
        {
            if(counts.empty())
            {
                return fallback;
            }

            auto best = counts.begin();
            for(auto it = std::next(counts.begin()); it != counts.end(); ++it)
            {
                // Strictly-greater so ties leave the existing best (which is the smallest
                // key encountered) -- for the enum this means ISO15099 wins ties.
                if(it->second > best->second)
                {
                    best = it;
                }
            }
            return best->first;
        }

        ThermFile::FrameCavityProperties promoteProjectConvectionModel(
          const ConvectionModelTally & tally, ThermFile::FrameCavityProperties properties)
        {
            if(tally.empty())
            {
                return properties;
            }
            properties.convectionModel = pickWinner(tally.counts, ThermFile::ConvectionModel::ISO15099);
            return properties;
        }
    }   // namespace

    LegacyMaterialsCapture capture(const std::string & materialsXml)
    {
        LegacyMaterialsCapture out;

        auto node{Common::getTopNodeFromString(materialsXml, "Materials")};
        if(!node.has_value())
        {
            return out;
        }

        std::visit(
          [&out](auto & adapter) {
              std::vector<LegacyMaterialEntry> entries;
              adapter >> FileParse::Child{"Material", entries};
              for(const auto & entry : entries)
              {
                  if(entry.cavity.has_value())
                  {
                      out.cavities[entry.uuid] = LegacyCavityEntry{entry.cavity->cavityStandard, entry.cavity->gas};
                  }
                  else if(entry.radiationEnclosure.has_value())
                  {
                      out.radiationEnclosures.insert(entry.uuid);
                  }
              }
          },
          node.value());

        return out;
    }

    MaterialsLibrary::DB applyToMaterials(const LegacyMaterialsCapture & legacy, MaterialsLibrary::DB materials)
    {
        // Strip legacy material UUIDs from the DB. The flat Material parser
        // dropped their cavity/radiation-enclosure sub-elements silently, leaving
        // empty Solid records behind -- those records reference UUIDs the shim is
        // repurposing, so they must go.
        for(const auto & uuid : legacy.cavities | std::views::keys)
        {
            materials.deleteWithUUID(uuid);
        }
        for(const auto & uuid : legacy.radiationEnclosures)
        {
            materials.deleteWithUUID(uuid);
        }
        return materials;
    }

    ThermFile::ThermModel applyToModel(const LegacyMaterialsCapture & legacy, ThermFile::ThermModel model)
    {
        // Pass A: rewrite polygons and tally per-cavity convection-model usage.
        ConvectionModelTally tally;
        for(auto & polygon : model.polygons)
        {
            if(const auto match = legacy.cavities.find(polygon.materialUUID); match != legacy.cavities.end())
            {
                const auto & entry = match->second;
                const bool ventilated = isVentilatedStandard(entry.cavityStandard);
                polygon.polygonType = ThermFile::PolygonType::FrameCavity;
                polygon.cavity = ThermFile::CavityData{entry.gas, ventilated};

                ++tally.counts[normalizeConvectionModel(entry.cavityStandard)];

                polygon.materialUUID.clear();
            }
            else if(legacy.radiationEnclosures.contains(polygon.materialUUID))
            {
                polygon.polygonType = ThermFile::PolygonType::RadiationEnclosure;
                polygon.materialUUID.clear();
            }
        }

        // Pass B: promote the tally onto the project-level convection model.
        model.properties.calculationOptions.frameCavityProperties = promoteProjectConvectionModel(
          tally, std::move(model.properties.calculationOptions.frameCavityProperties));

        return model;
    }

    MigrationResult applyAll(const LegacyMaterialsCapture & legacy,
                             MaterialsLibrary::DB materials,
                             ThermFile::ThermModel model)
    {
        return MigrationResult{.materials = applyToMaterials(legacy, std::move(materials)),
                               .model = applyToModel(legacy, std::move(model))};
    }
}   // namespace ThermFile::Migration::FrameCavity
