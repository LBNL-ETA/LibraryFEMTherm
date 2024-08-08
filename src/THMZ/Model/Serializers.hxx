#pragma once

#include "THMX.hxx"

#include "THMZ/Preferences/Serializers.hxx"
#include "THMZ/Properties/Serializers.hxx"
#include "THMZ/Geometry/Serializers.hxx"
#include "THMZ/GlazingSystem/Serializers.hxx"
#include "THMZ/CMA/Serializers.hxx"

namespace ThermFile
{
    template<typename NodeAdapter>
    const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::ThermModel & model)
    {
        node >> FileParse::Child{"Version", model.version};
        node >> FileParse::Child{"CalculationReady", model.calculationReady};
        node >> FileParse::Child{"Preferences", model.preferences};
        node >> FileParse::Child{"Properties", model.properties};
        node >> FileParse::Child{"GlazingOrigin", model.glazingOrigin};
        node >> FileParse::Child{{"Cavities", "Cavity"}, model.cavities};
        node >> FileParse::Child{{"Polygons", "Polygon"}, model.polygons};
        node >> FileParse::Child{{"Boundaries", "Boundary"}, model.boundaryConditions};
        node >> FileParse::Child{{"GlazingSystems", "GlazingSystem"}, model.glazingSystems};
        node >> FileParse::Child{"CMAData", model.cmaData};

        return node;
    }

    template<typename NodeAdapter>
    NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::ThermModel & model)
    {
        node << FileParse::Child{"Version", model.version};
        node << FileParse::Child{"CalculationReady", model.calculationReady};
        node << FileParse::Child{"Preferences", model.preferences};
        node << FileParse::Child{"Properties", model.properties};
        node << FileParse::Child{"GlazingOrigin", model.glazingOrigin};
        node << FileParse::Child{{"Cavities", "Cavity"}, model.cavities};
        node << FileParse::Child{{"Polygons", "Polygon"}, model.polygons};
        node << FileParse::Child{{"Boundaries", "Boundary"}, model.boundaryConditions};
        node << FileParse::Child{{"GlazingSystems", "GlazingSystem"}, model.glazingSystems};
        node << FileParse::Child{"CMAData", model.cmaData};

        return node;
    }
}   // namespace ThermFile