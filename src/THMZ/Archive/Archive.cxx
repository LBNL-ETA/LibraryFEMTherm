#include <algorithm>
#include <exception>
#include <filesystem>
#include <iterator>

#include "THMZ/Mesh/DB.hxx"
#include "THMZ/Model/DB.hxx"
#include "THMZ/Model/migration/Pipeline.hxx"
#include "THMZ/SteadyStateMeshResults/DB.hxx"
#include "THMZ/SteadyStateResults/DB.hxx"
#include "THMZ/ZipModule/ZipModule.hxx"

#include "Archive.hxx"

namespace ThermArchive
{
    namespace
    {
        using Failure = lbnl::Unexpected<std::string>;

        //! What a part's parser returns for an entry the archive does hold.
        template<typename Type>
        Part<Type> readOrFailure(std::optional<Type> parsed, const std::string & partName)
        {
            if(!parsed.has_value())
            {
                return Failure{"The archive's " + partName + " could not be read"};
            }
            return parsed;
        }

        //! The text of a part's entry, under whichever format's spelling the archive holds.
        std::string entryText(const Archive & archive, const std::string & baseName)
        {
            const auto entries{ThermZip::unzipFiles(archive.fileName, ThermZip::entryNameCandidates(baseName))};
            return ThermZip::findEntry(entries, baseName);
        }

        //! A part parsed from the text of its entry: nothing when the archive has no such
        //! entry, a failure when it has one the parser cannot read or the file can no longer
        //! be opened.
        template<typename Parser>
        auto parsed(const Archive & archive, const std::string & baseName, const Parser & parser)
          -> Part<typename decltype(parser(std::string{}))::value_type>
        {
            using Type = typename decltype(parser(std::string{}))::value_type;
            if(!hasEntry(archive, baseName))
            {
                return std::optional<Type>{};
            }
            try
            {
                return readOrFailure(parser(entryText(archive, baseName)), baseName);
            }
            catch(const std::exception & failure)
            {
                return Failure{failure.what()};
            }
        }

        //! A library read from the text of its entry.
        template<typename Library>
        Part<Library> library(const Archive & archive, const std::string & baseName)
        {
            if(!hasEntry(archive, baseName))
            {
                return std::optional<Library>{};
            }
            try
            {
                Library result;
                result.fromString(entryText(archive, baseName));
                return std::optional<Library>{std::move(result)};
            }
            catch(const std::exception & failure)
            {
                return Failure{failure.what()};
            }
        }

        //! An entry name with forward slashes, as the archive's constants spell a directory.
        //! A zip written on Windows may hold backslashes.
        std::string withForwardSlashes(std::string name)
        {
            std::ranges::replace(name, '\\', '/');
            return name;
        }

        //! The entries inside a directory of the archive, by the names the archive stores.
        std::vector<std::string> entriesUnder(const Archive & archive, const std::string & directory)
        {
            const std::string prefix{directory + "/"};
            std::vector<std::string> names;
            std::ranges::copy_if(archive.entryNames, std::back_inserter(names), [&prefix](const std::string & name) {
                return withForwardSlashes(name).starts_with(prefix);
            });
            return names;
        }
    }   // namespace

    lbnl::ExpectedExt<Archive, std::string> read(const std::string & fileName)
    {
        if(!std::filesystem::exists(fileName))
        {
            return Failure{"No such archive: " + fileName};
        }
        try
        {
            return Archive{.fileName = fileName, .entryNames = ThermZip::entryNames(fileName)};
        }
        catch(const std::exception &)
        {
            return Failure{"Not a readable archive: " + fileName};
        }
    }

    bool hasEntry(const Archive & archive, const std::string & baseName)
    {
        const auto candidates{ThermZip::entryNameCandidates(baseName)};
        return std::ranges::any_of(candidates, [&archive](const std::string & candidate) {
            return std::ranges::find(archive.entryNames, candidate) != archive.entryNames.end();
        });
    }

    Part<std::string> entry(const Archive & archive, const std::string & name)
    {
        if(std::ranges::find(archive.entryNames, name) == archive.entryNames.end())
        {
            return std::optional<std::string>{};
        }
        try
        {
            return std::optional<std::string>{ThermZip::unzipFile(archive.fileName, name)};
        }
        catch(const std::exception & failure)
        {
            return Failure{failure.what()};
        }
    }

    lbnl::ExpectedExt<std::map<std::string, std::string>, std::string>
      entries(const Archive & archive, const std::vector<std::string> & names)
    {
        try
        {
            return ThermZip::unzipFiles(archive.fileName, names);
        }
        catch(const std::exception & failure)
        {
            return Failure{failure.what()};
        }
    }

