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

    void setGravityOrientation(const Geometry::GravityVector & vector, ModelDetails & exposure)
    {
        exposure.gravityOrientation = ThermFile::Gravity::getOrientation(vector);
    }
}   // namespace ThermFile
