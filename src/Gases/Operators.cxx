#include "Operators.hxx"

#include "Gases.hxx"
#include "LibraryUtilities/Common.hxx"

namespace GasesLibrary
{
    bool operator==(const Coefficients & lhs, const Coefficients & rhs)
    {
        return LibraryCommon::isEqual(lhs.A, rhs.A, 1e-6) && LibraryCommon::isEqual(lhs.B, rhs.B, 1e-9)
               && LibraryCommon::isEqual(lhs.C, rhs.C, 1e-6);
    }

    bool operator!=(const Coefficients & lhs, const Coefficients & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const GasProperties & lhs, const GasProperties & rhs)
    {
        constexpr auto tolerance{1e-6};
        return LibraryCommon::isEqual(lhs.MolecularWeight, rhs.MolecularWeight, tolerance)
               && LibraryCommon::isEqual(lhs.SpecificHeatRatio, rhs.SpecificHeatRatio, tolerance)
               && lhs.Conductivity == rhs.Conductivity && lhs.Viscosity == rhs.Viscosity
               && lhs.SpecificHeat == rhs.SpecificHeat;
    }

    bool operator!=(const GasProperties & lhs, const GasProperties & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const GasCommon & lhs, const GasCommon & rhs)
    {
        return lhs.UUID == rhs.UUID && lhs.Name == rhs.Name && lhs.Protected == rhs.Protected;
    }

    bool operator!=(const GasCommon & lhs, const GasCommon & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const PureGas & lhs, const PureGas & rhs)
    {
        return static_cast<const GasCommon &>(lhs) == static_cast<const GasCommon &>(rhs)
               && lhs.Properties == rhs.Properties;
    }

    bool operator!=(const PureGas & lhs, const PureGas & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const Component & lhs, const Component & rhs)
    {
        return lhs.PureGasName == rhs.PureGasName && LibraryCommon::isEqual(lhs.Fraction, rhs.Fraction, 1e-6);
    }

    bool operator!=(const Component & lhs, const Component & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const Gas & lhs, const Gas & rhs)
    {
        return static_cast<const GasCommon &>(lhs) == static_cast<const GasCommon &>(rhs)
               && lhs.Components == rhs.Components;
    }

    bool operator!=(const Gas & lhs, const Gas & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const GasesData & lhs, const GasesData & rhs)
    {
        return lhs.gas == rhs.gas && lhs.components == rhs.components;
    }

    bool operator!=(const GasesData & lhs, const GasesData & rhs)
    {
        return !(lhs == rhs);
    }

    bool arePhysicalPropertiesEqual(const PureGas & lhs, const PureGas & rhs)
    {
        return lhs.Properties == rhs.Properties;
    }

    bool arePhysicalPropertiesEqual(const std::optional<PureGas> & lhs, const std::optional<PureGas> & rhs)
    {
        bool result{true};
        if(lhs.has_value() && rhs.has_value())
        {
            result = arePhysicalPropertiesEqual(lhs.value(), rhs.value());
        }
        else if(lhs.has_value() || rhs.has_value())
        {
            result = false;
        }
        return result;
    }

    bool arePhysicalPropertiesEqual(const Gas & lhs, const Gas & rhs)
    {
        return lhs.Components == rhs.Components;
    }

    bool arePhysicalPropertiesEqual(const GasesData & lhs, const GasesData & rhs)
    {
        bool result{true};
        if(lhs.components.size() == rhs.components.size())
        {
            for(size_t i = 0; i < lhs.components.size(); ++i)
            {
                if(!arePhysicalPropertiesEqual(lhs.components[i], rhs.components[i]))
                {
                    result = false;
                    break;
                }
            }
        }
        else
        {
            result = false;
        }

        if(result)
        {
            result = arePhysicalPropertiesEqual(lhs.gas, rhs.gas);
        }

        return result;
    }
}   // namespace GasesLibrary