    Part<ThermFile::ThermModel> model(const Archive & archive)
    {
        if(!hasEntry(archive, ThermZip::ModelFileName))
        {
            return std::optional<ThermFile::ThermModel>{};
        }
        try
        {
            // Bringing a model up to the current version reads other entries too, so the
            // whole archive is opened.
            return readOrFailure(
              ThermFile::loadThermModelFromEntries(ThermFile::Migration::openArchive(archive.fileName)),
              ThermZip::ModelFileName);
        }
        catch(const std::exception & failure)
        {
            return Failure{failure.what()};
        }
    }

    Part<ThermFile::Mesh::Mesh> mesh(const Archive & archive)
    {
        return parsed(archive, ThermZip::MeshName, [](const std::string & text) {
            return ThermFile::Mesh::meshFromString(text, FileParse::FileFormat::Unknown);
        });
    }

    Part<MaterialsLibrary::DB> materials(const Archive & archive)
    {
        return library<MaterialsLibrary::DB>(archive, ThermZip::MaterialsFileName);
    }

    Part<GasesLibrary::DB> gases(const Archive & archive)
    {
        return library<GasesLibrary::DB>(archive, ThermZip::GasesFileName);
    }

    Part<BCLibrary::DB> boundaryConditions(const Archive & archive)
    {
        return library<BCLibrary::DB>(archive, ThermZip::BoundaryConditionsFileName);
    }

    Part<BCSteadyStateLibrary::DB> legacySteadyStateBoundaryConditions(const Archive & archive)
    {
        return library<BCSteadyStateLibrary::DB>(archive, ThermZip::SteadyStateBCFileName);
    }

    lbnl::ExpectedExt<std::vector<TimeSeriesLibrary::TimeSeriesData>, std::string> datasets(const Archive & archive)
    {
        const auto names{entriesUnder(archive, ThermZip::TimeSeriesDir)};
        if(names.empty())
        {
            return std::vector<TimeSeriesLibrary::TimeSeriesData>{};
        }
        try
        {
            return TimeSeriesLibrary::datasetsFromThmzEntries(ThermZip::unzipFiles(archive.fileName, names));
        }
        catch(const std::exception & failure)
        {
            return Failure{failure.what()};
        }
    }

    Part<ThermFile::SteadyStateResults> steadyStateResults(const Archive & archive)
    {
        return parsed(archive, ThermZip::SteadyStateResultsName, [](const std::string & text) {
            return ThermFile::steadyStateResultsFromString(text, FileParse::FileFormat::Unknown);
        });
    }

    Part<ThermFile::MeshResults> steadyStateMeshResults(const Archive & archive)
    {
        return parsed(archive, ThermZip::SteadyStateMeshResultsName, [](const std::string & text) {
            return ThermFile::steadyStateMeshResultsFromString(text);
        });
    }

    Part<ThermFile::SteadyStateField::Field> steadyStateField(const Archive & archive, const ThermFile::RunType runType)
    {
        using ThermFile::SteadyStateField::Field;
        if(!hasEntry(archive, ThermZip::MeshName) || !hasEntry(archive, ThermZip::SteadyStateMeshResultsName))
        {
            return std::optional<Field>{};
        }
        try
        {
            auto names{ThermZip::entryNameCandidates(ThermZip::MeshName)};
            const auto results{ThermZip::entryNameCandidates(ThermZip::SteadyStateMeshResultsName)};
            names.insert(names.end(), results.begin(), results.end());
            auto field{ThermFile::SteadyStateField::read(ThermZip::unzipFiles(archive.fileName, names), runType)};
            if(!field.has_value())
            {
                return Failure{field.error()};
            }
            return std::optional<Field>{std::move(field.value())};
        }
        catch(const std::exception & failure)
        {
            return Failure{failure.what()};
        }
    }

    Part<ThermFile::TransientResults::Results> transientResults(const Archive & archive)
    {
        using ThermFile::TransientResults::Results;
        const auto names{entriesUnder(archive, ThermZip::ResultsDirPath)};
        if(names.empty())
        {
            return std::optional<Results>{};
        }
        try
        {
            auto results{ThermFile::TransientResults::read(ThermZip::unzipFiles(archive.fileName, names))};
            if(!results.has_value())
            {
                return Failure{results.error()};
            }
            return std::optional<Results>{std::move(results.value())};
        }
        catch(const std::exception & failure)
        {
            return Failure{failure.what()};
        }
    }
}   // namespace ThermArchive
