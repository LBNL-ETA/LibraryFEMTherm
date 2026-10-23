#include <string>

#include <lbnl/algorithm.hxx>
#include <lbnl/optional.hxx>

#include "BoundaryLookup.hxx"

namespace ThermFile
{
    std::optional<BCLibrary::BoundaryCondition> boundaryRecord(const BCLibrary::DB & library,
                                                               const Boundary & boundary)
    {
        return lbnl::extend(boundary.bcUUID)
          .and_then([&library](const std::string & uuid) { return library.getByUUID(uuid); })
          .or_else([&library, &boundary] { return library.getByName(boundary.name); })
          .raw();
    }

    std::optional<TimeSeriesLibrary::TimeSeriesData>
      boundaryDataset(const std::vector<TimeSeriesLibrary::TimeSeriesData> & datasets, const Boundary & boundary)
    {
        return lbnl::extend(boundary.timeSeriesUUID)
          .and_then(
            [&datasets](const std::string & uuid)
            {
                return lbnl::find_element(datasets, [&uuid](const auto & dataset) { return dataset.UUID == uuid; });
            })
          .raw();
    }
}   // namespace ThermFile
