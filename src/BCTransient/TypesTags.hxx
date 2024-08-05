#pragma once

#include <string>
#include <map>
#include <optional>
#include <vector>

#include "Definitions.hxx"

namespace BCTypesLibrary
{
    class StringTags
    {
    public:
        StringTags() = default;
        ~StringTags() = default;

        [[nodiscard]] std::string stringBCType(BCType bcType) const;
        [[nodiscard]] std::vector<std::string> stringsBCType() const;
        [[nodiscard]] std::vector<std::string> stringsBCModel() const;
        [[nodiscard]] std::vector<std::string> stringsDirection() const;
        [[nodiscard]] std::vector<std::string> stringsConvectionModel() const;
        [[nodiscard]] std::vector<std::string> stringsRadiationModel() const;

        const std::map<BCType, std::string> bcType{{BCType::Transient, "Transient"},
                                                   {BCType::SteadyState, "SteadyState"}};

        const std::map<BCModel, std::string> bcModel{{BCModel::Neumann, "Neumann"}, {BCModel::Dirichlet, "Dirichlet"}};

        const std::map<Direction, std::string> direction{{Direction::Windward, "Windward"},
                                                         {Direction::Leeward, "Leeward"}};

        const std::map<ConvectionModel, std::string> convectionModel{
          {ConvectionModel::ASHRAE_NFRC_Inside, "ASHRAE/NFRC Inside"},
          {ConvectionModel::ASHRAE_NFRC_Outside, "ASHRAE/NFRC Outside"},
          {ConvectionModel::Fixed_Convection_Coefficient, "Fixed Convection Coefficient"},
          {ConvectionModel::TARP, "TARP"},
          {ConvectionModel::Kimura, "Kimura"},
          {ConvectionModel::Yazdanian_Klems, "Yazdanian Klems"},
          {ConvectionModel::Montazeri, "Montazeri"}};

        const std::map<RadiationModel, std::string> radiationModel{
          {RadiationModel::Automatic_Enclosure, "Automatic Enclosure"},
          {RadiationModel::Black_Body_Radiation, "Black Body Radiation"},
          {RadiationModel::Fixed_Radiation_Coefficient, "Fixed Radiation Coefficient"}};
    };

    [[nodiscard]] std::string bcTypeToString(BCType bcType);
    [[nodiscard]] BCType bcTypeFromString(std::string_view value);

    [[nodiscard]] std::string bcModelToString(BCModel bcModel);
    [[nodiscard]] BCModel bcModelFromString(std::string_view value);

    [[nodiscard]] std::string directionToString(Direction direction);
    [[nodiscard]] Direction directionFromString(std::string_view value);

    [[nodiscard]] std::string convectionModelToString(ConvectionModel convectionModel);
    [[nodiscard]] ConvectionModel convectionModelFromString(std::string_view value);

    [[nodiscard]] std::string radiationModelToString(RadiationModel radiationModel);
    [[nodiscard]] RadiationModel radiationModelFromString(std::string_view value);
}   // namespace BCTypesLibrary
