#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include <filesystem>
#include <format>
#include <optional>
#include <string>
#include <vector>

#include <lbnl/expected.hxx>

#include "TimeSeriesData/ContentHash.hxx"
#include "TimeSeriesData/CsvSeries.hxx"
#include "TimeSeriesData/DB.hxx"
#include "TimeSeriesData/Epw.hxx"
#include "TimeSeriesData/Import.hxx"
#include "TimeSeriesData/Standards.hxx"
#include "TimeSeriesData/Tags.hxx"
#include "TimeSeriesData/TimeSeriesData.hxx"
#include "TimeSeriesData/Window.hxx"

namespace py = pybind11;

using TimeSeriesLibrary::Series;
using TimeSeriesLibrary::SeriesRole;
using TimeSeriesLibrary::TimeAxis;
using TimeSeriesLibrary::TimeSeriesData;

namespace
{
    //! A failed import or window is a ValueError carrying the library's message.
    template<typename Value>
    Value unwrap(lbnl::ExpectedExt<Value, std::string> result)
    {
        if(!result.has_value())
        {
            throw py::value_error(result.error());
        }
        return result.value();
    }

    void bindDataModel(py::module_ & mod)
    {
        py::enum_<SeriesRole>(mod, "SeriesRole")
            .value("AirTemperature", SeriesRole::AirTemperature)
            .value("RelativeHumidity", SeriesRole::RelativeHumidity)
            .value("WindSpeed", SeriesRole::WindSpeed)
            .value("WindDirection", SeriesRole::WindDirection)
            .value("SolarIrradiance", SeriesRole::SolarIrradiance)
            .value("HeatFlux", SeriesRole::HeatFlux)
            .value("ConvectiveCoefficient", SeriesRole::ConvectiveCoefficient)
            .value("RadiantTemperature", SeriesRole::RadiantTemperature)
            .value("RadiativeCoefficient", SeriesRole::RadiativeCoefficient)
            .value("Emissivity", SeriesRole::Emissivity)
            .value("PrescribedTemperature", SeriesRole::PrescribedTemperature)
            .value("PrescribedHumidity", SeriesRole::PrescribedHumidity);

        py::class_<Series>(mod, "Series", "One role plus its values, one value per timestep.")
            .def(py::init<>())
            .def(py::init<SeriesRole, std::vector<double>>(), py::arg("role"), py::arg("values"))
            .def_readwrite("role", &Series::role)
            .def_readwrite("values", &Series::values)
            .def("__eq__", [](const Series & lhs, const Series & rhs) { return lhs == rhs; }, py::is_operator())
            .def("__repr__", [](const Series & series) {
                return std::format("Series(role={}, values=<{} values>)",
                                   TimeSeriesLibrary::seriesRoleToString(series.role),
                                   series.values.size());
            });

        py::class_<TimeAxis>(mod,
                             "TimeAxis",
                             "Where row zero sits in the year and how far apart the rows are: a calendar "
                             "position with no year, and a step in seconds.")
            .def(py::init([](const std::size_t month, const std::size_t day, const std::size_t hour, const std::size_t minute, const double stepSeconds) {
                     return TimeAxis{.month = month, .day = day, .hour = hour, .minute = minute, .stepSeconds = stepSeconds};
                 }),
                 py::arg("month") = 1U,
                 py::arg("day") = 1U,
                 py::arg("hour") = 0U,
                 py::arg("minute") = 0U,
                 py::arg("step_seconds") = 3600.0)
            .def_readwrite("month", &TimeAxis::month)
            .def_readwrite("day", &TimeAxis::day)
            .def_readwrite("hour", &TimeAxis::hour)
            .def_readwrite("minute", &TimeAxis::minute)
            .def_readwrite("step_seconds", &TimeAxis::stepSeconds)
            .def("__eq__", [](const TimeAxis & lhs, const TimeAxis & rhs) { return lhs == rhs; }, py::is_operator())
            .def("__repr__", [](const TimeAxis & axis) {
                return std::format("TimeAxis(month={}, day={}, hour={}, minute={}, step_seconds={})",
                                   axis.month,
                                   axis.day,
                                   axis.hour,
                                   axis.minute,
                                   py::repr(py::cast(axis.stepSeconds)).cast<std::string>());
            });

        py::class_<TimeSeriesData>(mod,
                                   "TimeSeriesData",
                                   "One dataset: identity, time axis and its series.\n\n"
                                   "`series` is copied out as a Python list: build the list and "
                                   "assign it (`data.series = [...]`); appending to the returned "
                                   "list does not change the dataset.")
            .def(py::init([](std::string name,
                             std::vector<Series> series,
                             const TimeAxis & axis,
                             std::string uuid,
                             std::optional<std::string> projectName,
                             std::optional<std::string> description,
                             std::string color,
                             std::optional<std::string> source,
                             const bool isProtected) {
                     return TimeSeriesData{.UUID = std::move(uuid),
                                           .Name = std::move(name),
                                           .ProjectName = std::move(projectName),
                                           .Protected = isProtected,
                                           .Description = std::move(description),
                                           .Color = std::move(color),
                                           .Source = std::move(source),
                                           .axis = axis,
                                           .series = std::move(series)};
                 }),
                 py::arg("name") = std::string{"Default Name"},
                 py::arg("series") = std::vector<Series>{},
                 py::arg("axis") = TimeAxis{},
                 py::arg("uuid") = std::string{},
                 py::arg("project_name") = py::none(),
                 py::arg("description") = py::none(),
                 py::arg("color") = std::string{"0x1F6FB4"},
                 py::arg("source") = py::none(),
                 py::arg("protected_") = false)
            .def_readwrite("uuid", &TimeSeriesData::UUID)
            .def_readwrite("name", &TimeSeriesData::Name)
            .def_readwrite("project_name", &TimeSeriesData::ProjectName)
            .def_readwrite("protected_", &TimeSeriesData::Protected)
            .def_readwrite("description", &TimeSeriesData::Description)
            .def_readwrite("color", &TimeSeriesData::Color)
            .def_readwrite("source", &TimeSeriesData::Source)
            .def_readwrite("axis", &TimeSeriesData::axis)
            .def_readwrite("series", &TimeSeriesData::series)
            .def("__eq__", [](const TimeSeriesData & lhs, const TimeSeriesData & rhs) { return lhs == rhs; }, py::is_operator())
            .def("__repr__", [](const TimeSeriesData & data) {
                return std::format("TimeSeriesData(name={}, {})", py::repr(py::cast(data.Name)).cast<std::string>(), TimeSeriesLibrary::describeAxis(data));
            });
    }

