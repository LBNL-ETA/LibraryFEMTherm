#pragma once

#include <string>
#include <vector>
#include <optional>


#include "Tags.hxx"

namespace GasesLibrary
{
    struct Coefficients
    {
        double A{0.0};
        double B{0.0};
        double C{0.0};
    };

    //! \brief Structure to hold properties of gas. It will be used by two different structures.
    struct GasProperties
    {
        double MolecularWeight{0.0};
        double SpecificHeatRatio{0.0};
        Coefficients Conductivity;
        Coefficients Viscosity;
        Coefficients SpecificHeat;
    };

    struct GasCommon
    {
        std::string UUID;
        std::string Name;
        bool Protected{false};
    };

    struct PureGas : public GasCommon
    {
        GasProperties Properties;

        //! This is used to assign record to certain project.
        //! This property is not (and should not be) saved into the XML file.
        std::string ProjectName{""};
    };

    struct Component
    {
        double Fraction{0.0};
        std::string PureGasName;
    };

    struct Gas : public GasCommon
    {
        std::vector<Component> Components;

        //! This is used to assign record to certain project.
        //! This property is not (and should not be) saved into the XML file.
        std::string ProjectName{""};
    };

    //! \brief This is used to create complete gas data structure needed for calculations and serialization. It can also
    //! be used to validate gas completeness in the XML library.
    struct GasesData
    {
        Gas gas;
        std::vector<std::optional<PureGas>> components;

        std::string Name{""};
        std::string ProjectName{""};
        bool Protected{false};
    };

    [[nodiscard]] bool isGasesDataComplete(const GasesData & data);

}   // namespace GasesLibrary
