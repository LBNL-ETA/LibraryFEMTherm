#include "Tags.hxx"

#include "LibraryUtilities/Common.hxx"

namespace MaterialsLibrary
{
    // Had to create structures that will hold maps because using it directly from THERM was causing static
    // initialization order fiasco
    namespace
    {
        //! /brief Holding tag definitions for the XML
        struct Tags
        {
            Tags() = default;
            ~Tags() = default;

            const std::map<MaterialRoughness, std::string> m_MaterialRoughness{
              {MaterialRoughness::VeryRough, "Very Rough"},
              {MaterialRoughness::Rough, "Rough"},
              {MaterialRoughness::MediumRough, "Medium Rough"},
              {MaterialRoughness::MediumSmooth, "Medium Smooth"},
              {MaterialRoughness::Smooth, "Smooth"},
              {MaterialRoughness::VerySmooth, "Very Smooth"}};
        };
    }   // namespace

    std::string materialsString()
    {
        return "Materials";
    }

    std::string materialString()
    {
        return "Material";
    }

    std::string solidString()
    {
        return "Solid";
    }

    std::string opticalString()
    {
        return "Optical";
    }

    std::string materialRoughnessToString(MaterialRoughness materialRoughness)
    {
        Tags tags;
        return LibraryCommon::enumToString(materialRoughness, tags.m_MaterialRoughness);
    }

    MaterialRoughness materialRoughnessFromString(std::string_view value)
    {
        Tags tags;
        return LibraryCommon::enumFromString(value, tags.m_MaterialRoughness);
    }

    std::string frontSideString()
    {
        return "Front";
    }

    std::string backSideString()
    {
        return "Back";
    }

    std::string directString()
    {
        return "Direct";
    }

    std::string diffuseString()
    {
        return "Diffuse";
    }

    std::string solarString()
    {
        return "Solar";
    }

    std::string visibleString()
    {
        return "Visible";
    }
}   // namespace MaterialsLibrary