    void bindQueries(py::module_ & mod)
    {
        mod.def("time_series_steps", &TimeSeriesLibrary::steps, py::arg("data"),
                "Number of timesteps the dataset provides (its shortest series).");
        mod.def("describe_axis", &TimeSeriesLibrary::describeAxis, py::arg("data"),
                "\"1 Jan 00:00, step 1 h, 8760 rows\"");
        mod.def("time_series_aligned", &TimeSeriesLibrary::aligned, py::arg("lhs"), py::arg("rhs"),
                "Same axis and same step count: the two can drive one simulation.");
        mod.def("time_series_alignment_issues", &TimeSeriesLibrary::alignmentIssues, py::arg("datasets"),
                "Empty when every dataset lines up with the first; otherwise one line per dataset.");
        mod.def("has_role", &TimeSeriesLibrary::hasRole, py::arg("data"), py::arg("role"));
        mod.def("values_for_role", &TimeSeriesLibrary::valuesForRole, py::arg("data"), py::arg("role"),
                "The series values for a role, or None when the dataset has no such series.");
        mod.def(
          "series_values",
          [](const TimeSeriesData & data, const SeriesRole role) {
              const auto values{TimeSeriesLibrary::valuesForRole(data, role)};
              if(!values.has_value())
              {
                  throw py::key_error(data.Name + " carries no " + TimeSeriesLibrary::seriesRoleToString(role) + " series");
              }
              return values.value();
          },
          py::arg("data"),
          py::arg("role"),
          "The series values for a role, which the dataset must carry: raises KeyError naming the dataset "
          "and the role otherwise. The form to hand a column straight to a boundary input.");
        mod.def("provided_roles", &TimeSeriesLibrary::providedRoles, py::arg("data"));
        mod.def("series_role_to_string", &TimeSeriesLibrary::seriesRoleToString, py::arg("role"));
        mod.def("series_role_from_string", &TimeSeriesLibrary::seriesRoleFromString, py::arg("value"));
        mod.def("series_role_strings", &TimeSeriesLibrary::seriesRoleStrings);
        mod.def("content_uuid", &TimeSeriesLibrary::contentUuid, py::arg("data"),
                "The deterministic UUID of the dataset's content (roles, values and axis): the same data "
                "imported twice lands on one record. Name, colour and the other metadata play no part.");
        mod.def("axis_after", &TimeSeriesLibrary::axisAfter, py::arg("axis"), py::arg("steps"),
                "The axis `steps` rows later, wrapping at the end of the nominal year.");
        mod.def(
          "time_series_window",
          [](const TimeSeriesData & data, const std::size_t firstStep, const std::size_t count) {
              return unwrap(TimeSeriesLibrary::window(data, firstStep, count));
          },
          py::arg("data"),
          py::arg("first_step"),
          py::arg("count"),
          "Rows [first_step, first_step + count) of every series as a dataset of its own, the axis moved to "
          "the first row kept and the UUID recomputed from the content: one month of a yearly weather file, "
          "say. Raises ValueError when the window is empty or reaches past the end.");
    }

