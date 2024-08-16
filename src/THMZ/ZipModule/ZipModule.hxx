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