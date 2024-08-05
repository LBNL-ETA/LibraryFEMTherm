#pragma once

#include <fileParse/Common.hxx>
#include <fileParse/Vector.hxx>

#include "Serializers.hxx"

#include "Tags.hxx"
#include "Gases.hxx"

#include "LibraryUtilities/Util.hxx"

namespace GasesLibrary
{
    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, GasesLibrary::Coefficients & coefficients)
    {
        GasesLibrary::Tags tag;
        node >> FileParse::Child{tag.A(), coefficients.A};
        node >> FileParse::Child{tag.B(), coefficients.B};
        node >> FileParse::Child{tag.C(), coefficients.C};
        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const GasesLibrary::Coefficients & coefficients)
    {
        GasesLibrary::Tags tag;
        node << FileParse::Child{tag.A(), coefficients.A};
        node << FileParse::Child{tag.B(), coefficients.B};
        node << FileParse::Child{tag.C(), coefficients.C};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, GasesLibrary::GasProperties & properties)
    {
        GasesLibrary::Tags tag;
        node >> FileParse::Child{tag.molecularWeight(), properties.MolecularWeight};
        node >> FileParse::Child{tag.specificHeatRatio(), properties.SpecificHeatRatio};
        node >> FileParse::Child{tag.conductivity(), properties.Conductivity};
        node >> FileParse::Child{tag.viscosity(), properties.Viscosity};
        node >> FileParse::Child{tag.specificHeat(), properties.SpecificHeat};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const GasesLibrary::GasProperties & properties)
    {
        GasesLibrary::Tags tag;
        node << FileParse::Child{tag.molecularWeight(), properties.MolecularWeight};
        node << FileParse::Child{tag.specificHeatRatio(), properties.SpecificHeatRatio};
        node << FileParse::Child{tag.conductivity(), properties.Conductivity};
        node << FileParse::Child{tag.viscosity(), properties.Viscosity};
        node << FileParse::Child{tag.specificHeat(), properties.SpecificHeat};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, GasesLibrary::GasCommon & common)
    {
        GasesLibrary::Tags tag;
        node >> FileParse::Child{tag.UUID(), common.UUID};
        node >> FileParse::Child{tag.name(), common.Name};
        node >> FileParse::Child{tag.protectedTag(), common.Protected};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const GasesLibrary::GasCommon & common)
    {
        GasesLibrary::Tags tag;
        node << FileParse::Child{tag.UUID(), common.UUID};
        node << FileParse::Child{tag.name(), common.Name};
        node << FileParse::Child{tag.protectedTag(), common.Protected};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, GasesLibrary::PureGas & pureGas)
    {
        GasesLibrary::GasCommon & common = pureGas;
        node >> common;

        GasesLibrary::Tags tag;
        node >> FileParse::Child{tag.properties(), pureGas.Properties};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const GasesLibrary::PureGas & pureGas)
    {
        const GasesLibrary::GasCommon & common = pureGas;
        node << common;

        GasesLibrary::Tags tag;
        node << FileParse::Child{tag.properties(), pureGas.Properties};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, GasesLibrary::Component & component)
    {
        GasesLibrary::Tags tag;
        node >> FileParse::Child{tag.fraction(), component.Fraction};
        node >> FileParse::Child{tag.pureGas(), component.PureGasName};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const GasesLibrary::Component & component)
    {
        GasesLibrary::Tags tag;

        node << FileParse::Child{tag.fraction(), component.Fraction};
        node << FileParse::Child{tag.pureGas(), component.PureGasName};

        return node;
    }

    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, GasesLibrary::Gas & gas)
    {
        GasesLibrary::GasCommon & common = gas;
        node >> common;

        GasesLibrary::Tags tag;
        node >> FileParse::Child{{tag.components(), tag.component()}, gas.Components};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const GasesLibrary::Gas & gas)
    {
        const GasesLibrary::GasCommon & common = gas;
        node << common;

        GasesLibrary::Tags tag;
        node << FileParse::Child{{tag.components(), tag.component()}, gas.Components};

        return node;
    }
}   // namespace GasesLibrary