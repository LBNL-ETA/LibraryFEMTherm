#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <map>
#include <vector>

namespace ThermZip
{
    const std::string ModelFileName = "Model.xml";
    const std::string GasesFileName = "Gases.xml";
    const std::string MaterialsFileName = "Materials.xml";
    const std::string SteadyStateBCFileName = "SteadyStateBC.xml";
    const std::string TransientTypeBCFileName = "TransientTypeBC.xml";

    // Transient results directory
    const std::string ResultsDir = "transient results";
    const std::string GeometryFileName = "Geometry.xml";
    const std::string HeatFluxFileName = "HeatFlux.csv";
    const std::string HeatFluxEdgesFileName = "HeatFluxEdges.csv";
    const std::string HumidityFileName = "Humidities.csv";
    const std::string TemperatureFileName = "Temperatures.csv";
    const std::string WaterContentFileName = "WaterContent.csv";
    const std::string WaterFluxFileName = "WaterFlux.csv";
    const std::string WaterFluxEdgesFileName = "WaterFluxEdges.csv";

    // Timestep boundary conditions directory
    const std::string TimestepFilesDir = "timestep input files";

    const std::string SteadyStateResultsName = "SteadyStateResults.xml";
    const std::string SteadyStateMeshResultsName = "SteadyStateMeshResults.xml";
    const std::string MeshName = "Mesh.xml";
    const std::string CMALibrary = "CMALibrary.xml";

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
    void unzipFiles(std::string_view source, std::string_view destination);

    //! Unzips files from a zip archive and returns the content of the files in a map
    //! \param source The name of the zip archive
    //! \param fnames The names of the files to extract from the zip archive
    //! \return The content of the files as a map where the key is the file name and the value is the content of the
    //! file
    std::map<std::string, std::string> unzipFiles(std::string_view source,
                                                  std::vector<std::string> const & fnames = std::vector<std::string>());

    //! Unzips a file from a zip archive and returns the content of the file in a string
    //! \param zipFileName The name of the zip archive
    //! \param fileName The name of the file to extract from the zip archive
    //! \return The content of the file as a string
    std::string unzipFile(std::string_view zipFileName, std::string_view fileName);

    //! Adds a file to a zip archive with the specified content as a string without changing the content of the zip
    //! archive if the file already exists
    //! \param zipFileName The name of the zip archive
    //! \param fileName The name of the file to add to the zip archive
    //! \param text The content of the file to add to the zip archive
    //! \return 1 if the file was successfully added to the zip archive, 0 otherwise
    int addToZipFile(std::string_view zipFileName, std::string_view fileName, std::string_view text);

    std::optional<std::string> getFilePathIfExists(const std::string & directory, File enumValue);

    std::optional<std::string> getFilePathIfExists(const std::string & directory, Results enumValue);

    std::string fullPath(const std::string & directory, File enumValue);

    std::string fullPath(const std::string & directory, Results enumValue);

    // Helper function for THERM to determine if all result files are stored and ready for the display
    bool allResultsExist(const std::string & aResultsDirectory);

}   // namespace ThermZip