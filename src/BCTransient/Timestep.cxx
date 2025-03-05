#include <fileParse/XMLNodeAdapter.hxx>
#include <fileParse/Vector.hxx>

#include "Timestep.hxx"

#include "TimestepSerializers.hxx"

namespace BCInputFileLibrary
{


    void BoundaryConditionTimestep::loadFromXMLFile(const std::string & fileName)
    {
        const auto xInputDataNode{getTopNodeFromFile(fileName, "InputBoundaryConditionsData")};

        if(xInputDataNode.has_value())
        {
            xInputDataNode.value() >> FileParse::Child{"Version", version};
            xInputDataNode.value() >> FileParse::Child{"Name", name};

            auto optTransient{xInputDataNode->getFirstChildByName("BoundaryConditionTransient")};
            if(optTransient)
            {
                const XMLNodeAdapter& xBCTransient{optTransient.value()};
                xBCTransient >> convection;
                xBCTransient >> radiation;
                xBCTransient >> FileParse::Child{"TemperatureTimestep", temperature};
                xBCTransient >> FileParse::Child{"HumidityTimestep", humidity};
                xBCTransient >> FileParse::Child{"HeatFluxTimestep", heatFlux};
                xBCTransient >> FileParse::Child{"SolarRadiationTimestep", solarRadiation};
            }
        }
    }

    void BoundaryConditionTimestep::loadFromXMLString(const std::string &xmlString) {
        const auto xInputDataNode{getTopNodeFromString(xmlString, "InputBoundaryConditionsData")};

        if(xInputDataNode.has_value())
        {
            xInputDataNode.value() >> FileParse::Child{"Version", version};
            xInputDataNode.value() >> FileParse::Child{"Name", name};

            auto optTransient{xInputDataNode->getFirstChildByName("BoundaryConditionTransient")};
            if(optTransient)
            {
                const XMLNodeAdapter& xBCTransient{optTransient.value()};
                xBCTransient >> convection;
                xBCTransient >> radiation;
                xBCTransient >> FileParse::Child{"TemperatureTimestep", temperature};
                xBCTransient >> FileParse::Child{"HumidityTimestep", humidity};
                xBCTransient >> FileParse::Child{"HeatFluxTimestep", heatFlux};
                xBCTransient >> FileParse::Child{"SolarRadiationTimestep", solarRadiation};
            }
        }
    }

    bool BoundaryConditionTimestep::isLoadSuccesful() const
    {
        return true;
    }

    std::string BoundaryConditionTimestep::errorMessage() const
    {
        return "";
    }

}   // namespace BCInputFileLibrary
