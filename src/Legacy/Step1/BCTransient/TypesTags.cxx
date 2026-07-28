#include "TypesTags.hxx"

#include "LibraryUtilities/Common.hxx"

namespace BCTypesLibrary
{

    std::string StringTags::stringBCType(const BCType bc) const
    {
        return LibraryCommon::enumToString(bc, bcType);
    }

    std::vector<std::string> StringTags::stringsBCType() const
    {
        return LibraryCommon::stringVector(bcType);
    }

    std::vector<std::string> StringTags::stringsBCModel() const
    {
        return LibraryCommon::stringVector(bcModel);
    }

    std::vector<std::string> StringTags::stringsDirection() const
    {
        return LibraryCommon::stringVector(direction);
    }

    std::vector<std::string> StringTags::stringsConvectionModel() const
    {
        return LibraryCommon::stringVector(convectionModel);
    }

    std::vector<std::string> StringTags::stringsRadiationModel() const
    {
        return LibraryCommon::stringVector(radiationModel);
    }

    std::string bcTypeToString(BCType bc)
    {
        StringTags tags;
        return LibraryCommon::enumToString(bc, tags.bcType);
    }

    BCType bcTypeFromString(std::string_view value)
    {
        StringTags tags;
        return LibraryCommon::enumFromString(value, tags.bcType);
    }

    std::string bcModelToString(BCModel bcModel)
    {
        StringTags tags;
        return LibraryCommon::enumToString(bcModel, tags.bcModel);
    }

    BCModel bcModelFromString(std::string_view value)
    {
        StringTags tags;
        return LibraryCommon::enumFromString(value, tags.bcModel);
    }

    std::string directionToString(Direction direction)
    {
        StringTags tags;
        return LibraryCommon::enumToString(direction, tags.direction);
    }

    Direction directionFromString(std::string_view value)
    {
        StringTags tags;
        return LibraryCommon::enumFromString(value, tags.direction);
    }

    std::string convectionModelToString(ConvectionModel convectionModel)
    {
        StringTags tags;
        return LibraryCommon::enumToString(convectionModel, tags.convectionModel);
    }

    ConvectionModel convectionModelFromString(std::string_view value)
    {
        StringTags tags;
        return LibraryCommon::enumFromString(value, tags.convectionModel);
    }

    std::string radiationModelToString(RadiationModel radiationModel)
    {
        StringTags tags;
        return LibraryCommon::enumToString(radiationModel, tags.radiationModel);
    }

    RadiationModel radiationModelFromString(std::string_view value)
    {
        StringTags tags;
        return LibraryCommon::enumFromString(value, tags.radiationModel);
    }
}
