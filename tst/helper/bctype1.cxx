#include "bctype1.hxx"

namespace TestBCType
{
    std::string testDatabase()
    {
        std::string fileContent{"<BoundaryConditionsType>\n"};
        fileContent += bcType1();
        fileContent += bcType2();
        fileContent += "</BoundaryConditionsType>";
        return fileContent;
    }

    std::string testDatabaseBC1()
    {
        std::string fileContent{"<BoundaryConditionsType>\n"};
        fileContent += bcType1();
        fileContent += "</BoundaryConditionsType>";
        return fileContent;
    }

    std::string testDatabaseBC2()
    {
        std::string fileContent{"<BoundaryConditionsType>\n"};
        fileContent += bcType2();
        fileContent += "</BoundaryConditionsType>";
        return fileContent;
    }

    std::string bcType1()
    {
        const std::string fileContent{"<BoundaryConditionType>\n"
                                      "  <UUID>d5bea3f2-b241-11e9-a2a3-2a2ae2dbcce4</UUID>\n"
                                      "  <Name>Test name</Name>\n"
                                      "  <Protected>true</Protected>\n"
                                      "  <BCType>Transient</BCType>\n"
                                      "  <BCModel>Neumann</BCModel>\n"
                                      "  <Convection>\n"
                                      "    <Model>Fixed Convection Coefficient</Model>\n"
                                      "  </Convection>\n"
                                      "  <Radiation>\n"
                                      "    <Model>Automatic Enclosure</Model>\n"
                                      "  </Radiation>\n"
                                      "  <UseHeatFlux>false</UseHeatFlux>\n"
                                      "  <UseTemperature>false</UseTemperature>\n"
                                      "  <UseHumidity>false</UseHumidity>\n"
                                      "  <Color>0xFF0000</Color>\n"
                                      "</BoundaryConditionType>"};

        return fileContent;
    }

    std::string bcType2()
    {
        const std::string fileContent{"<BoundaryConditionType>\n"
                                      "  <UUID>d5bea8e8-b241-11e9-a2a3-2a2ae2dbcce4</UUID>\n"
                                      "  <Name>SteadyState Interior</Name>\n"
                                      "  <Protected>true</Protected>\n"
                                      "  <BCType>SteadyState</BCType>\n"
                                      "  <BCModel>Neumann</BCModel>\n"
                                      "  <Convection>\n"
                                      "    <Model>Fixed Convection Coefficient</Model>\n"
                                      "    <Temperature>21</Temperature>\n"
                                      "    <FilmCoefficient>2.5</FilmCoefficient>\n"
                                      "  </Convection>\n"
                                      "  <Radiation>\n"
                                      "    <Model>Automatic Enclosure</Model>\n"
                                      "    <Temperature>10</Temperature>\n"
                                      "    <SurfaceEmissivity>0.9</SurfaceEmissivity>\n"
                                      "    <ViewFactor>1.0</ViewFactor>\n"
                                      "  </Radiation>\n"
                                      "  <UseHeatFlux>false</UseHeatFlux>\n"
                                      "  <UseTemperature>false</UseTemperature>\n"
                                      "  <UseHumidity>false</UseHumidity>\n"
                                      "  <Color>0x00FF00</Color>\n"
                                      "</BoundaryConditionType>"};

        return fileContent;
    }
}   // namespace TestBCType
