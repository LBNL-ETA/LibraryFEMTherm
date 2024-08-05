#pragma once

#include <fileParse/Enum.hxx>

#include "THMZ/Properties/Properties.hxx"

namespace ThermFile
{
    namespace Gravity
    {
        template<typename NodeAdapter>
        inline const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::Gravity::Orientation & orientation)
        {
            return FileParse::deserializeEnum<NodeAdapter, ThermFile::Gravity::Orientation>(
              node, orientation, ThermFile::Gravity::orientationFromString);
        }

        template<typename NodeAdapter>
        inline NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::Gravity::Orientation & orientation)
        {
            return FileParse::serializeEnum<NodeAdapter, ThermFile::Gravity::Orientation>(
              node, orientation, ThermFile::Gravity::orientationToString);
        }
    }   // namespace Gravity

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::ModelPurpose & purpose)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::ModelPurpose>(
          node, purpose, ThermFile::modelPurposeFromString);
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::ModelPurpose & purpose)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::ModelPurpose>(
          node, purpose, ThermFile::modelPurposeToString);
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::SimulationEngine & engine)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::SimulationEngine>(
          node, engine, ThermFile::simulationEngineFromString);
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::SimulationEngine & engine)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::SimulationEngine>(
          node, engine, ThermFile::simulationEngineToString);
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::CalculationMode & mode)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::CalculationMode>(
          node, mode, ThermFile::calculationModeFromString);
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::CalculationMode & mode)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::CalculationMode>(
          node, mode, ThermFile::calculationModeToString);
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node,
                                          ThermFile::TransientCalculationMethodology & methodology)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::TransientCalculationMethodology>(
          node, methodology, ThermFile::transientCalculationMethodologyFromString);
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::TransientCalculationMethodology & methodology)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::TransientCalculationMethodology>(
          node, methodology, ThermFile::transientCalculationMethodologyToString);
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::MesherType & mesherType)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::MesherType>(
          node, mesherType, ThermFile::mesherTypeFromString);
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::MesherType & mesherType)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::MesherType>(
          node, mesherType, ThermFile::mesherTypeToString);
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node,
                                          ThermFile::SteadyStateCalculationMethodology & methodology)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::SteadyStateCalculationMethodology>(
          node, methodology, ThermFile::steadyStateCalculationMethodologyFromString);
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node,
                                    const ThermFile::SteadyStateCalculationMethodology & methodology)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::SteadyStateCalculationMethodology>(
          node, methodology, ThermFile::steadyStateCalculationMethodologyToString);
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::Orientation & orientation)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::Orientation>(
          node, orientation, ThermFile::orientationFromString);
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::Orientation & orientation)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::Orientation>(
          node, orientation, ThermFile::orientationToString);
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node,
                                          ThermFile::WindowCrossSectionType & windowCrossSectionType)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::WindowCrossSectionType>(
          node, windowCrossSectionType, ThermFile::windowCrossSectionTypeFromString);
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node,
                                    const ThermFile::WindowCrossSectionType & windowCrossSectionType)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::WindowCrossSectionType>(
          node, windowCrossSectionType, ThermFile::windowCrossSectionTypeToString);
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node, ThermFile::OpaqueAssemblyType & opaqueAssemblyType)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::OpaqueAssemblyType>(
          node, opaqueAssemblyType, ThermFile::opaqueAssemblyTypeFromString);
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::OpaqueAssemblyType & opaqueAssemblyType)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::OpaqueAssemblyType>(
          node, opaqueAssemblyType, ThermFile::opaqueAssemblyTypeToString);
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node,
                                          ThermFile::OpaqueCrossSectionType & opaqueCrossSection)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::OpaqueCrossSectionType>(
          node, opaqueCrossSection, ThermFile::opaqueCrossSectionFromString);
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::OpaqueCrossSectionType & opaqueCrossSection)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::OpaqueCrossSectionType>(
          node, opaqueCrossSection, ThermFile::opaqueCrossSectionToString);
    }

    template<typename NodeAdapter>
    inline const NodeAdapter & operator>>(const NodeAdapter & node,
                                          ThermFile::OtherCrossSectionType & otherCrossSectionType)
    {
        return FileParse::deserializeEnum<NodeAdapter, ThermFile::OtherCrossSectionType>(
          node, otherCrossSectionType, ThermFile::otherCrossSectionTypeFromString);
    }

    template<typename NodeAdapter>
    inline NodeAdapter & operator<<(NodeAdapter & node, const ThermFile::OtherCrossSectionType & otherCrossSectionType)
    {
        return FileParse::serializeEnum<NodeAdapter, ThermFile::OtherCrossSectionType>(
          node, otherCrossSectionType, ThermFile::otherCrossSectionTypeToString);
    }
}   // namespace ThermFile