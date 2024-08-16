#include "ZipModule.hxx"

#include <map>

#include <miniz.h>
#include <sstream>
#include <iterator>
#include <fstream>
#include <algorithm>

#include "Common/Common.hxx"

namespace ThermZip
{
    namespace
    {
        std::vector<std::string> buildTransientFiles(std::string_view directory, const std::vector<std::string> & files)
        {
            std::vector<std::string> result;
            result.reserve(files.size());

            std::string transientDir = std::string(directory).append(TimestepFilesDir + "\\");
            Common::createDirectoryIfNotExists(transientDir);

            for(const auto & file : files)
            {
                result.push_back(transientDir + file);
            }

            return result;
        }
    }   // namespace

    const std::map<File, std::string> FileToString = {{File::Gases, GasesFileName},
                                                      {File::Materials, MaterialsFileName},
                                                      {File::SteadyStateBC, SteadyStateBCFileName},
                                                      {File::TransientTypeBC, TransientTypeBCFileName}};

    std::string toString(File file)
    {
        if(FileToString.find(file) != FileToString.end())
        {
            return FileToString.at(file);
        }
        return {};
    }

    File toFile(const std::string & file)
    {
        for(const auto & entry : FileToString)
        {
            if(entry.second == file)
            {
                return entry.first;
            }
        }
        return File::Unknown;
    }

    const std::map<Results, std::string> ResultsToString = {{Results::Geometry, GeometryFileName},
                                                            {Results::HeatFlux, HeatFluxFileName},
                                                            {Results::HeatFluxEdges, HeatFluxEdgesFileName},
                                                            {Results::Humidities, HumidityFileName},
                                                            {Results::Temperatures, TemperatureFileName},
                                                            {Results::WaterContent, WaterContentFileName},
                                                            {Results::WaterFlux, WaterFluxFileName},
                                                            {Results::WaterFluxEdges, WaterFluxEdgesFileName}};

    std::string toString(Results results)
    {
        if(ResultsToString.find(results) != ResultsToString.end())
        {
            return ResultsToString.at(results);
        }
        return {};
    }

    Results toResults(const std::string & results)
    {
        for(const auto & entry : ResultsToString)
        {
            if(entry.second == results)
            {
                return entry.first;
            }
        }
        return Results::Unknown;
    }

    std::string resultsDirectory(const std::string & directory)
    {
        return directory + "\\" + ResultsDir;
    }

    std::string timestepFilesDirectory(const std::string & directory)
    {
        return directory + "\\" + TimestepFilesDir;
    }

    std::string modelFileName(const std::string & directory)
    {
        return directory + "\\" + ModelFileName;
    }

    std::string steadyStateResultsName(const std::string & directory)
    {
        return directory + "\\" + SteadyStateResultsName;
    }

    std::string steadyStateMeshResultsName(const std::string & directory)
    {
        return directory + "\\" + SteadyStateMeshResultsName;
    }

    std::string meshName(const std::string & directory)
    {
        return directory + "\\" + MeshName;
    }

    namespace Helper
    {
        // Utility function for creating a relative path
        std::string createRelativePath(const std::string & fullPath, const std::string & basePath)
        {
            // Check if the base path ends with a directory separator and adjust the offset accordingly
            std::size_t basePathLength = basePath.length();
            // Check if the base path already ends with a path separator
            if(basePath.back() == '\\' || basePath.back() == '/')
            {
                basePathLength -= 1;   // No need to skip the separator if it's at the end of basePath
            }

            std::string::size_type pos = fullPath.find(basePath);
            if(pos != std::string::npos)
            {
                // Correctly offset by the base path length and the separator (if not at the end of basePath)
                return fullPath.substr(pos + basePathLength + 1);
            }
            return fullPath;   // Return the original path if basePath not found
        }
    }   // namespace Helper

    bool zipFiles(std::map<std::string, std::string> const& data, const std::string& destinationZipFile)
    {
        return true;
    }

    bool zipFiles(const std::string & sourceDirectory, const std::string & destinationZipFile)
    {
        mz_zip_archive zip_archive;
        memset(&zip_archive, 0, sizeof(zip_archive));

        if(!mz_zip_writer_init_file(&zip_archive, destinationZipFile.c_str(), 0))
        {
            return false;   // Initialization failed
        }

        // Iterate through all files in the source directory and add them to the zip
        for(const auto & entry : std::filesystem::recursive_directory_iterator(sourceDirectory))
        {
            if(entry.is_directory())
            {
                continue;   // Skip directories themselves, mz_zip_add_file will create them as needed
            }

            std::string fullPath = entry.path().string();
            std::string relativePath = Helper::createRelativePath(fullPath, sourceDirectory);

            // Add the file to the zip archive
            if(!mz_zip_writer_add_file(
                 &zip_archive, relativePath.c_str(), fullPath.c_str(), "", 0, MZ_BEST_COMPRESSION))
            {
                mz_zip_writer_end(&zip_archive);   // Clean up and exit on failure
                return false;
            }
        }

        mz_zip_writer_finalize_archive(&zip_archive);
        mz_zip_writer_end(&zip_archive);

        return true;
    }


    // Helper function to read a file into a buffer
    std::vector<char> readFileToBuffer(const std::string & filename)
    {
        std::ifstream file(filename, std::ios::binary);
        if(!file)
        {
            throw std::runtime_error("Failed to open file: " + filename);
        }
        return std::vector<char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }

