#pragma once

namespace BCTypesLibrary
{
    enum class BCType
    {
        Transient,
        SteadyState
    };

    enum class BCModel
    {
        Neumann,
        Dirichlet
    };

    enum class Direction
    {
        Leeward,
        Windward
    };

    enum class ConvectionModel
    {
        ASHRAE_NFRC_Inside,
        ASHRAE_NFRC_Outside,
        Fixed_Convection_Coefficient,
        TARP,
        Kimura,
        Yazdanian_Klems,
        Montazeri
    };

    enum class RadiationModel
    {
        Automatic_Enclosure,
        Black_Body_Radiation,
        Fixed_Radiation_Coefficient
    };
}