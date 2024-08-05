#include "gases1.hxx"

#include <Gases/Gases.hxx>
#include <Gases/Tags.hxx>

namespace TestGases
{
    std::string testDatabase()
    {
        GasesLibrary::Tags tags;
        std::string fileContent{"<" + tags.gases() + ">\n"};
        fileContent += "<Version>1</Version>\n";
        fileContent += air_pure();
        fileContent += argon_pure();
        fileContent += mixtureAirArgon();
        fileContent += mixtureAirSF6();
        fileContent += "</" + tags.gases() + ">";
        return fileContent;
    }

    std::string air_pure()
    {
        return {"\t<PureGas>\n"
                "\t\t<UUID>8d33196f-f052-46e6-8353-bccb9a779f9c</UUID>\n"
                "\t\t<Name>Air</Name>\n"
                "\t\t<Protected>true</Protected>\n"
                "\t\t<Properties>\n"
                "\t\t\t<MolecularWeight>28.97</MolecularWeight>\n"
                "\t\t\t<SpecificHeatRatio>1.4</SpecificHeatRatio>\n"
                "\t\t\t<Conductivity>\n"
                "\t\t\t\t<A>0.002873</A>\n"
                "\t\t\t\t<B>0.0000776</B>\n"
                "\t\t\t\t<C>0</C>\n"
                "\t\t\t</Conductivity>\n"
                "\t\t\t<Viscosity>\n"
                "\t\t\t\t<A>0.000003723</A>\n"
                "\t\t\t\t<B>4.94E-08</B>\n"
                "\t\t\t\t<C>0</C>\n"
                "\t\t\t</Viscosity>\n"
                "\t\t\t<SpecificHeat>\n"
                "\t\t\t\t<A>1002.737</A>\n"
                "\t\t\t\t<B>0.012324</B>\n"
                "\t\t\t\t<C>0</C>\n"
                "\t\t\t</SpecificHeat>\n"
                "\t\t</Properties>\n"
                "\t</PureGas>\n"};
    }

    std::string argon_pure()
    {
        return {"\t<PureGas>\n"
                "\t\t<UUID>444d94e4-326e-4c1c-aef1-666771b569cd</UUID>\n"
                "\t\t<Name>Argon</Name>\n"
                "\t\t<Protected>true</Protected>\n"
                "\t\t<Properties>\n"
                "\t\t\t<MolecularWeight>39.948</MolecularWeight>\n"
                "\t\t\t<SpecificHeatRatio>1.67</SpecificHeatRatio>\n"
                "\t\t\t<Conductivity>\n"
                "\t\t\t\t<A>0.002285</A>\n"
                "\t\t\t\t<B>0.00005149</B>\n"
                "\t\t\t\t<C>0</C>\n"
                "\t\t\t</Conductivity>\n"
                "\t\t\t<Viscosity>\n"
                "\t\t\t\t<A>0.000003379</A>\n"
                "\t\t\t\t<B>6.451E-08</B>\n"
                "\t\t\t\t<C>0</C>\n"
                "\t\t\t</Viscosity>\n"
                "\t\t\t<SpecificHeat>\n"
                "\t\t\t\t<A>521.9285</A>\n"
                "\t\t\t\t<B>0</B>\n"
                "\t\t\t\t<C>0</C>\n"
                "\t\t\t</SpecificHeat>\n"
                "\t\t</Properties>\n"
                "\t</PureGas>\n"};
    }

    std::string mixtureAirArgon()
    {
        return {"\t<Gas>\n"
                "\t\t<UUID>37881919-7cdf-4428-bf5c-10702bc78415</UUID>\n"
                "\t\t<Name>Air 10%/Argon 90%</Name>\n"
                "\t\t<Protected>true</Protected>\n"
                "\t\t<Components>\n"
                "\t\t\t<Component>\n"
                "\t\t\t\t<Fraction>\n"
                "\t\t\t\t\t0.1\n"
                "\t\t\t\t</Fraction>\n"
                "\t\t\t\t<PureGas>\n"
                "\t\t\t\t\tAir\n"
                "\t\t\t\t</PureGas>\n"
                "\t\t\t</Component>\n"
                "\t\t\t<Component>\n"
                "\t\t\t\t<Fraction>\n"
                "\t\t\t\t\t0.9\n"
                "\t\t\t\t</Fraction>\n"
                "\t\t\t\t<PureGas>\n"
                "\t\t\t\t\tArgon\n"
                "\t\t\t\t</PureGas>\n"
                "\t\t\t</Component>\n"
                "\t\t</Components>\n"
                "\t</Gas>\n"};
    }

    std::string mixtureAirSF6()
    {
        return {"\t<Gas>\n"
                "\t\t<UUID>a948ebb6-2cb6-4792-9109-ac0d3dd72712</UUID>\n"
                "\t\t<Name>Air 10%/SF6 90%</Name>\n"
                "\t\t<Protected>true</Protected>\n"
                "\t\t<Components>\n"
                "\t\t\t<Component>\n"
                "\t\t\t\t<Fraction>\n"
                "\t\t\t\t\t0.1\n"
                "\t\t\t\t</Fraction>\n"
                "\t\t\t\t<PureGas>\n"
                "\t\t\t\t\tAir\n"
                "\t\t\t\t</PureGas>\n"
                "\t\t\t</Component>\n"
                "\t\t\t<Component>\n"
                "\t\t\t\t<Fraction>\n"
                "\t\t\t\t\t0.9\n"
                "\t\t\t\t</Fraction>\n"
                "\t\t\t\t<PureGas>\n"
                "\t\t\t\t\tSF6\n"
                "\t\t\t\t</PureGas>\n"
                "\t\t\t</Component>\n"
                "\t\t</Components>\n"
                "\t</Gas>\n"};
    }
}   // namespace TestGases