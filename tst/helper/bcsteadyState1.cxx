#include <BCSteadyState/Tags.hxx>

#include "bcsteadyState1.hxx"

namespace TestSteadyStateBC {

    std::string testDatabase() {
        BCSteadyStateLibrary::Tags tags;
        std::string fileContent{"<" + tags.boundaryConditions() + ">\n"};
        fileContent += comprehensiveBC1();
        fileContent += simplifiedBC1();
        fileContent += radiationSurfaceBC1();
        fileContent += radiationSurfaceBCDefault();
        fileContent += "</" + tags.boundaryConditions() + ">";
        return fileContent;
    }

    std::string comprehensiveBC1() {
        return "\t<BoundaryCondition>\n"
               "\t\t<UUID>4c179ded-3f34-4102-b8d6-46d8e6d1c6d9</UUID>\n"
               "\t\t<Name>Interior (20 tilt) Aluminum Frame (convection only)</Name>\n"
               "\t\t<Protected>true</Protected>\n"
               "\t\t<Color>0xFF0000</Color>\n"
               "\t\t<Comprehensive>\n"
               "\t\t\t<RelativeHumidity>0.5</RelativeHumidity>\n"
               "\t\t\t<Convection>\n"
               "\t\t\t\t<Temperature>21</Temperature>\n"
               "\t\t\t\t<FilmCoefficient>4.65</FilmCoefficient>\n"
               "\t\t\t</Convection>\n"
               "\t\t\t<Radiation>\n"
               "\t\t\t\t<AutomaticEnclosure>\n"
               "\t\t\t\t\t<Temperature>21</Temperature>\n"
               "\t\t\t\t\t<Emissivity>1</Emissivity>\n"
               "\t\t\t\t</AutomaticEnclosure>\n"
               "\t\t\t</Radiation>\n"
               "\t\t</Comprehensive>\n"
               "\t</BoundaryCondition>\n";
    }

    std::string simplifiedBC1() {
        return "\t<BoundaryCondition>\n"
               "\t\t<UUID>61d7bd1c-22c6-4ea0-8720-0696e8c194ad</UUID>\n"
               "\t\t<Name>Adiabatic</Name>\n"
               "\t\t<Protected>true</Protected>\n"
               "\t\t<Color>0xFFFFFF</Color>\n"
               "\t\t<Simplified>\n"
               "\t\t\t<Temperature>0</Temperature>\n"
               "\t\t\t<FilmCoefficient>0</FilmCoefficient>\n"
               "\t\t\t<RelativeHumidity>0.5</RelativeHumidity>\n"
               "\t\t</Simplified>\n"
               "\t</BoundaryCondition>\n";
    }

    std::string radiationSurfaceBC1() {
        return "\t<BoundaryCondition>\n"
               "\t\t<UUID>d2539cd7-c6e6-4265-b2b9-dc482b1e1798</UUID>\n"
               "\t\t<Name>Radiation Surface</Name>\n"
               "\t\t<Protected>true</Protected>\n"
               "\t\t<Color>0x00FF00</Color>\n"
               "\t\t<RadiationSurface>\n"
               "\t\t\t<Temperature>21</Temperature>\n"
               "\t\t\t<Emissivity>1</Emissivity>\n"
               "\t\t</RadiationSurface>\n"
               "\t</BoundaryCondition>\n";
    }

    std::string radiationSurfaceBCDefault() {
        return "\t<BoundaryCondition>\n"
               "\t\t<UUID>04e294fb-beaa-4838-b663-a05e0942bb72</UUID>\n"
               "\t\t<Name>Default Radiation Surface</Name>\n"
               "\t\t<Protected>true</Protected>\n"
               "\t\t<Color>0xFF0000</Color>\n"
               "\t\t<RadiationSurface>\n"
               "\t\t\t<Default>true</Default>\n"
               "\t\t\t<Temperature>21</Temperature>\n"
               "\t\t\t<Emissivity>1</Emissivity>\n"
               "\t\t</RadiationSurface>\n"
               "\t</BoundaryCondition>\n";
    }

}