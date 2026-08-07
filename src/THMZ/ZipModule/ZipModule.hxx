#pragma once

#include <optional>
#include <string>
#include <map>
#include <vector>

#include <fileParse/FileFormat.hxx>

namespace ThermZip
{
    // Archive entry names are extension-free bases: the name is the entry's identity,
    // the extension is the serialization format, appended by entryNameForFormat.
    const std::string ModelFileName = "Model";
    const std::string GasesFileName = "Gases";
    const std::string MaterialsFileName = "Materials";
    const std::string SteadyStateBCFileName = "SteadyStateBC";
    const std::string TransientTypeBCFileName = "TransientTypeBC";

    // Transient results directory

    const std::string ResultsDirPath = "transient results";

    const std::string GeometryFilePath = ResultsDirPath + "/Geometry";
    const std::string HeatFluxFilePath = ResultsDirPath + "/HeatFlux.csv";
    const std::string HeatFluxEdgesFilePath = ResultsDirPath + "/HeatFluxEdges.csv";
    const std::string HumidityFilePath = ResultsDirPath + "/Humidities.csv";
    const std::string TemperatureFilePath = ResultsDirPath + "/Temperatures.csv";
    const std::string WaterContentFilePath = ResultsDirPath + "/WaterContent.csv";
    const std::string WaterFluxFilePath = ResultsDirPath + "/WaterFlux.csv";
    const std::string WaterFluxEdgesFilePath = ResultsDirPath + "/WaterFluxEdges.csv";

    inline std::string getGeometryFileName() { return GeometryFilePath; }
    inline std::string getHeatFluxFileName() { return HeatFluxFilePath; }
    inline std::string getHeatFluxEdgesFileName() { return HeatFluxEdgesFilePath; }
    inline std::string getHumidityFileName() { return HumidityFilePath; }
    inline std::string getTemperatureFileName() { return TemperatureFilePath; }
    inline std::string getWaterContentFileName() { return WaterContentFilePath; }
    inline std::string getWaterFluxFileName() { return WaterFluxFilePath; }
    inline std::string getWaterFluxEdgesFileName() { return WaterFluxEdgesFilePath; }

    // Timestep boundary conditions directory
    const std::string TimestepFilesDir = "timestep input files";

    // Unified boundary condition library and time series datasets (BC consolidation)
    const std::string BoundaryConditionsFileName = "BoundaryConditions";
    const std::string TimeSeriesDir = "time series";

    //! Archive entry base name for one time series dataset ("time series/<uuid>"), with
    //! the format's extension appended. Content-hash UUIDs make the name stable for
    //! identical data, so re-embedding the same dataset never duplicates an entry.
    std::string timeSeriesEntryName(const std::string & datasetUUID,
                                    FileParse::FileFormat format = FileParse::FileFormat::XML);

    //! Appends the format's extension to an extension-free entry base name
    //! ("Model" -> "Model.xml" or "Model.json").
    std::string entryNameForFormat(const std::string & baseName, FileParse::FileFormat format);

    //! Both format spellings of an entry base name, JSON variant first. Readers probe
    //! these in order so a re-saved archive wins over a stale counterpart entry.
    std::vector<std::string> entryNameCandidates(const std::string & baseName);

    //! Looks up an entry base name in an extracted-archive map under both format
    //! spellings; empty when the archive carries no such entry.
    std::string findEntry(const std::map<std::string, std::string> & entries, const std::string & baseName);

    const std::string SteadyStateResultsName = "SteadyStateResults";
    const std::string SteadyStateMeshResultsName = "SteadyStateMeshResults";
    const std::string MeshName = "Mesh";
    const std::string CMALibrary = "CMALibrary";

    bool zipFiles(const std::string & sourceDirectory, const std::string & destinationZipFile);
    void unzipFiles(std::string_view source, std::string_view destination);

    //! Simple helper function to add a timestep directory to a file name. If file name is "file.txt"
    //! and timestep directory is "timestep_1", the result will be "timestep_1/file.txt"
    //! For now timestep directory is hardcoded and the value is "timestep input files"
    std::string addTimestepDirectoryToFileName(const std::string & fileName);

    //! Deflate level used when none is given: miniz's default (6).
    constexpr int DefaultCompressionLevel{6};

    //! Level for large model archives. THMZ payloads are highly repetitive, so the deflate
    //! curve flattens early: measured on a 41 MB mesh entry, level 2 compresses in 166 ms to
    //! 2.68 MB where level 6 needs 363 ms to reach 2.04 MB. Less than a megabyte of file for
    //! less than half the time is the right trade for a section that grows with node count.
    constexpr int FastCompressionLevel{2};

    //! Zips files into a zip archive
    //! \param fileContents The content of the files to zip where the key is the file name and the value is the content of
    //! the file
    //! \param zipFileName The name of the zip archive
    //! \param compressionLevel Deflate level 0 (store) to 9 (smallest)
    void zipFiles(const std::map<std::string, std::string> & fileContents,
                  const std::string & zipFileName,
                  int compressionLevel = DefaultCompressionLevel);

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
    //! \param obsoleteNames Entries to drop while rewriting the archive; used to remove the
    //! other-format spelling of fileName so a format switch never leaves two model entries behind
    //! \return 1 if the file was successfully added to the zip archive, 0 otherwise
    int addToZipFile(std::string_view zipFileName,
                     std::string_view fileName,
                     std::string_view text,
                     const std::vector<std::string> & obsoleteNames = {});

}   // namespace ThermZip