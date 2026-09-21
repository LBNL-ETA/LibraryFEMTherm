#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "TimeSeriesData/DB.hxx"
#include "TimeSeriesData/Tags.hxx"
#include "TimeSeriesData/TimeSeriesData.hxx"

namespace py = pybind11;

//! Time-series datasets: the per-timestep values a transient boundary condition reads
//! (air temperature, humidity, film coefficient, ...) plus the time axis that also sets
//! the simulation clock. In a THMZ each dataset is its own "time series/<uuid>" entry.
void bind_time_series(py::module_ & mod)
{
    py::enum_<TimeSeriesLibrary::SeriesRole>(mod, "SeriesRole")
        .value("AirTemperature", TimeSeriesLibrary::SeriesRole::AirTemperature)
        .value("RelativeHumidity", TimeSeriesLibrary::SeriesRole::RelativeHumidity)
        .value("WindSpeed", TimeSeriesLibrary::SeriesRole::WindSpeed)
        .value("WindDirection", TimeSeriesLibrary::SeriesRole::WindDirection)
        .value("SolarIrradiance", TimeSeriesLibrary::SeriesRole::SolarIrradiance)
        .value("HeatFlux", TimeSeriesLibrary::SeriesRole::HeatFlux)
        .value("ConvectiveCoefficient", TimeSeriesLibrary::SeriesRole::ConvectiveCoefficient)
        .value("RadiantTemperature", TimeSeriesLibrary::SeriesRole::RadiantTemperature)
        .value("RadiativeCoefficient", TimeSeriesLibrary::SeriesRole::RadiativeCoefficient)
        .value("Emissivity", TimeSeriesLibrary::SeriesRole::Emissivity)
        .value("PrescribedTemperature", TimeSeriesLibrary::SeriesRole::PrescribedTemperature)
        .value("PrescribedHumidity", TimeSeriesLibrary::SeriesRole::PrescribedHumidity);

    py::class_<TimeSeriesLibrary::Series>(mod, "Series",
                                          "One role plus its values, one value per timestep.")
        .def(py::init<>())
        .def(py::init<TimeSeriesLibrary::SeriesRole, std::vector<double>>(),
             py::arg("role"), py::arg("values"))
        .def_readwrite("role", &TimeSeriesLibrary::Series::role)
        .def_readwrite("values", &TimeSeriesLibrary::Series::values);

    py::class_<TimeSeriesLibrary::TimeAxis>(mod, "TimeAxis",
                                            "Where row zero sits in the year and how far apart the rows are.")
        .def(py::init<>())
        .def_readwrite("month", &TimeSeriesLibrary::TimeAxis::month)
        .def_readwrite("day", &TimeSeriesLibrary::TimeAxis::day)
        .def_readwrite("hour", &TimeSeriesLibrary::TimeAxis::hour)
        .def_readwrite("minute", &TimeSeriesLibrary::TimeAxis::minute)
        .def_readwrite("step_seconds", &TimeSeriesLibrary::TimeAxis::stepSeconds);

    py::class_<TimeSeriesLibrary::TimeSeriesData>(mod, "TimeSeriesData",
                                                  "One dataset: identity, time axis and its series.\n\n"
                                                  "`series` is copied out as a Python list: build the list and "
                                                  "assign it (`data.series = [...]`); appending to the returned "
                                                  "list does not change the dataset.")
        .def(py::init<>())
        .def_readwrite("uuid", &TimeSeriesLibrary::TimeSeriesData::UUID)
        .def_readwrite("name", &TimeSeriesLibrary::TimeSeriesData::Name)
        .def_readwrite("project_name", &TimeSeriesLibrary::TimeSeriesData::ProjectName)
        .def_readwrite("protected_", &TimeSeriesLibrary::TimeSeriesData::Protected)
        .def_readwrite("description", &TimeSeriesLibrary::TimeSeriesData::Description)
        .def_readwrite("color", &TimeSeriesLibrary::TimeSeriesData::Color)
        .def_readwrite("source", &TimeSeriesLibrary::TimeSeriesData::Source)
        .def_readwrite("axis", &TimeSeriesLibrary::TimeSeriesData::axis)
        .def_readwrite("series", &TimeSeriesLibrary::TimeSeriesData::series);

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
    mod.def("provided_roles", &TimeSeriesLibrary::providedRoles, py::arg("data"));
    mod.def("series_role_to_string", &TimeSeriesLibrary::seriesRoleToString, py::arg("role"));
    mod.def("series_role_from_string", &TimeSeriesLibrary::seriesRoleFromString, py::arg("value"));
    mod.def("series_role_strings", &TimeSeriesLibrary::seriesRoleStrings);

    py::class_<TimeSeriesLibrary::DB>(mod, "TimeSeriesDB",
                                      "The time-series library: a list of datasets with lookup and edit.")
        .def(py::init<>())
        .def(py::init<const std::string &>(), py::arg("xml_file_name"))
        .def("load_from_string", &TimeSeriesLibrary::DB::loadFromString, py::arg("str"))
        .def("save_to_string", &TimeSeriesLibrary::DB::saveToString,
             py::arg("format") = FileParse::FileFormat::XML)
        .def("get_time_series_data", &TimeSeriesLibrary::DB::getTimeSeriesData,
             py::return_value_policy::reference_internal)
        .def("get_by_uuid", &TimeSeriesLibrary::DB::getByUUID, py::arg("uuid"))
        .def("get_by_name", &TimeSeriesLibrary::DB::getByName, py::arg("name"))
        .def("get_names", &TimeSeriesLibrary::DB::getNames)
        .def("add", &TimeSeriesLibrary::DB::add, py::arg("data"))
        .def("update", &TimeSeriesLibrary::DB::update, py::arg("data"))
        .def("update_or_add", &TimeSeriesLibrary::DB::updateOrAdd, py::arg("data"))
        .def("delete_with_uuid", &TimeSeriesLibrary::DB::deleteWithUUID, py::arg("uuid"))
        .def("is_dirty", &TimeSeriesLibrary::DB::isDirty);

    // THMZ storage: one archive entry per dataset, independent of any library file.
    mod.def("load_datasets_from_zip_file", &TimeSeriesLibrary::loadDatasetsFromZipFile,
            py::arg("zip_file_name"),
            "Every \"time series/<uuid>\" entry of a THMZ; empty for a pre-consolidation file.");
    mod.def("load_datasets_from_entries", &TimeSeriesLibrary::loadDatasetsFromEntries,
            py::arg("entries"),
            "Same, from an already-extracted archive dict.");
    mod.def("save_datasets_to_zip_file", &TimeSeriesLibrary::saveDatasetsToZipFile,
            py::arg("datasets"), py::arg("zip_file_name"),
            py::arg("format") = FileParse::FileFormat::XML,
            "Writes every dataset as its own entry; returns the number written.");
}