    void bindLibrary(py::module_ & mod)
    {
        py::class_<TimeSeriesLibrary::DB>(mod, "TimeSeriesDB",
                                          "The time-series library: a list of datasets with lookup and edit.")
            .def(py::init<>())
            .def(py::init<const std::string &>(), py::arg("xml_file_name"))
            .def("load_from_string", &TimeSeriesLibrary::DB::loadFromString, py::arg("str"))
            .def("save_to_string", &TimeSeriesLibrary::DB::saveToString, py::arg("format") = FileParse::FileFormat::XML)
            .def("get_time_series_data", &TimeSeriesLibrary::DB::getTimeSeriesData, py::return_value_policy::reference_internal)
            .def("get_by_uuid", &TimeSeriesLibrary::DB::getByUUID, py::arg("uuid"))
            .def("get_by_name", &TimeSeriesLibrary::DB::getByName, py::arg("name"))
            .def("get_names", &TimeSeriesLibrary::DB::getNames)
            .def("add", &TimeSeriesLibrary::DB::add, py::arg("data"))
            .def("update", &TimeSeriesLibrary::DB::update, py::arg("data"))
            .def("update_or_add", &TimeSeriesLibrary::DB::updateOrAdd, py::arg("data"))
            .def("delete_with_uuid", &TimeSeriesLibrary::DB::deleteWithUUID, py::arg("uuid"))
            .def("is_dirty", &TimeSeriesLibrary::DB::isDirty);

        // THMZ storage: one archive entry per dataset, independent of any library file.
        mod.def("load_datasets", &TimeSeriesLibrary::loadDatasetsFromZipFile, py::arg("thmz_file_name"),
                "Every \"time series/<uuid>\" entry of a THMZ; empty for a pre-consolidation file.");
        mod.def("load_datasets_from_entries", &TimeSeriesLibrary::loadDatasetsFromEntries, py::arg("entries"),
                "Same, from an already-extracted archive dict.");
        mod.def("save_datasets", &TimeSeriesLibrary::saveDatasetsToZipFile,
                py::arg("datasets"), py::arg("thmz_file_name"), py::arg("format") = FileParse::FileFormat::XML,
                "Writes every dataset as its own entry; returns the number written.");
    }

