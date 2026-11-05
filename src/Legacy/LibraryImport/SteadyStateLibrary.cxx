#include "SteadyStateLibrary.hxx"

#include <lbnl/algorithm.hxx>

#include "Legacy/Step1/BCSteadyState/DB.hxx"
#include "Legacy/Step1/Converters/Converters.hxx"
#include "TopElement.hxx"

namespace BCLibrary::Import::SteadyStateLibrary
{
    bool recognizes(const std::string & text)
    {
        return topElement(text) == "BoundaryConditions"
               && (text.find("<Comprehensive>") != std::string::npos || text.find("<Simplified>") != std::string::npos);
    }

    std::vector<BoundaryCondition> records(const std::string & text)
    {
        BCSteadyStateLibrary::DB legacy;
        legacy.loadFromString(text);
        return lbnl::transform_to_vector(legacy.getBoundaryConditions(),
                                         [](const BCSteadyStateLibrary::BoundaryCondition & record) {
                                             return fromSteadyState(record);
                                         });
    }
}   // namespace BCLibrary::Import::SteadyStateLibrary
