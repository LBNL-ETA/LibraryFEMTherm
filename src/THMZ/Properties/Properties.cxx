#include <stdexcept>

#include "Properties.hxx"

#include "LibraryUtilities/Common.hxx"

namespace ThermFile
{
    //! Section name in ini file that all properties will be read from and saved to.
    const std::string FilePropertiesSectionName{"File Properties"};

    ////////////////////////////////////////////////////////////////////////////////////
    /// General
    ////////////////////////////////////////////////////////////////////////////////////

    void General::setModifiedIsEqualToCreate()
    {
        lastModified = creationDate;
        lastModifiedVersion = creationVersion;
    }

    ExposureType::ExposureType(
      ModelPurpose modelPurpose,
      std::variant<WindowCrossSectionType, OpaqueCrossSectionType, OtherCrossSectionType> crossSection,
      std::optional<OpaqueAssemblyType> assemblyType) :
        modelPurpose(modelPurpose), crossSection(crossSection), assemblyType(assemblyType)
    {}

    ////////////////////////////////////////////////////////////////////////////////////
    /// ModelExposure
    ////////////////////////////////////////////////////////////////////////////////////

    void setGravityOrientation(const Geometry::GravityVector & vector, ModelExposure & exposure)
    {
        exposure.gravityOrientation = ThermFile::Gravity::getOrientation(vector);
    }
}   // namespace Therm
