#include "TransientLibrary.hxx"

#include <lbnl/algorithm.hxx>

#include "Legacy/Step1/BCTransient/Types.hxx"
#include "Legacy/Step1/Converters/Converters.hxx"
#include "TopElement.hxx"

namespace BCLibrary::Import::TransientLibrary
{
    bool recognizes(const std::string & text)
    {
        return topElement(text) == "BoundaryConditionsType";
    }

    std::vector<BoundaryCondition> records(const std::string & text)
    {
        BCTypesLibrary::DB legacy;
        legacy.loadFromString(text);
        return lbnl::transform_to_vector(legacy.getBoundaryConditions(), [](const BCTypesLibrary::TypeRecord & record) {
            return fromTypeRecord(record);
        });
    }
}   // namespace BCLibrary::Import::TransientLibrary
