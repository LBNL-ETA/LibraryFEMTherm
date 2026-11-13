#pragma once

#include <map>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

#include <lbnl/expected.hxx>

#include "BoundaryConditions/DB.hxx"
#include "Gases/DB.hxx"
#include "Legacy/Step1/BCSteadyState/DB.hxx"
#include "Materials/DB.hxx"
#include "SteadyStateField/SteadyStateField.hxx"
#include "THMZ/Common/Enumerators.hxx"
#include "THMZ/Mesh/Mesh.hxx"
#include "THMZ/Model/THMX.hxx"
#include "THMZ/SteadyStateMeshResults/Results.hxx"
#include "THMZ/SteadyStateResults/Results.hxx"
#include "TimeSeriesData/DB.hxx"
#include "TransientResults/TransientResults.hxx"

//! A THERM archive (.thmz) as one value, and its parts as functions of that value.
//!
//! Reading an archive reads the list of its entries and nothing else. Each part is read from
//! the file when it is asked for, and again each time it is asked for: the archive holds no
//! content, so a caller that wants to keep a part keeps what the function returned.
//!
//! One rule for what can go wrong:
//!   - the file is missing or is not a readable archive: read() fails;
//!   - a part is not in the archive: the part is empty (nullopt, or no datasets);
//!   - a part is in the archive but cannot be read: asking for it fails, naming the part.
namespace ThermArchive
{
    //! Where an archive is and the names of the entries it holds.
    struct Archive
    {
        std::string fileName;
        std::vector<std::string> entryNames;
    };

    //! A part of an archive: the part, nothing when the archive has none, or why it could
    //! not be read.
    template<typename Type>
    using Part = lbnl::ExpectedExt<std::optional<Type>, std::string>;

    //! Opens the archive and reads the names of its entries. Fails when the file does not
    //! exist or is not a readable archive.
    [[nodiscard]] lbnl::ExpectedExt<Archive, std::string> read(const std::string & fileName);

    //! True when the archive holds the entry under either format's spelling
    //! ("Model" for Model.xml or Model.json).
    [[nodiscard]] bool hasEntry(const Archive & archive, const std::string & baseName);

    //! The text of one entry by its full name; nothing when the archive has no such entry.
    [[nodiscard]] Part<std::string> entry(const Archive & archive, const std::string & name);

    //! The text of the named entries, name -> content; every entry when no name is given.
    //! A name the archive does not hold is left out of the result.
    [[nodiscard]] lbnl::ExpectedExt<std::map<std::string, std::string>, std::string>
      entries(const Archive & archive, const std::vector<std::string> & names = {});

    //! The model, brought up to the current version as every loader of a model does.
    [[nodiscard]] Part<ThermFile::ThermModel> model(const Archive & archive);

    [[nodiscard]] Part<ThermFile::Mesh::Mesh> mesh(const Archive & archive);

    [[nodiscard]] Part<MaterialsLibrary::DB> materials(const Archive & archive);

    [[nodiscard]] Part<GasesLibrary::DB> gases(const Archive & archive);

    //! The unified boundary-condition library; nothing for an archive written before the
    //! libraries were unified.
    [[nodiscard]] Part<BCLibrary::DB> boundaryConditions(const Archive & archive);

    //! The steady-state boundary-condition library of an archive written before the libraries
    //! were unified (its SteadyStateBC entry); nothing for an archive that has none.
    [[nodiscard]] Part<BCSteadyStateLibrary::DB> legacySteadyStateBoundaryConditions(const Archive & archive);

    //! Every time series dataset the archive stores; none is an empty list, not a failure.
    [[nodiscard]] lbnl::ExpectedExt<std::vector<TimeSeriesLibrary::TimeSeriesData>, std::string>
      datasets(const Archive & archive);

    [[nodiscard]] Part<ThermFile::SteadyStateResults> steadyStateResults(const Archive & archive);

    [[nodiscard]] Part<ThermFile::MeshResults> steadyStateMeshResults(const Archive & archive);

    //! The stored mesh joined with the nodal results of one run; nothing when the archive
    //! lacks the mesh or the nodal results. Fails when both are there but neither has a case
    //! of that run type.
    [[nodiscard]] Part<ThermFile::SteadyStateField::Field> steadyStateField(const Archive & archive,
                                                                            ThermFile::RunType runType);

    [[nodiscard]] Part<ThermFile::TransientResults::Results> transientResults(const Archive & archive);

    //! A part for a caller that treats every way of not getting it alike: the part, or
    //! nothing when the archive could not be read, has no such part, or the part could not
    //! be read. `reader` is one of the part functions above:
    //!     const auto archive{ThermArchive::read(fileName)};
    //!     const auto mesh{ThermArchive::partOrNothing(archive, &ThermArchive::mesh)};
    template<typename Reader>
    [[nodiscard]] auto partOrNothing(const lbnl::ExpectedExt<Archive, std::string> & archive, const Reader & reader)
      -> std::remove_cvref_t<decltype(reader(archive.value()).value())>
    {
        using Result = std::remove_cvref_t<decltype(reader(archive.value()).value())>;
        if(!archive.has_value())
        {
            return Result{};
        }
        const auto part{reader(archive.value())};
        return part.has_value() ? part.value() : Result{};
    }
}   // namespace ThermArchive
