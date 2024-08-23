#include "Gases.hxx"

#include "Tags.hxx"

namespace GasesLibrary
{

    Coefficients::Coefficients(double a, double b, double c) : A{a}, B{b}, C{c}
    {}

    PureGas::PureGas(const std::string & uuid,
                     const std::string & name,
                     bool isProtected,
                     const GasProperties & properties) :
        GasCommon(uuid, name, isProtected), Properties{properties}
    {}

    Component::Component(double fraction, const std::string & pureGasName) :
        Fraction(fraction), PureGasName(pureGasName)
    {}

    Gas::Gas(const std::string & uuid,
             const std::string & name,
             bool isProtected,
             const std::vector<Component> & components) :
        GasCommon(uuid, name, isProtected), Components(components)
    {}

    GasProperties::GasProperties(double molecularWeight,
                                 double specificHeatRatio,
                                 const Coefficients & conductivity,
                                 const Coefficients & viscosity,
                                 const Coefficients & specificHeat) :
        MolecularWeight(molecularWeight),
        SpecificHeatRatio(specificHeatRatio),
        Conductivity(conductivity),
        Viscosity(viscosity),
        SpecificHeat(specificHeat)
    {}

    GasCommon::GasCommon(const std::string & uuid,
                         const std::string & name,
                         bool isProtected) :
        UUID(uuid), Name(name), Protected(isProtected)
    {}

    bool isGasesDataComplete(const GasesData & data)
    {
        bool result{true};

        // check if each value in data.components has_value is true
        for(const auto & component : data.components)
        {
            if(!component.has_value())
            {
                result = false;
                break;
            }
        }

        return result;
    }

    GasesData::GasesData(const Gas & gas, const std::vector<std::optional<PureGas>> & components) :
        gas(gas), components(components), Name(gas.Name), ProjectName(gas.ProjectName)
    {}
}   // namespace GasesLibrary
