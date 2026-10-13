#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "THMZ/SteadyStateResults/Results.hxx"
#include "THMZ/SteadyStateResults/DB.hxx"
#include "THMZ/SteadyStateResults/UFactors.hxx"

namespace py = pybind11;

void bind_steady_state_results(py::module_ & mod)
{
    // --- Projection ---
    py::class_<ThermFile::Projection>(mod, "Projection")
        .def(py::init<>())
        .def_readwrite("length_type", &ThermFile::Projection::lengthType)
        .def_property("length",
            [](const ThermFile::Projection & proj) -> py::object {
                if(!proj.length.has_value()) return py::none();
                return py::cast(*proj.length);
            },
            [](ThermFile::Projection & proj, py::object val) {
                if(val.is_none()) { proj.length = std::nullopt; return; }
                proj.length = val.cast<double>();
            })
        .def_property("u_factor",
            [](const ThermFile::Projection & proj) -> py::object {
                if(!proj.uFactor.has_value()) return py::none();
                return py::cast(*proj.uFactor);
            },
            [](ThermFile::Projection & proj, py::object val) {
                if(val.is_none()) { proj.uFactor = std::nullopt; return; }
                proj.uFactor = val.cast<double>();
            })
        .def_property("projection_angle",
            [](const ThermFile::Projection & proj) -> py::object {
                if(!proj.projectionAngle.has_value()) return py::none();
                return py::cast(*proj.projectionAngle);
            },
            [](ThermFile::Projection & proj, py::object val) {
                if(val.is_none()) { proj.projectionAngle = std::nullopt; return; }
                proj.projectionAngle = val.cast<double>();
            });

    // --- SteadyStateUFactors ---
    py::class_<ThermFile::SteadyStateUFactors>(mod, "SteadyStateUFactors")
        .def(py::init<>())
        .def_readwrite("tag", &ThermFile::SteadyStateUFactors::tag)
        .def_readwrite("delta_t", &ThermFile::SteadyStateUFactors::deltaT)
        .def_readwrite("heat_flux", &ThermFile::SteadyStateUFactors::heatFlux)
        .def_readwrite("default_display_type", &ThermFile::SteadyStateUFactors::defaultDisplayType)
        .def_readwrite("projections", &ThermFile::SteadyStateUFactors::projections);

    // --- SteadyStateResultsCase ---
    py::class_<ThermFile::SteadyStateResultsCase>(mod, "SteadyStateResultsCase")
        .def(py::init<>())
        .def_property("model_type",
            [](const ThermFile::SteadyStateResultsCase & cas) -> py::object {
                if(!cas.modelType.has_value()) return py::none();
                return py::cast(*cas.modelType);
            },
            [](ThermFile::SteadyStateResultsCase & cas, py::object val) {
                if(val.is_none()) { cas.modelType = std::nullopt; return; }
                cas.modelType = val.cast<ThermFile::RunType>();
            })
        .def_property("simulation_error",
            [](const ThermFile::SteadyStateResultsCase & cas) -> py::object {
                if(!cas.simulationError.has_value()) return py::none();
                return py::cast(*cas.simulationError);
            },
            [](ThermFile::SteadyStateResultsCase & cas, py::object val) {
                if(val.is_none()) { cas.simulationError = std::nullopt; return; }
                cas.simulationError = val.cast<double>();
            })
        .def_property("glazing_case",
            [](const ThermFile::SteadyStateResultsCase & cas) -> py::object {
                if(!cas.glazingCase.has_value()) return py::none();
                return py::cast(*cas.glazingCase);
            },
            [](ThermFile::SteadyStateResultsCase & cas, py::object val) {
                if(val.is_none()) { cas.glazingCase = std::nullopt; return; }
                cas.glazingCase = val.cast<ThermFile::CMACase>();
            })
        .def_property("spacer_case",
            [](const ThermFile::SteadyStateResultsCase & cas) -> py::object {
                if(!cas.spacerCase.has_value()) return py::none();
                return py::cast(*cas.spacerCase);
            },
            [](ThermFile::SteadyStateResultsCase & cas, py::object val) {
                if(val.is_none()) { cas.spacerCase = std::nullopt; return; }
                cas.spacerCase = val.cast<ThermFile::CMACase>();
            })
        .def_readwrite("steady_state_u_factors", &ThermFile::SteadyStateResultsCase::steadyStateUFactors);

    // --- SteadyStateResults ---
    py::class_<ThermFile::SteadyStateResults>(mod, "SteadyStateResults")
        .def(py::init<>())
        .def_readwrite("version", &ThermFile::SteadyStateResults::version)
        .def_readwrite("cases", &ThermFile::SteadyStateResults::cases);

    // --- Free functions (overloaded saveToFile/saveToZipFile/saveToString) ---
    mod.def("load_steady_state_results_from_file",
            &ThermFile::loadSteadyStateResultsFromFile,
            py::arg("file_name"));

    mod.def("save_steady_state_results_to_file",
            static_cast<int(*)(const ThermFile::SteadyStateResults &, std::string_view,
                               FileParse::FileFormat)>(
                &ThermFile::saveToFile),
            py::arg("results"), py::arg("file_name"),
            py::arg("format") = FileParse::FileFormat::XML);

    mod.def("load_steady_state_results_from_zip_file",
            &ThermFile::loadSteadyStateResultsFromZipFile,
            py::arg("zip_file_name"));

    mod.def("save_steady_state_results_to_zip_file",
            static_cast<int(*)(const ThermFile::SteadyStateResults &, std::string_view,
                               FileParse::FileFormat)>(
                &ThermFile::saveToZipFile),
            py::arg("results"), py::arg("zip_file_name"),
            py::arg("format") = FileParse::FileFormat::XML);

    mod.def("load_steady_state_results_from_string",
            &ThermFile::loadSteadyStateResultsFromString,
            py::arg("str"), py::arg("format") = FileParse::FileFormat::XML);

    mod.def("save_steady_state_results_to_string",
            static_cast<std::string(*)(const ThermFile::SteadyStateResults &,
                                       FileParse::FileFormat)>(
                &ThermFile::saveToString),
            py::arg("results"), py::arg("format") = FileParse::FileFormat::XML);

    // --- Reading a U-factor the way THERM's results screen does ---
    mod.attr("DEGENERATE_PROJECTION_LENGTH") = ThermFile::degenerateProjectionLength;
    mod.def("is_degenerate", &ThermFile::isDegenerate, py::arg("projection"),
            "The projection has a length, and it is at or below DEGENERATE_PROJECTION_LENGTH.");
    mod.def("is_reportable", &ThermFile::isReportable, py::arg("projection"),
            "The projection carries a U-factor and a length, and the length is not degenerate.");
    mod.def("projection_of", &ThermFile::projectionOf, py::arg("tag"), py::arg("length_type"),
            "The tag's projection of that UValueDimensionType, or None.");
    mod.def("displayed_projection", &ThermFile::displayedProjection, py::arg("tag"),
            "The projection THERM shows for the tag: the one of its default display type, or None.");
    mod.def("displayed_u_factor", &ThermFile::displayedUFactor, py::arg("tag"),
            "The U-factor THERM shows for the tag; None when that projection is missing or not reportable.");
}
