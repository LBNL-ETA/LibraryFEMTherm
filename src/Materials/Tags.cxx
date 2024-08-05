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

            const std::map<CavityStandard, std::string> m_CavityStandard{
              {CavityStandard::NFRC, "NFRC"},
              {CavityStandard::CEN, "CEN"},
              {CavityStandard::CENVentilated, "CENVentilated"},
              {CavityStandard::NFRCWithUserDimensions, "NFRCWithUserDimensions"},
              {CavityStandard::ISO15099, "ISO15099"},
              {CavityStandard::ISO15099Ventilated, "ISO15099Ventilated"}};
        };

        //! /brief Enumeration strings as they are displayed to the user
        class StringDisplay
        {
        public:
            StringDisplay() = default;
            ~StringDisplay() = default;

            const std::map<CavityStandard, std::string> m_CavityStandard{
              {CavityStandard::NFRC, "NFRC"},
              {CavityStandard::CEN, "CEN"},
              {CavityStandard::CENVentilated, "CEN (slightly ventilated)"},
              {CavityStandard::NFRCWithUserDimensions, "NFRC with user dimensions"},
              {CavityStandard::ISO15099, "ISO15099"},
              {CavityStandard::ISO15099Ventilated, "ISO15099 ventilated"}};
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

    std::string cavityString()
    {
        return "Cavity";
    }

    std::string enclosureString()
    {
        Tags tags;
        return "RadiationEnclosure";
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

    std::string cavityStandardToString(CavityStandard cavStandard)
    {
        Tags tags;
        return LibraryCommon::enumToString(cavStandard, tags.m_CavityStandard);
    }

    CavityStandard cavityStandardFromString(std::string_view cavityStandardTag)
    {
        Tags tags;
        return LibraryCommon::enumFromString(cavityStandardTag, tags.m_CavityStandard);
    }

    std::string cavityStandardToDisplayString(CavityStandard cavityStandard)
    {
        StringDisplay tags;
        return LibraryCommon::enumToString(cavityStandard, tags.m_CavityStandard);
    }

    CavityStandard cavityStandardFromDisplayString(std::string_view cavity)
    {
        StringDisplay tags;
        return LibraryCommon::enumFromString(cavity, tags.m_CavityStandard);
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