    //! Where datasets come from: weather files, CSV files, and interior conditions
    //! generated from an exterior per standard.
    void bindCreation(py::module_ & mod)
    {
        namespace Epw = TimeSeriesLibrary::Epw;
        namespace Csv = TimeSeriesLibrary::Csv;
        namespace Standards = TimeSeriesLibrary::Standards;
        namespace Import = TimeSeriesLibrary::Import;

        py::class_<Epw::Location>(mod, "WeatherStation", "The station a weather file was recorded at.")
            .def_readonly("city", &Epw::Location::city)
            .def_readonly("region", &Epw::Location::region)
            .def_readonly("country", &Epw::Location::country)
            .def_readonly("wmo", &Epw::Location::wmo)
            .def_readonly("latitude", &Epw::Location::latitude)
            .def_readonly("longitude", &Epw::Location::longitude)
            .def_readonly("time_zone", &Epw::Location::timeZone)
            .def_readonly("elevation", &Epw::Location::elevation)
            .def("__repr__", [](const Epw::Location & location) {
                return std::format("WeatherStation(city={}, country={}, latitude={}, longitude={})",
                                   py::repr(py::cast(location.city)).cast<std::string>(),
                                   py::repr(py::cast(location.country)).cast<std::string>(),
                                   location.latitude,
                                   location.longitude);
            });
        py::class_<Epw::ImportResult>(mod,
                                      "EnergyPlusWeatherFile",
                                      "An EnergyPlus weather file as read: the dataset (AirTemperature, RelativeHumidity, "
                                      "WindSpeed, WindDirection and RadiantTemperature from the horizontal infrared), the "
                                      "station, and the fields no series role can express, by name.")
            .def_readonly("data", &Epw::ImportResult::data)
            .def_readonly("location", &Epw::ImportResult::location)
            .def_readonly("unmapped_columns", &Epw::ImportResult::unmappedColumns);
        mod.def(
          "read_epw",
          [](const std::filesystem::path & path) { return unwrap(Import::readWeather(path)); },
          py::arg("path"),
          "Reads an EnergyPlus weather file, or a zip holding exactly one (as weather files are distributed, "
          "beside their .ddy and .stat); the dataset is named after the file's stem and its UUID is its content. "
          "Raises ValueError when the file cannot be read.");
        mod.def(
          "read_epw_string",
          [](const std::string & content, const std::string & name) { return unwrap(Epw::readFromString(content, name)); },
          py::arg("content"),
          py::arg("name"),
          "The same from the file's text, under the given dataset name.");

        py::class_<Csv::ImportResult>(mod,
                                      "CsvImportResult",
                                      "A CSV read: the dataset, the headers that named no role and were skipped, and "
                                      "whether a time column set the axis.")
            .def_readonly("data", &Csv::ImportResult::data)
            .def_readonly("ignored_headers", &Csv::ImportResult::ignoredHeaders)
            .def_readonly("had_time_column", &Csv::ImportResult::hadTimeColumn);
        mod.def(
          "read_csv",
          [](const std::filesystem::path & path) { return unwrap(Csv::read(path.string())); },
          py::arg("path"),
          "Reads a CSV whose headers name series roles, in either spelling (AirTemperature or Air Temperature). A "
          "Time, Date, Datetime or Timestamp column sets the axis; evenly spaced rows are required. Blank cells "
          "carry the last reading forward. Raises ValueError when nothing can be read.");
        mod.def(
          "read_csv_string",
          [](const std::string & content, const std::string & name) { return unwrap(Csv::readFromString(content, name)); },
          py::arg("content"),
          py::arg("name"),
          "The same from the CSV text, under the given dataset name.");
        mod.def("write_csv", &Csv::writeToString, py::arg("data"),
                "The dataset as the CSV read_csv accepts: a time column on its axis, then one column per role.");
        mod.def(
          "write_csv_file",
          [](const TimeSeriesData & data, const std::filesystem::path & path) { Csv::writeToFile(data, path.string()); },
          py::arg("data"),
          py::arg("path"));
        mod.def("csv_header_for_role", &Csv::displayHeaderForRole, py::arg("role"),
                "The spelled-out column header of a role, as write_csv writes it (\"Air Temperature\").");

        py::enum_<Standards::Standard>(mod, "InteriorStandard", "The standard an interior climate is generated by.")
            .value("EN_15026_DIN_4108", Standards::Standard::EN_15026_DIN_4108)
            .value("ISO_13788", Standards::Standard::ISO_13788)
            .value("ASHRAE_160", Standards::Standard::ASHRAE_160);
        py::enum_<Standards::MoistureLoad>(mod, "MoistureLoad", "The indoor moisture load class of the standard.")
            .value("Normal", Standards::MoistureLoad::Normal)
            .value("High", Standards::MoistureLoad::High);
        mod.def(
          "generate_interior",
          [](const TimeSeriesData & exterior, const Standards::Standard standard, const Standards::MoistureLoad load, const std::size_t stepsPerDay) {
              return unwrap(Standards::generateInterior(exterior, standard, load, stepsPerDay));
          },
          py::arg("exterior"),
          py::arg("standard"),
          py::arg("load"),
          py::arg("steps_per_day") = 24U,
          "Indoor AirTemperature and RelativeHumidity derived from the exterior dataset's daily means per the "
          "standard, on the exterior's own axis and step count. The exterior must carry AirTemperature. Raises "
          "ValueError otherwise.");

        py::class_<Import::ImportedDataset>(mod, "ImportedDataset", "One dataset an import produced, with the CSV columns it skipped.")
            .def_readonly("data", &Import::ImportedDataset::data)
            .def_readonly("skipped_columns", &Import::ImportedDataset::skippedColumns);
        py::class_<Import::ImportOutcome>(mod, "ImportOutcome", "Every dataset a file held, and the zip entries that failed.")
            .def_readonly("datasets", &Import::ImportOutcome::datasets)
            .def_readonly("failed_entries", &Import::ImportOutcome::failedEntries);
        mod.def(
          "import_datasets",
          [](const std::filesystem::path & path) { return unwrap(Import::importDatasets(path)); },
          py::arg("path"),
          "Datasets from a file by its extension: .epw, .csv, or a .zip holding any number of either (each "
          "under its entry's stem). Raises ValueError for an unsupported extension, an unreadable file or a zip "
          "with nothing importable.");
        mod.def(
          "import_datasets_from_entries",
          [](const std::map<std::string, std::string> & entries) { return unwrap(Import::importDatasetsFromEntries(entries)); },
          py::arg("entries"),
          "The zip case on its contents: entry name -> text.");
        mod.def("supported_import_extensions", &Import::supportedExtensions);
    }
}   // namespace

//! Time-series datasets: the per-timestep values a transient boundary condition reads
//! (air temperature, humidity, film coefficient, ...) plus the time axis that also sets
//! the simulation clock. In a THMZ each dataset is its own "time series/<uuid>" entry.
//! Datasets come from weather files, CSV files, a standard's interior generation, or are
//! stated outright; a window of one is a dataset of its own.
void bind_time_series(py::module_ & mod)
{
    bindDataModel(mod);
    bindQueries(mod);
    bindLibrary(mod);
    bindCreation(mod);
}
