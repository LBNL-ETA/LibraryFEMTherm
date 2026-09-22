#pragma once

#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <lbnl/expected.hxx>

#include "GeometryForResults/DB.hxx"

//! The node results of a transient solve, read back out of the archive.
//!
//! After a transient solve the archive carries a "transient results" folder: the solver
//! mesh as Geometry, and one CSV per quantity. The nodal CSVs (Temperatures, Humidities,
//! WaterContent, and IceContent when freezing is modelled) have the header
//! "timestep #,error tolerance,1,..,N" and one row per frame: the frame number, the
//! solve's error, then one value per node. The flux CSVs (HeatFlux, WaterFlux) have the
//! header "1-x,1-y,..,N-x,N-y" and one row per frame with no leading columns.
//!
//! Columns are the solver's node order, and the geometry's node ID is that same 1-based
//! index, so column k belongs to the node with ID k. Positions come from the geometry,
//! which stores them relative to the centre of the model's enclosing rectangle, not in the
//! drawing's coordinates.
//!
//! Archives written since the initial-conditions frame was added number their rows from
//! 0; older ones start at 1. The reader reports which it found rather than hiding it.
//!
//! Humidity is stored as a fraction; nothing here rescales anything.
namespace ThermFile::TransientResults
{
    enum class Quantity
    {
        Temperature,
        Humidity,
        WaterContent,
        IceContent
    };

    //! The archive entry base name a nodal quantity is stored under.
    [[nodiscard]] std::string entryName(Quantity quantity);

    //! One node of the solver mesh, in column order.
    struct Node
    {
        std::size_t id{0U};   //!< the geometry's ID and the CSV column, 1-based
        double x{0.0};
        double y{0.0};

        [[nodiscard]] friend bool operator==(const Node & lhs, const Node & rhs) = default;
    };

    struct FluxVector
    {
        double x{0.0};
        double y{0.0};

        [[nodiscard]] friend bool operator==(const FluxVector & lhs, const FluxVector & rhs) = default;
    };

    //! One nodal quantity over the run: values[frame][column], one solve error per frame.
    struct NodalSeries
    {
        std::vector<std::vector<double>> values;
        std::vector<double> solveError;
    };

    //! Every stored quantity of the archive's transient run.
    struct Results
    {
        //! The solver mesh as stored: nodes, elements, materials, polygons.
        GeometryLibrary::InputGeometryDataRecord geometry;
        //! Column order: nodes[k] is the node with ID k + 1, the owner of column k.
        std::vector<Node> nodes;
        //! The frame number of each row, as written: 0, 1, 2, ... or 1, 2, 3, ...
        std::vector<std::size_t> frames;
        //! True when the first row is the initial-conditions frame (frame 0).
        bool firstFrameIsInitial{false};
        //! The nodal quantities the archive carries; a missing CSV is a missing key.
        std::map<Quantity, NodalSeries> nodal;
        //! flux[frame][column]; empty when the archive carries no such entry.
        std::vector<std::vector<FluxVector>> heatFlux;
        std::vector<std::vector<FluxVector>> waterFlux;

        [[nodiscard]] std::size_t nodeCount() const;
        [[nodiscard]] std::size_t frameCount() const;
        [[nodiscard]] bool has(Quantity quantity) const;
    };

    //! Whether the entries carry a transient results folder with at least one nodal CSV.
    [[nodiscard]] bool hasTransientResults(const std::map<std::string, std::string> & entries);

    //! Reads the transient results out of already-extracted archive entries, name ->
    //! content. Fails when the geometry is missing or empty, when no nodal quantity is
    //! present, when a row's column count disagrees with the geometry, or when the
    //! quantities disagree with each other on frame numbering.
    [[nodiscard]] lbnl::ExpectedExt<Results, std::string> read(const std::map<std::string, std::string> & entries);

    //! Extracts the archive and reads its transient results.
    [[nodiscard]] lbnl::ExpectedExt<Results, std::string> readFromZipFile(const std::string & zipFileName);
}   // namespace ThermFile::TransientResults
