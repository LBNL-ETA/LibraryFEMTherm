#include "ZipModule.hxx"

#include <map>

#include <miniz.h>
#include <sstream>
#include <iterator>
#include <fstream>
#include <algorithm>

namespace ThermZip
{
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

    void zipFiles(const std::map<std::string, std::string> & fileContents, const std::string & zipFileName)
    {
        mz_zip_archive zipArchive;
        memset(&zipArchive, 0, sizeof(zipArchive));

        // Initialize the zip archive for writing
        if(!mz_zip_writer_init_file(&zipArchive, zipFileName.c_str(), 0))
        {
            throw std::runtime_error("Failed to initialize zip archive for writing");
        }

        // Iterate over each file in the map
        for(const auto & [filePath, content] : fileContents)
        {
            // Add the file content to the zip archive
            if(!mz_zip_writer_add_mem(&zipArchive, filePath.c_str(), content.data(), content.size(), MZ_DEFAULT_LEVEL))
            {
                mz_zip_writer_end(&zipArchive);
                std::stringstream msg;
                msg << "Failed to add file to zip: " << filePath;
                throw std::runtime_error(msg.str());
            }
        }

        // Finalize and close the zip archive
        if(!mz_zip_writer_finalize_archive(&zipArchive))
        {
            mz_zip_writer_end(&zipArchive);
            throw std::runtime_error("Failed to finalize zip archive");
        }

        mz_zip_writer_end(&zipArchive);
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
        std::vector<char> zipBuffer = readFileToBuffer(std::string(source));

        mz_zip_archive zipArchive;
        memset(&zipArchive, 0, sizeof(zipArchive));

        if(!mz_zip_reader_init_mem(&zipArchive, zipBuffer.data(), zipBuffer.size(), 0))
        {
            throw std::runtime_error("Failed to initialize zip archive");
        }

        std::map<std::string, std::string> fileContents;

        size_t fileCount = mz_zip_reader_get_num_files(&zipArchive);
        for(mz_uint i = 0; i < fileCount; ++i)
        {
            mz_zip_archive_file_stat fileStat;
            if(!mz_zip_reader_file_stat(&zipArchive, i, &fileStat))
            {
                std::stringstream msg;
                msg << "Failed to get file stat for file index " << i;
                mz_zip_reader_end(&zipArchive);
                throw std::runtime_error(msg.str());
            }

            if(!fnames.empty() && std::find(fnames.begin(), fnames.end(), fileStat.m_filename) == fnames.end())
            {
                continue;
            }

            if(!mz_zip_reader_is_file_a_directory(&zipArchive, i))
            {
                std::vector<char> fileBuffer(fileStat.m_uncomp_size + 1, 0);   // +1 to ensure null-termination

                if(!mz_zip_reader_extract_to_mem(&zipArchive, i, fileBuffer.data(), fileStat.m_uncomp_size, 0))
                {
                    std::stringstream msg;
                    msg << "Failed to extract file: " << fileStat.m_filename;
                    mz_zip_reader_end(&zipArchive);
                    throw std::runtime_error(msg.str());
                }

                // Create a string, ensuring only valid data is included
                std::string extractedContent(fileBuffer.data(), fileStat.m_uncomp_size);

                // Optional: Trim any unwanted characters at the end
                size_t end = extractedContent.find_last_not_of("\0\xFF\xFE\xFD");
                if(end != std::string::npos)
                {
                    extractedContent = extractedContent.substr(0, end + 1);
                }

                fileContents[fileStat.m_filename] = extractedContent;
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
        if(itr == contents.end())
        {
            std::stringstream msg;
            msg << "THMZ file " << zipFileName << " does not contain " << fileName;
            throw std::runtime_error(msg.str());
        }

        return itr->second;
    }

    int addToZipFile(std::string_view zipFileName, std::string_view fileName, std::string_view text)
    {
        // Step 1: Extract existing files
        std::vector<std::string> fileNames;   // Empty vector to extract all files

        std::map<std::string, std::string> existingFiles = std::filesystem::exists(zipFileName)
                                                             ? unzipFiles(zipFileName, fileNames)
                                                             : std::map<std::string, std::string>();

        // Step 2: Add or replace the target file in the map
        existingFiles[std::string(fileName)] = std::string(text);

        // Step 3: Recreate the zip archive
        mz_zip_archive zipArchive;
        memset(&zipArchive, 0, sizeof(zipArchive));

        // Initialize the writer for a new zip file
        if(!mz_zip_writer_init_file(&zipArchive, std::string(zipFileName).c_str(), 0))
        {
            throw std::runtime_error("Failed to initialize zip archive for writing");
        }

        // Add all files back to the archive
        for(const auto & [name, content] : existingFiles)
        {
            if(!mz_zip_writer_add_mem(&zipArchive, name.c_str(), content.data(), content.size(), 0))
            {
                mz_zip_writer_end(&zipArchive);
                throw std::runtime_error("Failed to add file to zip archive");
            }
        }

        // Finalize and close the zip archive
        mz_zip_writer_finalize_archive(&zipArchive);
        mz_zip_writer_end(&zipArchive);

        return 1;   // Return success
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

    std::string addTimestepDirectoryToFileName(const std::string & fileName)
    {
        return (TimestepFilesDir / fileName).string();
    }
}   // namespace ThermZip