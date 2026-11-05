#include "CurrentLibrary.hxx"

#include "BoundaryConditions/DB.hxx"
#include "SteadyStateLibrary.hxx"
#include "TopElement.hxx"

namespace BCLibrary::Import::CurrentLibrary
{
    bool recognizes(const std::string & text)
    {
        return topElement(text) == "BoundaryConditions" && !SteadyStateLibrary::recognizes(text);
    }

    std::vector<BoundaryCondition> records(const std::string & text)
    {
        DB library;
        library.loadFromString(text);
        return library.getBoundaryConditions();
    }
}   // namespace BCLibrary::Import::CurrentLibrary
