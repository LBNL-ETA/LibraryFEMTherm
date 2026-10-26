#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include <filesystem>
#include <format>
#include <optional>
#include <string>
#include <vector>

#include <lbnl/expected.hxx>

#include "LibraryUtilities/StableUuid.hxx"
#include "TimeSeriesData/ContentHash.hxx"
#include "TimeSeriesData/CsvSeries.hxx"
#include "TimeSeriesData/DB.hxx"
#include "TimeSeriesData/Epw.hxx"
#include "TimeSeriesData/Import.hxx"
#include "TimeSeriesData/Standards.hxx"
#include "TimeSeriesData/Tags.hxx"
#include "TimeSeriesData/TimeSeriesData.hxx"
#include "TimeSeriesData/Window.hxx"

#include "CalendarPosition.hxx"

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

    //! The steps as the axis keeps them; an empty list states no step at all.
    TimeSeriesLibrary::StepSeconds checkedSteps(const TimeSeriesLibrary::StepSeconds & stepSeconds)
    {
        const auto * lengths{std::get_if<std::vector<double>>(&stepSeconds)};
        if(lengths == nullptr)
        {
            return stepSeconds;
        }
        if(lengths->empty())
        {
            throw py::value_error("step_seconds needs at least one length");
        }
        return TimeSeriesLibrary::stepSecondsOf(*lengths);
    }

    void bindDatasetWarnings(py::module_ & mod)
    {
        using TimeSeriesLibrary::DatasetWarning;
        using TimeSeriesLibrary::DatasetWarningKind;
        py::enum_<DatasetWarningKind>(mod, "DatasetWarningKind", "What a dataset holds that a run does not refuse.")
            .value("SeriesLengthsDiffer", DatasetWarningKind::SeriesLengthsDiffer)
            .value("StepLengthsDiffer", DatasetWarningKind::StepLengthsDiffer);

        py::class_<DatasetWarning>(mod,
                                   "DatasetWarning",
                                   "One warning, as facts: `steps` is what a run takes, `stated` the count that "
                                   "differs from it. describe_warning words it.")
            .def_readonly("kind", &DatasetWarning::kind)
            .def_readonly("steps", &DatasetWarning::steps)
            .def_readonly("stated", &DatasetWarning::stated)
            .def("__eq__", [](const DatasetWarning & lhs, const DatasetWarning & rhs) { return lhs == rhs; }, py::is_operator())
            .def("__repr__", [](const DatasetWarning & warning) {
                return std::format("DatasetWarning(kind={}, steps={}, stated={})",
                                   py::str(py::cast(warning.kind)).cast<std::string>(),
                                   warning.steps,
                                   warning.stated);
            });

        mod.def("time_series_warnings", &TimeSeriesLibrary::datasetWarnings, py::arg("data"),
                "What the dataset holds that a run does not refuse but that may not be what was meant: series "
                "of different lengths, or a step-length list that does not match the steps. Empty when there is "
                "nothing to say. The run goes ahead regardless, on as many steps as the shortest series.");
        mod.def("describe_warning", &TimeSeriesLibrary::describeWarning, py::arg("data"), py::arg("warning"),
                "One warning in LibraryFEMTherm's words, the default; other words are built from its fields.");
        mod.def("describe_warnings", &TimeSeriesLibrary::describeWarnings, py::arg("data"),
                "Every warning the dataset raises, each in LibraryFEMTherm's words; empty when there is nothing "
                "to say.");
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

        py::enum_<TimeSeriesLibrary::Month>(mod, "Month", "The months by name, numbered from 1 as a time axis counts them.")
            .value("January", TimeSeriesLibrary::Month::January)
            .value("February", TimeSeriesLibrary::Month::February)
            .value("March", TimeSeriesLibrary::Month::March)
            .value("April", TimeSeriesLibrary::Month::April)
            .value("May", TimeSeriesLibrary::Month::May)
            .value("June", TimeSeriesLibrary::Month::June)
            .value("July", TimeSeriesLibrary::Month::July)
            .value("August", TimeSeriesLibrary::Month::August)
            .value("September", TimeSeriesLibrary::Month::September)
            .value("October", TimeSeriesLibrary::Month::October)
            .value("November", TimeSeriesLibrary::Month::November)
            .value("December", TimeSeriesLibrary::Month::December);

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

        auto axis{py::class_<TimeAxis>(mod,
                                       "TimeAxis",
                                       "Where row zero sits in the year and how far apart the rows are: a calendar "
                                       "position with no year, and `step_seconds`, one length in seconds for every "
                                       "step or a list of one length per step. A list whose lengths are all the same "
                                       "is kept as that one length.\n\n"
                                       "Month, day and hour count from 1, as an EnergyPlus weather file counts "
                                       "them: hour 1 is 00:00 to 01:00, and the first step starts `minute` minutes "
                                       "into its hour, minute counting from 0. The month may be given by name, "
                                       "`Month.January`, or by number. A position outside the calendar is a "
                                       "ValueError, never clamped.")
                    .def(py::init([](const CalendarPositionBinding::MonthLike & month,
                                     const std::size_t day,
                                     const std::size_t hour,
                                     const std::size_t minute,
                                     const TimeSeriesLibrary::StepSeconds & stepSeconds) {
                             return CalendarPositionBinding::checkedPosition(TimeAxis{.month = CalendarPositionBinding::positionNumber(month),
                                                                                      .day = day,
                                                                                      .hour = hour,
                                                                                      .minute = minute,
                                                                                      .stepSeconds = checkedSteps(stepSeconds)});
                         }),
                         py::arg("month") = 1U,
                         py::arg("day") = 1U,
                         py::arg("hour") = 1U,
                         py::arg("minute") = 0U,
                         py::arg("step_seconds") = 3600.0)};
        CalendarPositionBinding::addCalendarPosition(axis);
        axis.def_property(
              "step_seconds",
              [](const TimeAxis & axis) { return axis.stepSeconds; },
              [](TimeAxis & axis, const TimeSeriesLibrary::StepSeconds & stepSeconds) {
                  axis.stepSeconds = checkedSteps(stepSeconds);
              },
              "One length in seconds for every step, or a list of one length per step.")
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
                                   "Without a `uuid`, one is drawn at random, so no two datasets stated by "
                                   "hand share one; `content_uuid(data)` gives the UUID of the content instead.\n\n"
                                   "`series` is copied out as a Python list: build the list and "
                                   "assign it (`data.series = [...]`); appending to the returned "
                                   "list does not change the dataset.")
            .def(py::init([](std::string name,
                             std::vector<Series> series,
                             const TimeAxis & axis,
                             std::optional<std::string> uuid,
                             std::optional<std::string> projectName,
                             std::optional<std::string> description,
                             std::string color,
                             std::optional<std::string> source,
                             const bool isProtected) {
                     return TimeSeriesData{.UUID = uuid.value_or(LibraryCommon::randomUuid()),
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
                 py::arg("uuid") = py::none(),
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
        bindDatasetWarnings(mod);
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
                "The axis `steps` rows later, wrapping at the end of the nominal year, the remaining steps kept.");
        mod.def("start_label", &TimeSeriesLibrary::startLabel, py::arg("axis"),
                "Where the axis starts, as a clock reads it: \"1 Jan 00:00\" for hour 1, minute 0.");
        mod.def("axis_at", &TimeSeriesLibrary::axisAt, py::arg("seconds_into_year"),
                "The calendar position that many seconds into the nominal year, wrapping at either end, hourly.");
        mod.def("evenly_stepped", [](const TimeSeriesData & data) { return TimeSeriesLibrary::evenlyStepped(data.axis); },
                py::arg("data"), "Whether the dataset's axis states one length for every step.");
        mod.def(
          "step_lengths",
          [](const TimeSeriesData & data) { return TimeSeriesLibrary::stepLengths(data.axis, TimeSeriesLibrary::steps(data)); },
          py::arg("data"),
          "One length in seconds per step of the dataset, even or not.");
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

        py::enum_<Csv::StepColumn>(mod, "StepColumn", "Which CSV column states the steps.")
            .value("Time", Csv::StepColumn::Time,
                   "The Time column: a row's value belongs to the step ending at its time, so the first row is only "
                   "the start of the run and n rows make n-1 steps.")
            .value("Timestep", Csv::StepColumn::Timestep,
                   "The Timestep column (Timestep, Step, Step Seconds or dt): row i is step i, of the length in "
                   "seconds the row states. A Time column, if present, places the start one step before the first "
                   "stamp.");
        py::class_<Csv::FixedStep>(mod, "FixedStep",
                                   "Every row is one step of this many seconds. A Time column, if present, places the "
                                   "start one step before the first stamp.")
            .def(py::init([](const double seconds) { return Csv::FixedStep{.seconds = seconds}; }), py::arg("seconds"))
            .def_readwrite("seconds", &Csv::FixedStep::seconds)
            .def("__eq__", [](const Csv::FixedStep & lhs, const Csv::FixedStep & rhs) { return lhs == rhs; }, py::is_operator())
            .def("__repr__", [](const Csv::FixedStep & fixed) {
                return std::format("FixedStep(seconds={})", py::repr(py::cast(fixed.seconds)).cast<std::string>());
            });

        py::class_<Csv::ImportResult>(mod,
                                      "CsvImportResult",
                                      "A CSV read: the dataset, the headers that named no role and were skipped, which "
                                      "clock columns the file had, and whether the first row was only the start.")
            .def_readonly("data", &Csv::ImportResult::data)
            .def_readonly("ignored_headers", &Csv::ImportResult::ignoredHeaders)
            .def_readonly("had_time_column", &Csv::ImportResult::hadTimeColumn)
            .def_readonly("had_timestep_column", &Csv::ImportResult::hadTimestepColumn)
            .def_readonly("first_row_was_start", &Csv::ImportResult::firstRowWasStart,
                          "Under StepColumn.Time the first row was the start, not a step: its values were dropped.");
        mod.def(
          "read_csv",
          [](const std::filesystem::path & path, const Csv::StepChoice & steps) { return unwrap(Csv::read(path.string(), steps)); },
          py::arg("path"),
          py::arg("steps"),
          "Reads a CSV whose headers name series roles, in either spelling (AirTemperature or Air Temperature). "
          "`steps` says how rows become steps: StepColumn.Time reads the Time column, each row's value driving the "
          "step that ends at its time (the first row is only the start); StepColumn.Timestep reads one length in "
          "seconds per row from the Timestep column; FixedStep(seconds) makes every row one step of that length. "
          "Steps may be uneven. A column the choice asks for and the file lacks raises ValueError, as does anything "
          "unreadable. Blank value cells carry the last reading forward.");
        mod.def(
          "read_csv_string",
          [](const std::string & content, const std::string & name, const Csv::StepChoice & steps) {
              return unwrap(Csv::readFromString(content, name, steps));
          },
          py::arg("content"),
          py::arg("name"),
          py::arg("steps"),
          "The same from the CSV text, under the given dataset name.");
        mod.def("write_csv", &Csv::writeToString, py::arg("data"),
                "The dataset as the CSV read_csv accepts: a Time column stamping the end of each step, a Timestep "
                "column of the step lengths in seconds, then one column per role. Read back with StepColumn.Timestep "
                "it is the same dataset; with StepColumn.Time it is one step shorter.");
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
          [](const std::filesystem::path & path, const Csv::StepChoice & steps) {
              return unwrap(Import::importDatasets(path, steps));
          },
          py::arg("path"),
          py::arg("steps"),
          "Datasets from a file by its extension: .epw, .csv, or a .zip holding any number of either (each "
          "under its entry's stem). `steps` is how a CSV's rows become steps, as for read_csv; a weather file has "
          "its own clock and ignores it. Raises ValueError for an unsupported extension, an unreadable file or a "
          "zip with nothing importable.");
        mod.def(
          "import_datasets_from_entries",
          [](const std::map<std::string, std::string> & entries, const Csv::StepChoice & steps) {
              return unwrap(Import::importDatasetsFromEntries(entries, steps));
          },
          py::arg("entries"),
          py::arg("steps"),
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
