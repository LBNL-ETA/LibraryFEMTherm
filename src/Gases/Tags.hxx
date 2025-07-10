#pragma once

#include <string>

namespace GasesLibrary
{
    //! @brief Class for storing XML tags as defined by the Gases.xsd schema.
    class Tags
    {
    public:
        Tags() = default;
        ~Tags() = default;

        [[nodiscard]] std::string gases() const;
        [[nodiscard]] std::string gas() const;

        [[nodiscard]] std::string properties() const;
        [[nodiscard]] std::string component() const;
        [[nodiscard]] std::string components() const;

        [[nodiscard]] std::string UUID() const;
        [[nodiscard]] std::string name() const;
        [[nodiscard]] std::string projectName() const;
        [[nodiscard]] std::string protectedTag() const;

        [[nodiscard]] std::string pureGas() const;

        [[nodiscard]] std::string molecularWeight() const;
        [[nodiscard]] std::string specificHeatRatio() const;
        [[nodiscard]] std::string conductivity() const;
        [[nodiscard]] std::string viscosity() const;
        [[nodiscard]] std::string specificHeat() const;

        [[nodiscard]] std::string A() const;
        [[nodiscard]] std::string B() const;
        [[nodiscard]] std::string C() const;

        [[nodiscard]] std::string fraction() const;

    private:
        const std::string m_Gases{"Gases"};
        const std::string m_Gas{"Gas"};
        const std::string m_PureGas{"PureGas"};

        const std::string m_Properties{"Properties"};
        const std::string m_Component{"Component"};
        const std::string m_Components{"Components"};

        const std::string m_UUID{"UUID"};
        const std::string m_Name{"Name"};
        const std::string m_ProjectName{"ProjectName"};
        const std::string m_Protected{"Protected"};

        const std::string m_MolecularWeight{"MolecularWeight"};
        const std::string m_SpecificHeatRatio{"SpecificHeatRatio"};
        const std::string m_Conductivity{"Conductivity"};
        const std::string m_Viscosity{"Viscosity"};
        const std::string m_SpecificHeat{"SpecificHeat"};

        const std::string m_A{"A"};
        const std::string m_B{"B"};
        const std::string m_C{"C"};

        const std::string m_Fraction{"Fraction"};
    };
}   // namespace GasesLibrary
