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

    const std::filesystem::path ResultsDirPath = "transient results";

    const std::filesystem::path GeometryFilePath = ResultsDirPath / "Geometry.xml";
    const std::filesystem::path HeatFluxFilePath = ResultsDirPath / "HeatFlux.csv";
    const std::filesystem::path HeatFluxEdgesFilePath = ResultsDirPath / "HeatFluxEdges.csv";
    const std::filesystem::path HumidityFilePath = ResultsDirPath / "Humidities.csv";
    const std::filesystem::path TemperatureFilePath = ResultsDirPath / "Temperatures.csv";
    const std::filesystem::path WaterContentFilePath = ResultsDirPath / "WaterContent.csv";
    const std::filesystem::path WaterFluxFilePath = ResultsDirPath / "WaterFlux.csv";
    const std::filesystem::path WaterFluxEdgesFilePath = ResultsDirPath / "WaterFluxEdges.csv";

    inline std::string getGeometryFileName() { return GeometryFilePath.string(); }
    inline std::string getHeatFluxFileName() { return HeatFluxFilePath.string(); }
    inline std::string getHeatFluxEdgesFileName() { return HeatFluxEdgesFilePath.string(); }
    inline std::string getHumidityFileName() { return HumidityFilePath.string(); }
    inline std::string getTemperatureFileName() { return TemperatureFilePath.string(); }
    inline std::string getWaterContentFileName() { return WaterContentFilePath.string(); }
    inline std::string getWaterFluxFileName() { return WaterFluxFilePath.string(); }
    inline std::string getWaterFluxEdgesFileName() { return WaterFluxEdgesFilePath.string(); }

    // Timestep boundary conditions directory
    const std::filesystem::path TimestepFilesDir = "timestep input files";

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

    bool zipFiles(const std::string & sourceDirectory, const std::string & destinationZipFile);
    void unzipFiles(std::string_view source, std::string_view destination);

    //! Simple helper function to add a timestep directory to a file name. If file name is "file.txt"
    //! and timestep directory is "timestep_1", the result will be "timestep_1/file.txt"
    //! For now timestep directory is hardcoded and the value is "timestep input files"
    std::string addTimestepDirectoryToFileName(const std::string & fileName);

    //! Zips files into a zip archive
    //! \param fileContents The content of the files to zip where the key is the file name and the value is the content of
    //! the file
    //! \param zipFileName The name of the zip archive
    void zipFiles(const std::map<std::string, std::string> & fileContents, const std::string & zipFileName);

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

}   // namespace ThermZip