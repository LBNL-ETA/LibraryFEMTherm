#include <algorithm>
#include <exception>
#include <filesystem>

#include "THMZ/Mesh/DB.hxx"
#include "THMZ/Model/DB.hxx"
#include "THMZ/SteadyStateMeshResults/DB.hxx"
#include "THMZ/SteadyStateResults/DB.hxx"
#include "THMZ/ZipModule/ZipModule.hxx"

#include "Archive.hxx"

namespace ThermArchive
{
    namespace
    {
        using Failure = lbnl::Unexpected<std::string>;

        //! What a part's loader returns for an entry the archive does hold.
        template<typename Type>
        Part<Type> readOrFailure(std::optional<Type> loaded, const std::string & partName)
        {
            if(!loaded.has_value())
            {
                return Failure{"The archive's " + partName + " could not be read"};
            }
            return loaded;
        }

        //! The part an existing loader reads from the archive's file: nothing when the archive
        //! has no such entry, a failure when it has one the loader cannot read or the file
        //! can no longer be opened.
        template<typename Loader>
        auto part(const Archive & archive, const std::string & baseName, const Loader & loader)
          -> Part<typename decltype(loader(archive.fileName))::value_type>
        {
            using Type = typename decltype(loader(archive.fileName))::value_type;
            if(!hasEntry(archive, baseName))
            {
                return std::optional<Type>{};
            }
            try
            {
                return readOrFailure(loader(archive.fileName), baseName);
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
                const auto entries{ThermZip::unzipFiles(archive.fileName, ThermZip::entryNameCandidates(baseName))};
                Library result;
                result.fromString(ThermZip::findEntry(entries, baseName));
                return std::optional<Library>{std::move(result)};
            }
            catch(const std::exception & failure)
            {
                return Failure{failure.what()};
            }
        }

        bool hasEntryUnder(const Archive & archive, const std::string & directory)
        {
            const std::string prefix{directory + "/"};
            return std::ranges::any_of(archive.entryNames,
                                       [&prefix](const std::string & name) { return name.starts_with(prefix); });
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
        return part(archive, ThermZip::ModelFileName, &ThermFile::loadThermModelFromZipFile);
    }

    Part<ThermFile::Mesh::Mesh> mesh(const Archive & archive)
    {
        return part(archive, ThermZip::MeshName, &ThermFile::Mesh::loadMeshFromZipFile);
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

    lbnl::ExpectedExt<std::vector<TimeSeriesLibrary::TimeSeriesData>, std::string> datasets(const Archive & archive)
    {
        if(!hasEntryUnder(archive, ThermZip::TimeSeriesDir))
        {
            return std::vector<TimeSeriesLibrary::TimeSeriesData>{};
        }
        try
        {
            return TimeSeriesLibrary::loadDatasetsFromZipFile(archive.fileName);
        }
        catch(const std::exception & failure)
        {
            return Failure{failure.what()};
        }
    }

    Part<ThermFile::SteadyStateResults> steadyStateResults(const Archive & archive)
    {
        return part(archive, ThermZip::SteadyStateResultsName, &ThermFile::loadSteadyStateResultsFromZipFile);
    }

    Part<ThermFile::MeshResults> steadyStateMeshResults(const Archive & archive)
    {
        return part(archive, ThermZip::SteadyStateMeshResultsName, &ThermFile::loadSteadyStateMeshResultsFromZipFile);
    }

    Part<ThermFile::SteadyStateField::Field> steadyStateField(const Archive & archive, const ThermFile::RunType runType)
    {
        using ThermFile::SteadyStateField::Field;
        if(!hasEntry(archive, ThermZip::MeshName) || !hasEntry(archive, ThermZip::SteadyStateMeshResultsName))
        {
            return std::optional<Field>{};
        }
        auto field{ThermFile::SteadyStateField::loadFromZipFile(archive.fileName, runType)};
        if(!field.has_value())
        {
            return Failure{field.error()};
        }
        return std::optional<Field>{std::move(field.value())};
    }

    Part<ThermFile::TransientResults::Results> transientResults(const Archive & archive)
    {
        using ThermFile::TransientResults::Results;
        if(!hasEntryUnder(archive, ThermZip::ResultsDirPath))
        {
            return std::optional<Results>{};
        }
        try
        {
            auto results{ThermFile::TransientResults::loadFromZipFile(archive.fileName)};
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
