#pragma once

#include <vector>
#include <optional>

#include "THMZ/Common/Enumerators.hxx"
#include "THMZ/CMA/Enumerators.hxx"

namespace ThermFile::Mesh
{
    struct Node
    {
        size_t index{0u};
        double x{0.0};
        double y{0.0};
    };

    struct Element
    {
        size_t index{0u};
        size_t node1{0u};
        size_t node2{0u};
        size_t node3{0u};
        size_t node4{0u};
        std::optional<size_t> materialID{0u};
    };

    namespace Segment
    {
        struct Flux
        {
            size_t index{0u};
            size_t node1{0u};
            size_t node2{0u};
            double flux1{0.0};
            double flux2{0.0};
            size_t label{0u};
        };

        struct Temperature
        {
            size_t node{0u};
            double temperature{0.0};
            size_t label{0u};
        };

        struct Convection
        {
            size_t index{0u};
            size_t node1{0u};
            size_t node2{0u};
            double filmCoefficient{0.0};
            double temperature1{0.0};
            double temperature2{0.0};
            size_t label{0u};
        };

        struct Radiation
        {
            size_t index{0u};
            size_t node1{0u};
            size_t node2{0u};
            double filmCoefficient{0.0};
            double temperature1{0.0};
            double temperature2{0.0};
        };

        struct RadiationEnclosure
        {
            size_t index{0u};
            size_t node1{0u};
            size_t node2{0u};
            size_t condCode{0u};
            double emissivity{0.0};
            double temperature{0.0};
        };
    }   // namespace Segment

    struct BCSegments
    {
        std::optional<std::vector<Segment::Flux>> fluxes;
        std::optional<std::vector<Segment::Temperature>> temperatures;
        std::optional<std::vector<Segment::Convection>> convections;
        std::optional<std::vector<Segment::Radiation>> radiations;
        std::optional<std::vector<Segment::RadiationEnclosure>> radiationEnclosures;
    };

    struct Case
    {
        std::optional<RunType> modelType;
        std::optional<CMACase> glazingCase;
        std::optional<CMACase> spacerCase;
        std::vector<Node> nodes;
        std::vector<Element> elements;
        BCSegments bcSegments;
    };

    struct Mesh
    {
        std::string version{"1"};
        std::vector<Case> cases;
    };
}   // namespace ThermFile::Mesh