#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "BoundaryConditions/BoundaryConditions.hxx"

//! Boundary condition library files of every version THERM reads, as unified records. Every
//! format has one file in this folder that recognizes it and converts it, and Pipeline.cxx holds
//! the only list of formats. Callers read the text themselves and name no format, so adding one
//! changes nothing outside this folder.
namespace BCLibrary::Import
{
    struct ImportedRecords
    {
        std::vector<BoundaryCondition> records;
        //! Radiation surfaces, which are internal companions the library list does not show.
        std::size_t notApplicable{0u};
        //! Records of a single project, which leave the library when the document closes.
        std::size_t projectRecords{0u};
    };

    //! \brief The records of a library file, in unified form. An empty answer means the text is
    //! not a boundary condition library of any known format.
    [[nodiscard]] std::optional<ImportedRecords> recordsFromLibrary(const std::string & text);
}   // namespace BCLibrary::Import
