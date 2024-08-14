#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <map>

namespace ThermZip
{
    enum class File
    {
        Unknown,
        Gases,
        Materials,
        SteadyStateBC,
        TransientTypeBC
    };

    std::string toString(File file);
    File toFile(const std::string & file);

    enum class Results
    {
        Unknown,
        Geometry,
        HeatFlux,
        HeatFluxEdges,
        Humidities,
        Temperatures,
        WaterContent,
        WaterFlux,
        WaterFluxEdges
    };

    std::string toString(Results results);
    Results toResults(const std::string & results);

    std::string resultsDirectory(const std::string & directory);
    std::string timestepFilesDirectory(const std::string & directory);

    std::string modelFileName(const std::string & directory);
    std::string steadyStateResultsName(const std::string & directory);
    std::string steadyStateMeshResultsName(const std::string & directory);
    std::string meshName(const std::string & directory);

    bool zipFiles(const std::string & sourceDirectory, const std::string & destinationZipFile);
    bool zipFiles(std::map<std::string, std::string> const & data, const std::string & destinationZipFile);
    void unzipFiles(std::string_view source, std::string_view destination);
    std::map<std::string, std::string> unzipFiles(std::string_view source,
                                                  std::vector<std::string> const & fnames = std::vector<std::string>());
    std::string unzipFile(std::string_view zipFileName, std::string_view fileName);
    
    std::optional<std::string> getFilePathIfExists(const std::string & directory,
                                                                File enumValue);

    std::optional<std::string> getFilePathIfExists(const std::string & directory,
                                                                Results enumValue);
    
    std::string fullPath(const std::string & directory, File enumValue);

    std::string fullPath(const std::string & directory, Results enumValue);

    // Helper function for THERM to determine if all result files are stored and ready for the display
    bool allResultsExist(const std::string & aResultsDirectory);

}   // namespace ThermZip