#pragma once

#include <map>
#include <string>
#include <vector>

#include "BoundaryConditions/BoundaryConditions.hxx"
#include "Materials/DB.hxx"
#include "THMZ/Model/THMX.hxx"

//! Every conversion of an older THMZ file is listed in one place, Pipeline.cxx: each one lives in
//! its own file in this folder, and nothing outside this folder calls one. Callers open archives
//! with openArchive, and ThermFile::loadThermModelFromEntries runs applyAllToModel, so adding a
//! conversion changes neither THERM nor HygroTherm.
namespace ThermFile::Migration
{
    //! \brief The archive conversions: older content rewritten as entries of the current format,
    //! for conversions whose result is data in the archive rather than a change to the model.
    [[nodiscard]] std::map<std::string, std::string> applyAllToEntries(std::map<std::string, std::string> entries);

    //! \brief Reads an archive and runs the archive conversions on it. This is how an archive is
    //! opened for loading a model: throws what ThermZip::unzipFiles throws.
    [[nodiscard]] std::map<std::string, std::string> openArchive(const std::string & zipFileName);

    //! \brief The model conversions, returning the converted model. Each one reads what it needs
    //! from the archive entries. Running them on a converted model changes nothing. The returned
    //! model carries the current version, so version-gated conversions run once per file: a
    //! later save records it.
    [[nodiscard]] ThermFile::ThermModel applyAllToModel(const std::map<std::string, std::string> & entries,
                                                        ThermFile::ThermModel model);

    //! \brief The materials conversions, for a caller that loads the archive's materials itself.
    [[nodiscard]] MaterialsLibrary::DB applyAllToMaterials(const std::map<std::string, std::string> & entries,
                                                           MaterialsLibrary::DB materials);

    //! \brief The boundary conditions of an archive written before they were unified, as unified
    //! records. Empty for an archive with none.
    [[nodiscard]] std::vector<BCLibrary::BoundaryCondition>
      legacyBoundaryConditions(const std::map<std::string, std::string> & entries);
}   // namespace ThermFile::Migration
