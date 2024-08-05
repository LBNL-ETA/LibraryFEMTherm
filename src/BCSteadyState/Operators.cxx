#include "Operators.hxx"

#include "SteadyState.hxx"
#include "LibraryUtilities/Common.hxx"

namespace BCSteadyStateLibrary
{
    bool operator==(const Convection & lhs, const Convection & rhs)
    {
        constexpr auto tolerance{1e-6};
        return LibraryCommon::isEqual(lhs.temperature, rhs.temperature, tolerance)
               && LibraryCommon::isEqual(lhs.filmCoefficient, rhs.filmCoefficient, tolerance);
    }

    bool operator!=(const Convection & lhs, const Convection & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const ConstantFlux & lhs, const ConstantFlux & rhs)
    {
        constexpr auto tolerance{1e-6};
        return LibraryCommon::isEqual(lhs.flux, rhs.flux, tolerance);
    }

    bool operator!=(const ConstantFlux & lhs, const ConstantFlux & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const AutomaticEnclosure & lhs, const AutomaticEnclosure & rhs)
    {
        constexpr auto tolerance{1e-6};
        return LibraryCommon::isEqual(lhs.temperature, rhs.temperature, tolerance)
               && LibraryCommon::isEqual(lhs.emissivity, rhs.emissivity, tolerance);
    }

    bool operator!=(const AutomaticEnclosure & lhs, const AutomaticEnclosure & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const ManualEnclosure &, const ManualEnclosure &)
    {
        return true;
    }

    bool operator!=(const ManualEnclosure &, const ManualEnclosure &)
    {
        return false;
    }

    bool operator==(const BlackBodyRadiation & lhs, const BlackBodyRadiation & rhs)
    {
        constexpr auto tolerance{1e-6};
        return LibraryCommon::isEqual(lhs.temperature, rhs.temperature, tolerance)
               && LibraryCommon::isEqual(lhs.emissivity, rhs.emissivity, tolerance);
    }

    bool operator!=(const BlackBodyRadiation & lhs, const BlackBodyRadiation & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const LinearizedRadiation & lhs, const LinearizedRadiation & rhs)
    {
        constexpr auto tolerance{1e-6};
        return LibraryCommon::isEqual(lhs.temperature, rhs.temperature, tolerance)
               && LibraryCommon::isEqual(lhs.filmCoefficient, rhs.filmCoefficient, tolerance);
    }

    bool operator!=(const LinearizedRadiation & lhs, const LinearizedRadiation & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const Radiation & lhs, const Radiation & rhs)
    {
        return lhs.radiation == rhs.radiation;
    }

    bool operator!=(const Radiation & lhs, const Radiation & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const Comprehensive & lhs, const Comprehensive & rhs)
    {
        constexpr auto tolerance{1e-6};
        return LibraryCommon::isEqual(lhs.relativeHumidity, rhs.relativeHumidity, tolerance)
               && lhs.convection == rhs.convection && lhs.constantFlux == rhs.constantFlux
               && lhs.radiation == rhs.radiation;
    }

    bool operator!=(const Comprehensive & lhs, const Comprehensive & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const Simplified & lhs, const Simplified & rhs)
    {
        constexpr auto tolerance{1e-6};
        return LibraryCommon::isEqual(lhs.relativeHumidity, rhs.relativeHumidity, tolerance)
               && LibraryCommon::isEqual(lhs.temperature, rhs.temperature, tolerance)
               && LibraryCommon::isEqual(lhs.filmCoefficient, rhs.filmCoefficient, tolerance);
    }

    bool operator!=(const Simplified & lhs, const Simplified & rhs)
    {
        return !(lhs == rhs);
    }


    bool operator==(const RadiationSurface & lhs, const RadiationSurface & rhs)
    {
        constexpr auto tolerance{1e-6};
        return lhs.isDefault == rhs.isDefault && LibraryCommon::isEqual(lhs.temperature, rhs.temperature, tolerance)
               && LibraryCommon::isEqual(lhs.emissivity, rhs.emissivity, tolerance);
    }

    bool operator!=(const RadiationSurface & lhs, const RadiationSurface & rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const BoundaryCondition & lhs, const BoundaryCondition & rhs)
    {
        return lhs.UUID == rhs.UUID && lhs.Name == rhs.Name && lhs.Protected == rhs.Protected && lhs.Color == rhs.Color
               && lhs.data == rhs.data;
    }

    bool operator!=(const BoundaryCondition & lhs, const BoundaryCondition & rhs)
    {
        return !(lhs == rhs);
    }

    bool arePhysicalPropertiesEqual(const BoundaryCondition & lhs, const BoundaryCondition & rhs)
    {
        return lhs.data == rhs.data;
    }

}   // namespace BCSteadyState