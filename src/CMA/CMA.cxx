#include "CMA.hxx"

namespace CMALibrary
{

    Environment::Environment(double tin, double tout) : Tin{tin}, Tout{tout}
    {}

    DoubleLayer::DoubleLayer(double thickness1, double thickness2, LayerOptions layerOptions) :
        thickness1{thickness1}, thickness2{thickness2}, layerOptions{layerOptions}
    {}

    bool operator<(const LayerOptionsKey & lhs, const LayerOptionsKey & rhs)
    {
        if(lhs.glazing < rhs.glazing)
            return true;
        if(lhs.glazing > rhs.glazing)
            return false;
        return lhs.spacer < rhs.spacer;
    }

    LayerValues::LayerValues(double hcin, double hcout, double emissivityIn, double emissivityOut) :
        hcin{hcin}, hcout{hcout}, emissivityIn{emissivityIn}, emissivityOut{emissivityOut}
    {}

    SingleLayer::SingleLayer(double thickness, LayerOptions layerOptions) :
        thickness{thickness}, layerOptions{layerOptions}
    {}
}   // namespace CMAXML