#include "Materials.hxx"

#include <utility>

#include "LibraryUtilities/FileManipulation.hxx"
#include "Tags.hxx"

namespace MaterialsLibrary
{
    Material generate(std::string uuid)
    {
        Material record;
        record.UUID = std::move(uuid);
        record.data = Solid();
        return record;
    }

    Material generate()
    {
        return generate(std::string{});
    }

    void ensureHygroThermal(Material & material)
    {
        if(!material.data.hygroThermal.has_value())
        {
            material.data.hygroThermal = HygroThermal();
        }
    }

    void ensureIntegrated(Material & material)
    {
        if(!material.data.optical.has_value())
        {
            material.data.optical = Optical();
        }
        if(!material.data.optical->integrated.has_value())
        {
            material.data.optical->integrated = Integrated();
        }
    }

    void ensureIntegratedSolarAndVisible(Material & material)
    {
        ensureIntegrated(material);
        if(!material.data.optical->integrated->Solar.has_value())
        {
            material.data.optical->integrated->Solar = OpticalType();
        }

        if(!material.data.optical->integrated->Visible.has_value())
        {
            material.data.optical->integrated->Visible = OpticalType();
        }
    }
}   // namespace MaterialsLibrary