    std::map<std::string, std::string> unzipFiles(std::string_view source, std::vector<std::string> const & fnames)
    {
        // Read the test.zip file into memory
        std::vector<char> zipBuffer = readFileToBuffer(std::string(source));

        mz_zip_archive zipArchive;
        memset(&zipArchive, 0, sizeof(zipArchive));

        if(!mz_zip_reader_init_mem(&zipArchive, zipBuffer.data(), zipBuffer.size(), 0))
        {
            throw std::runtime_error("Failed to initialize zip archive");
        }

        std::map<std::string, std::string> fileContents;

        size_t fileCount = mz_zip_reader_get_num_files(&zipArchive);
        for(size_t i = 0; i < fileCount; ++i)
        {
            mz_zip_archive_file_stat fileStat;
            if(!mz_zip_reader_file_stat(&zipArchive, i, &fileStat))
            {
                std::stringstream msg;
                msg << "Failed to get file stat for file index " << i;
                mz_zip_reader_end(&zipArchive);
                throw std::runtime_error(msg.str());
            }

            if (!fnames.empty() && std::find(fnames.begin(), fnames.end(), fileStat.m_filename) == fnames.end())
            {
                continue;
            }

            if(!mz_zip_reader_is_file_a_directory(&zipArchive, i))
            {
                std::vector<char> fileBuffer(fileStat.m_uncomp_size);

                if(!mz_zip_reader_extract_to_mem(&zipArchive, i, fileBuffer.data(), fileBuffer.size(), 0))
                {
                    std::stringstream msg;
                    msg << "Failed to extract file: " << fileStat.m_filename;
                    mz_zip_reader_end(&zipArchive);
                    throw std::runtime_error(msg.str());
                }
				
                auto fileStart = std::find(fileBuffer.begin(), fileBuffer.end(), '<') - fileBuffer.begin();
                auto fileEnd = fileBuffer.rend() - std::find(fileBuffer.rbegin(), fileBuffer.rend(), '>');
                fileContents[fileStat.m_filename] =
                  std::string(fileBuffer.begin() + fileStart, fileBuffer.begin() + fileEnd);
            }
            else
            {
                continue;
            }
        }

        mz_zip_reader_end(&zipArchive);

        return fileContents;
    }

    std::string unzipFile(std::string_view zipFileName, std::string_view fileName)
    {
        std::vector<std::string> fnames{std::string(fileName)};
        auto contents = unzipFiles(zipFileName, fnames);
        auto itr = contents.find(std::string(fileName));
        if (itr == contents.end())
        {
            std::stringstream msg;
            msg << "THMZ file " << zipFileName << " does not contain " << fileName; 
            throw std::runtime_error(msg.str());
        }

        return itr->second;
    }

    void unzipFiles(std::string_view sourceView, std::string_view destinationView)
    {
        unzipFiles(sourceView);
        mz_zip_archive zip_archive;
        memset(&zip_archive, 0, sizeof(zip_archive));

        std::string zipFileName(sourceView);
        std::string destination(destinationView);

        if(!mz_zip_reader_init_file(&zip_archive, zipFileName.c_str(), 0))
        {
            return;   // Initialization failed
        }

        int fileCount = (int)mz_zip_reader_get_num_files(&zip_archive);
        for(int i = 0; i < fileCount; i++)
        {
            mz_zip_archive_file_stat file_stat;
            if(!mz_zip_reader_file_stat(&zip_archive, i, &file_stat))
            {
                continue;   // Failed to get file stat, continue to next file
            }

            // Construct the full path for the file to be extracted
            std::string fullPath = destination + "/" + file_stat.m_filename;   // Use forward slash for compatibility

            // Create directory structure if it doesn't exist
            std::filesystem::path dirPath = std::filesystem::path(fullPath).parent_path();
            std::filesystem::create_directories(dirPath);

            // Extract file
            if(!mz_zip_reader_extract_to_file(&zip_archive, i, fullPath.c_str(), 0))
            {
                continue;   // Extraction failed, continue to next file
            }
        }

        mz_zip_reader_end(&zip_archive);
    }

    std::optional<std::string> getFilePathIfExists(const std::string & directory, File enumValue)
    {
        std::optional<std::string> result;

        const auto path{directory + "\\" + toString(enumValue)};

        if(std::filesystem::exists(path))
        {
            result = path;
        }

        return result;
    }

    std::optional<std::string> getFilePathIfExists(const std::string & directory, Results enumValue)
    {
        std::optional<std::string> result;

        const auto path{resultsDirectory(directory) + "\\" + toString(enumValue)};

        if(std::filesystem::exists(path))
        {
            result = path;
        }

        return result;
    }

    std::string fullPath(const std::string & directory, File enumValue)
    {
        return {directory + "\\" + toString(enumValue)};
    }

    std::string fullPath(const std::string & directory, Results enumValue)
    {
        return {resultsDirectory(directory) + "\\" + toString(enumValue)};
    }

    bool allResultsExist(const std::string & aResultsDirectory)
    {
        const std::string geometry = aResultsDirectory + "\\" + GeometryFileName;
        const std::string heatFlux = aResultsDirectory + "\\" + HeatFluxFileName;
        const std::string heatFluxEdges = aResultsDirectory + "\\" + HeatFluxEdgesFileName;
        const std::string humidity = aResultsDirectory + "\\" + HumidityFileName;
        const std::string temperature = aResultsDirectory + "\\" + TemperatureFileName;
        const std::string waterContent = aResultsDirectory + "\\" + WaterContentFileName;
        const std::string waterFlux = aResultsDirectory + "\\" + WaterFluxFileName;
        const std::string waterFluxEdges = aResultsDirectory + "\\" + WaterFluxEdgesFileName;

        using std::filesystem::exists;

        return exists(geometry) && exists(heatFlux) && exists(heatFluxEdges) && exists(humidity) && exists(temperature)
               && exists(waterContent) && exists(waterFlux) && exists(waterFluxEdges);
    }
}   // namespace ThermZip