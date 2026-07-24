#include <map>

#include "Tags.hxx"

#include "LibraryUtilities/Common.hxx"

namespace BCLibrary
{
    namespace
    {
        const std::map<ConvectionModel, std::string> & convectionModelMap()
        {
            // String values match the legacy transient library so display names stay stable.
            static const std::map<ConvectionModel, std::string> models{
              {ConvectionModel::Fixed_Convection_Coefficient, "Fixed Convection Coefficient"},
              {ConvectionModel::ASHRAE_NFRC_Inside, "ASHRAE/NFRC Inside"},
              {ConvectionModel::ASHRAE_NFRC_Outside, "ASHRAE/NFRC Outside"},
              {ConvectionModel::TARP, "TARP"},
              {ConvectionModel::Yazdanian_Klems, "Yazdanian Klems"},
              {ConvectionModel::Kimura, "Kimura"},
              {ConvectionModel::Montazeri, "Montazeri"}};
            return models;
        }
    }   // namespace

    std::string Tags::boundaryConditions() const
    {
        return boundaryConditions_;
    }

    std::string Tags::boundaryCondition() const
    {
        return boundaryCondition_;
    }

    std::string Tags::version() const
    {
        return version_;
    }

    std::string Tags::uuid() const
    {
        return uuid_;
    }

    std::string Tags::name() const
    {
        return name_;
    }

    std::string Tags::projectName() const
    {
        return projectName_;
    }

    std::string Tags::protectedTag() const
    {
        return protectedTag_;
    }

    std::string Tags::color() const
    {
        return color_;
    }

    std::string Tags::isIGUSurface() const
    {
        return isIGUSurface_;
    }

    std::string Tags::surfaceExchange() const
    {
        return surfaceExchange_;
    }

    std::string Tags::prescribedState() const
    {
        return prescribedState_;
    }

    std::string Tags::radiationSurface() const
    {
        return radiationSurface_;
    }

    std::string Tags::constant() const
    {
        return constant_;
    }

    std::string Tags::fromEnvironment() const
    {
        return fromEnvironment_;
    }

    std::string Tags::model() const
    {
        return model_;
    }

    std::string Tags::convection() const
    {
        return convection_;
    }

    std::string Tags::radiation() const
    {
        return radiation_;
    }

    std::string Tags::solar() const
    {
        return solar_;
    }

    std::string Tags::flux() const
    {
        return flux_;
    }

    std::string Tags::automaticEnclosure() const
    {
        return automaticEnclosure_;
    }

    std::string Tags::manualEnclosure() const
    {
        return manualEnclosure_;
    }

    std::string Tags::blackBodyRadiation() const
    {
        return blackBodyRadiation_;
    }

    std::string Tags::fixedCoefficientRadiation() const
    {
        return fixedCoefficientRadiation_;
    }

    std::string Tags::airTemperature() const
    {
        return airTemperature_;
    }

    std::string Tags::filmCoefficient() const
    {
        return filmCoefficient_;
    }

    std::string Tags::windSpeed() const
    {
        return windSpeed_;
    }

    std::string Tags::windDirection() const
    {
        return windDirection_;
    }

    std::string Tags::temperature() const
    {
        return temperature_;
    }

    std::string Tags::emissivity() const
    {
        return emissivity_;
    }

    std::string Tags::viewFactor() const
    {
        return viewFactor_;
    }

    std::string Tags::coefficient() const
    {
        return coefficient_;
    }

    std::string Tags::relativeHumidity() const
    {
        return relativeHumidity_;
    }

    std::string Tags::irradiance() const
    {
        return irradiance_;
    }

    std::string Tags::absorptance() const
    {
        return absorptance_;
    }

    std::string Tags::isDefault() const
    {
        return isDefault_;
    }

    std::string convectionModelToString(ConvectionModel model)
    {
        return LibraryCommon::enumToString(model, convectionModelMap());
    }

    ConvectionModel convectionModelFromString(std::string_view value)
    {
        return LibraryCommon::enumFromString(value, convectionModelMap());
    }

    std::vector<std::string> convectionModelStrings()
    {
        return LibraryCommon::stringVector(convectionModelMap());
    }
}   // namespace BCLibrary
