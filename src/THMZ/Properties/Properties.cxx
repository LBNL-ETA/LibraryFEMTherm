#include <stdexcept>

#include "Properties.hxx"

#include "LibraryUtilities/Common.hxx"

namespace ThermFile
{
    void General::setModifiedIsEqualToCreate()
    {
        lastModified = creationDate;
        lastModifiedVersion = creationVersion;
    }

    void setGravityOrientation(const Geometry::GravityVector & vector, ModelExposure & exposure)
    {
        exposure.gravityOrientation = ThermFile::Gravity::getOrientation(vector);
    }
}   // namespace ThermFile
