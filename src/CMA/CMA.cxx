#include "CMA.hxx"

namespace CMALibrary
{

    bool operator<(const LayerOptionsKey & lhs, const LayerOptionsKey & rhs)
    {
        if(lhs.glazing < rhs.glazing)
            return true;
        if(lhs.glazing > rhs.glazing)
            return false;
        return lhs.spacer < rhs.spacer;
    }
}   // namespace CMAXML