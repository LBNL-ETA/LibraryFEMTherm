#include "Gases.hxx"

#include "Tags.hxx"

namespace GasesLibrary
{
    bool isGasesDataComplete(const GasesData & data)
    {
        bool result{true};

        // check if each value in data.components has_value is true
        for(const auto & component : data.components)
        {
            if(!component.has_value())
            {
                result = false;
                break;
            }
        }

        return result;
    }
}   // namespace GasesLibrary
