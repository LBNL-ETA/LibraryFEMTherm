#include "Tags.hxx"

namespace CMALibrary
{
    std::string Tags::rootTag() const
    {
        return m_RootTag;
    }

    std::string Tags::environmentTag() const
    {
        return m_EnvironmentTag;
    }

    std::string Tags::indoorTemperatureTag() const
    {
        return m_Tin;
    }

    std::string Tags::outdoorTemperatureTag() const
    {
        return m_Tout;
    }

    std::string Tags::glazingConductanceTag() const
    {
        return m_GlazingConductanceTag;
    }

    std::string Tags::spacerConductivityTag() const
    {
        return m_SpacersConductivityTag;
    }

    std::string Tags::glazingOptionTag() const
    {
        return m_GlazingOptionTag;
    }

    std::string Tags::spacerOptionTag() const
    {
        return m_SpacerOptionTag;
    }

    std::string Tags::hcInTag() const
    {
        return m_HcInTag;
    }

    std::string Tags::hcOutTag() const
    {
        return m_HcOutTag;
    }

    std::string Tags::emissivityInTag() const
    {
        return m_EmissivityInTag;
    }

    std::string Tags::emissivityOutTag() const
    {
        return m_EmissivityOutTag;
    }

    std::string Tags::layerOptionsTag() const
    {
        return m_LayerOptions;
    }

    std::string Tags::singleLayerTag() const
    {
        return m_SingleLayerTag;
    }

    std::string Tags::doubleLayerTag() const
    {
        return m_DoubleLayerTag;
    }

    std::string Tags::thicknessTag() const
    {
        return m_ThicknessTag;
    }

    std::string Tags::thickness1Tag() const
    {
        return m_Thickness1Tag;
    }

    std::string Tags::thickness2Tag() const
    {
        return m_Thickness2Tag;
    }

    std::string Tags::conductivityTag() const
    {
        return m_ConductivityTag;
    }

    std::string Tags::conductivity1Tag() const
    {
        return m_Conductivity1Tag;
    }

    std::string Tags::conductivity2Tag() const
    {
        return m_Conductivity2Tag;
    }
}   // namespace CMAXML