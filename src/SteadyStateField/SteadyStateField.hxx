#pragma once

#include <map>
#include <string>
#include <vector>

#include <lbnl/expected.hxx>

#include "THMZ/Common/Enumerators.hxx"
#include "THMZ/Mesh/Mesh.hxx"

//! The nodal results of one steady-state run, on the mesh they were solved on: what THERM's
//! colour views draw.
//!
//! A solved steady-state archive keeps the mesh (Mesh) and the nodal results
//! (SteadyStateMeshResults) as separate entries, each with one case per run. The two refer
//! to a node differently: a mesh element's corners are 0-based POSITIONS in the case's node
//! list, while a node's own index - the key the results use - is a 1-based LABEL. Reading
//! one by the other gives a mesh of long triangles slicing across the model rather than an
//! error, so the join is made here, once: every per-node vector below is in mesh-node order,
//! so an element corner indexes it directly.
//!
//! A run type can appear in several cases (a CMA archive keeps one per glazing and spacer
//! variant); the first case of the requested run type is read.
namespace ThermFile::SteadyStateField
{
    struct Field
    {
        RunType runType{RunType::Unknown};
        //! The stored mesh's nodes, in order: node k is the one an element corner k names.
        std::vector<Mesh::Node> nodes;
        //! The stored mesh's elements; corners are positions in nodes, a triangle repeats one.
        std::vector<Mesh::Element> elements;
        //! One value per node, in nodes' order. NaN where the results hold nothing for the
        //! node, and - for the moisture quantities - where the run did not solve moisture.
        std::vector<double> temperature;
        std::vector<double> xFlux;
        std::vector<double> yFlux;
        std::vector<double> humidity;
        std::vector<double> waterContent;
        std::vector<double> iceContent;
    };

    //! Magnitude of the heat flux at every node.
    [[nodiscard]] std::vector<double> fluxMagnitude(const Field & field);

    //! Reads the run's field out of already-extracted archive entries, name -> content. Fails
    //! when the archive has no stored mesh or no nodal results, or neither has a case of that
    //! run type.
    [[nodiscard]] lbnl::ExpectedExt<Field, std::string> read(const std::map<std::string, std::string> & entries,
                                                             RunType runType);

    //! Extracts the archive's mesh and nodal results and reads the run's field.
    [[nodiscard]] lbnl::ExpectedExt<Field, std::string> readFromZipFile(const std::string & zipFileName,
                                                                        RunType runType);
}   // namespace ThermFile::SteadyStateField
