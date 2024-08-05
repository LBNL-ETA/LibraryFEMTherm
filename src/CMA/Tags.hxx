#pragma once

#include <string>

namespace CMALibrary
{
    class Tags
    {
    public:
        Tags() = default;
        ~Tags() = default;

        [[nodiscard]] std::string rootTag() const;

        [[nodiscard]] std::string environmentTag() const;
        [[nodiscard]] std::string indoorTemperatureTag() const;
        [[nodiscard]] std::string outdoorTemperatureTag() const;
        [[nodiscard]] std::string glazingConductanceTag() const;
        [[nodiscard]] std::string spacerConductivityTag() const;

        [[nodiscard]] std::string glazingOptionTag() const;
        [[nodiscard]] std::string spacerOptionTag() const;

        [[nodiscard]] std::string hcInTag() const;
        [[nodiscard]] std::string hcOutTag() const;
        [[nodiscard]] std::string emissivityInTag() const;
        [[nodiscard]] std::string emissivityOutTag() const;

        [[nodiscard]] std::string layerOptionsTag() const;

        [[nodiscard]] std::string singleLayerTag() const;
        [[nodiscard]] std::string doubleLayerTag() const;

        [[nodiscard]] std::string thicknessTag() const;
        [[nodiscard]] std::string thickness1Tag() const;
        [[nodiscard]] std::string thickness2Tag() const;

        [[nodiscard]] std::string conductivityTag() const;
        [[nodiscard]] std::string conductivity1Tag() const;
        [[nodiscard]] std::string conductivity2Tag() const;

    private:
        const std::string m_RootTag{"CMAData"};

        const std::string m_EnvironmentTag{"Environment"};
        const std::string m_Tin{"Tin"};
        const std::string m_Tout{"Tout"};

        const std::string m_GlazingConductanceTag{"GlazingConductance"};
        const std::string m_SpacersConductivityTag{"SpacerConductivity"};

        const std::string m_GlazingOptionTag{"Glazing"};
        const std::string m_SpacerOptionTag{"Spacer"};

        const std::string m_HcInTag{"hcin"};
        const std::string m_HcOutTag{"hcout"};
        const std::string m_EmissivityInTag{"EmissivityIn"};
        const std::string m_EmissivityOutTag{"EmissivityOut"};

        const std::string m_LayerOptions{"Options"};

        const std::string m_SingleLayerTag{"SingleLayer"};
        const std::string m_DoubleLayerTag{"DoubleLayer"};

        const std::string m_ThicknessTag{"Thickness"};
        const std::string m_ConductivityTag{"Conductivity"};
        const std::string m_Thickness1Tag{"Thickness1"};
        const std::string m_Thickness2Tag{"Thickness2"};
        const std::string m_Conductivity1Tag{"Conductivity1"};
        const std::string m_Conductivity2Tag{"Conductivity2"};
    };
}   // namespace CMAXML