#include "Tags.hxx"

namespace GasesLibrary
{
    std::string Tags::gases() const
    {
        return m_Gases;
    }

    std::string Tags::component() const
    {
        return m_Component;
    }

    std::string Tags::components() const
    {
        return m_Components;
    }

    std::string Tags::UUID() const
    {
        return m_UUID;
    }

    std::string Tags::name() const
    {
        return m_Name;
    }

    std::string Tags::pureGas() const
    {
        return m_PureGas;
    }

    std::string Tags::molecularWeight() const
    {
        return m_MolecularWeight;
    }

    std::string Tags::specificHeatRatio() const
    {
        return m_SpecificHeatRatio;
    }

    std::string Tags::conductivity() const
    {
        return m_Conductivity;
    }

    std::string Tags::viscosity() const
    {
        return m_Viscosity;
    }

    std::string Tags::specificHeat() const
    {
        return m_SpecificHeat;
    }

    std::string Tags::A() const
    {
        return m_A;
    }

    std::string Tags::B() const
    {
        return m_B;
    }

    std::string Tags::C() const
    {
        return m_C;
    }

    std::string Tags::gas() const
    {
        return m_Gas;
    }

    std::string Tags::fraction() const
    {
        return m_Fraction;
    }

    std::string Tags::properties() const
    {
        return m_Properties;
    }

    std::string Tags::protectedTag() const
    {
        return m_Protected;
    }
}   // namespace GasesLibrary
