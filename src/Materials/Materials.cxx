#include "Materials.hxx"

#include <map>
#include <functional>
#include <utility>

#include "LibraryUtilities/FileManipulation.hxx"
#include "Tags.hxx"

namespace MaterialsLibrary
{
    Material generate(std::string uuid, MaterialType type)
    {
        std::map<MaterialType, std::function<void(Material &)>> materialActions = {
          {MaterialType::Solid, [](Material & record) { record.data = Solid(); }},
          {MaterialType::Cavity, [](Material & record) { record.data = Cavity(); }},
          {MaterialType::RadiationEnclosure, [](Material & record) { record.data = RadiationEnclosure(); }}};

        Material record;
        record.UUID = std::move(uuid);

        if(materialActions.find(type) != materialActions.end())
        {
            materialActions[type](record);
        }

        return record;
    }

    Material generate(MaterialType type)
    {
        std::string uuid;
        return generate(uuid, type);
    }

    void ensureHygroThermal(Material & material)
    {
        if(MaterialsLibrary::isSolid(material))
        {
            auto solid{MaterialsLibrary::getSolid(material)};
            if(!solid->hygroThermal.has_value())
            {
                solid->hygroThermal = MaterialsLibrary::HygroThermal();
            }
        }
    }

    void ensureIntegrated(Material & material)
    {
        if(MaterialsLibrary::isSolid(material))
        {
            auto solid{MaterialsLibrary::getSolid(material)};
            if(!solid->optical.has_value())
            {
                solid->optical = MaterialsLibrary::Optical();
                if(!solid->optical->integrated.has_value())
                {
                    solid->optical->integrated = MaterialsLibrary::Integrated();
                }
            }
        }
    }

    void ensureIntegratedSolarAndVisible(Material & material)
    {
        ensureIntegrated(material);
        if(MaterialsLibrary::isSolid(material))
        {
            auto solid{MaterialsLibrary::getSolid(material)};
            if(!solid->optical->integrated->Solar.has_value())
            {
                solid->optical->integrated->Solar = MaterialsLibrary::OpticalType();
            }

            if(!solid->optical->integrated->Visible.has_value())
            {
                solid->optical->integrated->Visible = MaterialsLibrary::OpticalType();
            }
        }
    }

    bool isSolid(const Material & material)
    {
        return std::holds_alternative<Solid>(material.data);
    }

    bool isCavity(const Material & material)
    {
        return std::holds_alternative<Cavity>(material.data);
    }

    bool isRadiationEnclosure(const Material & material)
    {
        return std::holds_alternative<RadiationEnclosure>(material.data);
    }

    std::string getMaterialType(const Material & material)
    {
        if(isSolid(material))
        {
            return "Solid";
        }

        if(isCavity(material))
        {
            return "Frame Cavity";
        }

        return "Radiation Enclosure";
    }

    Solid * getSolid(Material & material)
    {
        return std::get_if<Solid>(&material.data);
    }

    const Solid * getSolid(const Material & material)
    {
        return std::get_if<Solid>(&material.data);
    }

    Cavity * getCavity(Material & material)
    {
        return std::get_if<Cavity>(&material.data);
    }

    const Cavity * getCavity(const Material & material)
    {
        return std::get_if<Cavity>(&material.data);
    }

    RadiationEnclosure * getRadiationEnclosure(Material & material)
    {
        return std::get_if<RadiationEnclosure>(&material.data);
    }

    const RadiationEnclosure * getRadiationEnclosure(const Material & material)
    {
        return std::get_if<RadiationEnclosure>(&material.data);
    }
}   // namespace MaterialsLibrary
