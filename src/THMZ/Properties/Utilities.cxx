#include <stdexcept>

#include "THMZ/Properties/Properties.hxx"
#include "Utilities.hxx"
#include "LibraryUtilities/Common.hxx"

namespace ThermFile
{
    Limits getMeshParameterLimits(const MesherType type)
    {
        static const std::map<ThermFile::MesherType, Limits> meshLimits{
          {ThermFile::MesherType::QuadTreeMesher, {3u, 12u}}, {ThermFile::MesherType::Simmetrix_Version_2022, {3u, 100u}}};

        return {meshLimits.at(type).minimum, meshLimits.at(type).maximum};
    }

    Limits getIterationLimits()
    {
        return {1u, 30000u};
    }

    ExposureType createExposureType(ThermFile::ModelPurpose model)
    {
        using CrossSectionVariant = std::variant<WindowCrossSectionType, OpaqueCrossSectionType, OtherCrossSectionType>;
        using AssemblyTypeOptional = std::optional<OpaqueAssemblyType>;

        static const std::map<ThermFile::ModelPurpose, std::pair<CrossSectionVariant, AssemblyTypeOptional>>
          exposureParamsMap = {
            {ThermFile::ModelPurpose::Window_TransparentFacade, {WindowCrossSectionType::Sill, std::nullopt}},
            {ThermFile::ModelPurpose::OpaqueFacade, {OpaqueCrossSectionType::SillPlate, OpaqueAssemblyType::Wall}},
            {ThermFile::ModelPurpose::Other, {OtherCrossSectionType::GeneralCrossSection, std::nullopt}}};

        auto it = exposureParamsMap.find(model);
        if(it == exposureParamsMap.end())
        {
            throw std::logic_error("Unhandled ModelPurpose value in createExposureType");
        }

        const auto & [crossSection, assemblyType] = it->second;
        return ExposureType{model, crossSection, assemblyType};
    }

    ModelExposure createModelExposure(ThermFile::ModelPurpose model)
    {
        ModelExposure exposure;
        exposure.exposureType = createExposureType(model);
        return exposure;
    }
}   // namespace Therm
