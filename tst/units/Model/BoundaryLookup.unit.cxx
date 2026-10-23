#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <vector>

#include "Model/BoundaryLookup.hxx"

using BCLibrary::BoundaryCondition;
using ThermFile::Boundary;
using TimeSeriesLibrary::Series;
using TimeSeriesLibrary::SeriesRole;
using TimeSeriesLibrary::TimeSeriesData;

namespace
{
    BoundaryCondition adiabatic(const std::string & uuid, const std::string & name)
    {
        return BoundaryCondition{.UUID = uuid, .Name = name, .data = BCLibrary::NoExchange{}};
    }

    //! "outside" and "room side", each under its own UUID.
    BCLibrary::DB twoRecords()
    {
        BCLibrary::DB library;
        library.add(adiabatic("uuid-outside", "outside"));
        library.add(adiabatic("uuid-room", "room side"));
        return library;
    }

    TimeSeriesData dataset(const std::string & uuid, const std::string & name, const std::vector<SeriesRole> & roles)
    {
        TimeSeriesData data{.UUID = uuid, .Name = name};
        for(const auto role : roles)
        {
            data.series.push_back(Series{.role = role, .values = {1.0, 2.0}});
        }
        return data;
    }

    std::vector<TimeSeriesData> twoDatasets()
    {
        return {dataset("uuid-weather", "weather", {SeriesRole::AirTemperature}),
                dataset("uuid-room", "room", {SeriesRole::RelativeHumidity})};
    }
}   // namespace

TEST(BoundaryLookup, RecordByUuidWinsOverName)
{
    const auto record{ThermFile::boundaryRecord(twoRecords(), Boundary{.name = "outside", .bcUUID = "uuid-room"})};

    ASSERT_TRUE(record.has_value());
    EXPECT_EQ(record->Name, "room side");
}

TEST(BoundaryLookup, RecordByNameWithoutUuid)
{
    // A pre-consolidation archive binds by the boundary's name.
    const auto record{ThermFile::boundaryRecord(twoRecords(), Boundary{.name = "outside"})};

    ASSERT_TRUE(record.has_value());
    EXPECT_EQ(record->UUID, "uuid-outside");
}

TEST(BoundaryLookup, RecordByNameWhenUuidUnknown)
{
    const auto record{ThermFile::boundaryRecord(twoRecords(), Boundary{.name = "outside", .bcUUID = "uuid-gone"})};

    ASSERT_TRUE(record.has_value());
    EXPECT_EQ(record->UUID, "uuid-outside");
}

TEST(BoundaryLookup, NoRecordWhenLibraryHoldsNeither)
{
    EXPECT_FALSE(ThermFile::boundaryRecord(twoRecords(), Boundary{.name = "attic", .bcUUID = "uuid-gone"}).has_value());
}

TEST(BoundaryLookup, DatasetByUuid)
{
    const auto found{ThermFile::boundaryDataset(twoDatasets(), Boundary{.timeSeriesUUID = "uuid-room"})};

    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->Name, "room");
}

TEST(BoundaryLookup, NoDatasetWithoutUuid)
{
    EXPECT_FALSE(ThermFile::boundaryDataset(twoDatasets(), Boundary{.name = "weather"}).has_value());
}

TEST(BoundaryLookup, NoDatasetWhenUuidUnknown)
{
    EXPECT_FALSE(ThermFile::boundaryDataset(twoDatasets(), Boundary{.timeSeriesUUID = "uuid-gone"}).has_value());
}

TEST(BoundaryLookup, PairingCheckOnALookedUpPair)
{
    BCLibrary::Convection convection;
    convection.airTemperature = BCLibrary::FromTimeSeries{SeriesRole::AirTemperature};
    convection.filmCoefficient = BCLibrary::Constant{8.0};
    BCLibrary::DB library;
    library.add(BoundaryCondition{.UUID = "uuid-reads-air",
                                  .Name = "reads air",
                                  .data = BCLibrary::SurfaceExchange{.relativeHumidity = BCLibrary::Constant{0.5},
                                                                     .convection = convection}});
    const Boundary boundary{.name = "reads air", .bcUUID = "uuid-reads-air", .timeSeriesUUID = "uuid-room"};

    const auto record{ThermFile::boundaryRecord(library, boundary)};
    ASSERT_TRUE(record.has_value());

    // The room dataset carries humidity only, so the air temperature the record reads is missing.
    EXPECT_EQ(BCLibrary::missingRoles(record.value(), ThermFile::boundaryDataset(twoDatasets(), boundary)),
              (std::vector<SeriesRole>{SeriesRole::AirTemperature}));
}
