#include <cstddef>
#include <format>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "THMZ/Properties/Properties.hxx"

namespace py = pybind11;

namespace
{
    std::string conditionsRepr(const std::vector<ThermFile::StartCondition> & conditions)
    {
        std::string text;
        for(const auto & condition : conditions)
        {
            text += std::format("{}StartCondition(segment={}, bc_uuid='{}')",
                                text.empty() ? "" : ", ",
                                condition.segment,
                                condition.bcUUID);
        }
        return "[" + text + "]";
    }

    std::string startRepr(const ThermFile::UniformStart & start)
    {
        return std::format("UniformStart(temperature={}, humidity={})", start.temperature, start.humidity);
    }
}   // namespace

void bind_properties(py::module_ & mod)
{
    py::class_<ThermFile::UniformStart>(mod, "UniformStart",
                                        "A uniform state: temperature in degrees C, relative humidity as a fraction.")
        .def(py::init([](const double temperature, const double humidity) {
                 return ThermFile::UniformStart{.temperature = temperature, .humidity = humidity};
             }),
             py::arg("temperature") = 21.0,
             py::arg("humidity") = 0.5)
        .def_readwrite("temperature", &ThermFile::UniformStart::temperature)
        .def_readwrite("humidity", &ThermFile::UniformStart::humidity)
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def("__repr__", &startRepr);

    py::class_<ThermFile::StartCondition>(mod, "StartCondition",
                                          "A boundary segment held under another record while the starting state is "
                                          "solved: its index in the model's boundary list and the record it reads.")
        .def(py::init([](const std::size_t segment, std::string bcUUID) {
                 return ThermFile::StartCondition{.segment = segment, .bcUUID = std::move(bcUUID)};
             }),
             py::arg("segment"),
             py::arg("bc_uuid"))
        .def_readwrite("segment", &ThermFile::StartCondition::segment)
        .def_readwrite("bc_uuid", &ThermFile::StartCondition::bcUUID)
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def("__repr__", [](const ThermFile::StartCondition & condition) {
            return std::format("StartCondition(segment={}, bc_uuid='{}')", condition.segment, condition.bcUUID);
        });

    py::class_<ThermFile::SteadyStart>(mod, "SteadyStart",
                                       "The starting state is the steady state of the model with `conditions` "
                                       "swapped in; `start` is where that solve begins, None for the engine's default.")
        .def(py::init([](std::vector<ThermFile::StartCondition> conditions,
                         const std::optional<ThermFile::UniformStart> & start) {
                 return ThermFile::SteadyStart{.conditions = std::move(conditions), .start = start};
             }),
             py::arg("conditions") = std::vector<ThermFile::StartCondition>{},
             py::arg("start") = py::none())
        .def_readwrite("conditions", &ThermFile::SteadyStart::conditions)
        .def_readwrite("start", &ThermFile::SteadyStart::start)
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def("__repr__", [](const ThermFile::SteadyStart & start) {
            return std::format("SteadyStart(conditions={}, start={})",
                               conditionsRepr(start.conditions),
                               start.start.has_value() ? startRepr(start.start.value()) : std::string{"None"});
        });

    py::class_<ThermFile::TransientStart>(mod, "TransientStart",
                                          "The starting state is the end of a transient run over `step_lengths` "
                                          "(seconds) with `conditions` swapped in, itself started from `start`.")
        .def(py::init([](std::vector<ThermFile::StartCondition> conditions,
                         std::vector<double> stepLengths,
                         const ThermFile::UniformStart & start) {
                 return ThermFile::TransientStart{
                   .conditions = std::move(conditions), .stepLengths = std::move(stepLengths), .start = start};
             }),
             py::arg("conditions") = std::vector<ThermFile::StartCondition>{},
             py::arg("step_lengths") = std::vector<double>{},
             py::arg("start") = ThermFile::UniformStart{})
        .def_readwrite("conditions", &ThermFile::TransientStart::conditions)
        .def_readwrite("step_lengths", &ThermFile::TransientStart::stepLengths)
        .def_readwrite("start", &ThermFile::TransientStart::start)
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def("__repr__", [](const ThermFile::TransientStart & start) {
            return std::format("TransientStart(conditions={}, step_lengths=<{} steps>, start={})",
                               conditionsRepr(start.conditions),
                               start.stepLengths.size(),
                               startRepr(start.start));
        });

    py::class_<ThermFile::InitialConditions>(mod, "InitialConditions",
                                             "How a run's initial state is set: a UniformStart, or solved first by a "
                                             "SteadyStart or a TransientStart. A steady-state run reads only the "
                                             "uniform form, as the start of its iteration.")
        .def(py::init([](std::variant<ThermFile::UniformStart, ThermFile::SteadyStart, ThermFile::TransientStart> state) {
                 return ThermFile::InitialConditions{.state = std::move(state)};
             }),
             py::arg("state") = ThermFile::UniformStart{})
        .def_readwrite("state", &ThermFile::InitialConditions::state)
        .def(py::self == py::self)
        .def(py::self != py::self);

    py::class_<ThermFile::ModelingOptions>(mod, "ModelingOptions")
        .def(py::init<>())
        .def_readwrite("exclude_water_liquid_transportation", &ThermFile::ModelingOptions::excludeWaterLiquidTransportation)
        .def_readwrite("exclude_heat_of_evaporation", &ThermFile::ModelingOptions::excludeHeatOfEvaporation)
        .def_readwrite("exclude_capillary_conduction", &ThermFile::ModelingOptions::excludeCapillaryConduction)
        .def_readwrite("exclude_vapor_diffusion_conduction", &ThermFile::ModelingOptions::excludeVaporDiffusionConduction)
        .def_readwrite("exclude_latent_heat_of_fusion", &ThermFile::ModelingOptions::excludeLatentHeatOfFusion)
        .def_readwrite("exclude_thermal_conductivity_moisture_dependent", &ThermFile::ModelingOptions::excludeThermalConductivityMoistureDependent)
        .def_readwrite("exclude_thermal_conductivity_temperature_dependent", &ThermFile::ModelingOptions::excludeThermalConductivityTemperatureDependent);

    py::class_<ThermFile::EngineParameters>(mod, "EngineParameters")
        .def(py::init<>())
        .def_readwrite("convergence_tolerance", &ThermFile::EngineParameters::convergenceTolerance)
        .def_readwrite("relaxation_parameter", &ThermFile::EngineParameters::relaxationParameter);

    py::class_<ThermFile::MeshControl>(mod,
                                       "MeshControl",
                                       "THERM's mesh control: the mesher and its parameter, and the error estimator's "
                                       "target in percent and pass budget.")
        .def(py::init([](const bool runErrorEstimator,
                         const double errorEnergyNorm,
                         const std::size_t maximumIterations,
                         const std::size_t meshParameter) {
                 return ThermFile::MeshControl{.meshParameter = meshParameter,
                                               .runErrorEstimator = runErrorEstimator,
                                               .errorEnergyNorm = errorEnergyNorm,
                                               .maximumIterations = maximumIterations};
             }),
             py::arg("run_error_estimator") = true,
             py::arg("error_energy_norm") = 10.0,
             py::arg("maximum_iterations") = std::size_t{5u},
             py::arg("mesh_parameter") = std::size_t{6u})
        .def_readwrite("mesh_type", &ThermFile::MeshControl::meshType)
        .def_readwrite("mesh_parameter", &ThermFile::MeshControl::meshParameter)
        .def_readwrite("run_error_estimator", &ThermFile::MeshControl::runErrorEstimator)
        .def_readwrite("error_energy_norm", &ThermFile::MeshControl::errorEnergyNorm)
        .def_readwrite("maximum_iterations", &ThermFile::MeshControl::maximumIterations)
        .def(py::self == py::self)
        .def("__repr__", [](const ThermFile::MeshControl & control) {
            return std::format("MeshControl(run_error_estimator={}, error_energy_norm={}, maximum_iterations={}, "
                               "mesh_parameter={})",
                               control.runErrorEstimator ? "True" : "False",
                               control.errorEnergyNorm,
                               control.maximumIterations,
                               control.meshParameter);
        });

    py::class_<ThermFile::MiscProperties>(mod, "MiscProperties")
        .def(py::init<>())
        .def_readwrite("radiance_mode", &ThermFile::MiscProperties::radianceMode)
        .def_readwrite("use_cr_for_glazing_system", &ThermFile::MiscProperties::useCRforGlazingSystem)
        .def_readwrite("check_for_correct_window_bc_on_glazing_system", &ThermFile::MiscProperties::checkForCorrectWINDOWBCOnGlazingSystem);

    py::class_<ThermFile::HeatTransferModelingOptions>(mod, "HeatTransferModelingOptions")
        .def(py::init<>())
        .def_readwrite("automatically_adjust_relaxation_parameter", &ThermFile::HeatTransferModelingOptions::automaticallyAdjustRelaxationParameter)
        .def_readwrite("adjustment_step", &ThermFile::HeatTransferModelingOptions::adjustmentStep)
        .def_readwrite("maximum_iterations", &ThermFile::HeatTransferModelingOptions::maximumIterations)
        .def_readwrite("view_factor_smoothing", &ThermFile::HeatTransferModelingOptions::viewFactorSmoothing);

    py::class_<ThermFile::ModelOrientation>(mod, "ModelOrientation")
        .def(py::init<>())
        .def_readwrite("orientation", &ThermFile::ModelOrientation::orientation)
        .def_readwrite("cardinal_orientation", &ThermFile::ModelOrientation::cardinalOrientation);

    py::class_<ThermFile::General>(mod, "General")
        .def(py::init<>())
        .def_readwrite("calc_version", &ThermFile::General::calcVersion)
        .def_readwrite("file_name", &ThermFile::General::fileName)
        .def_readwrite("directory", &ThermFile::General::directory)
        .def_readwrite("creation_date", &ThermFile::General::creationDate)
        .def_readwrite("last_modified", &ThermFile::General::lastModified)
        .def_readwrite("creation_version", &ThermFile::General::creationVersion)
        .def_readwrite("last_modified_version", &ThermFile::General::lastModifiedVersion)
        .def_readwrite("title", &ThermFile::General::title)
        .def_readwrite("created_by", &ThermFile::General::createdBy)
        .def_readwrite("company", &ThermFile::General::company)
        .def_readwrite("client", &ThermFile::General::client)
        .def_readwrite("notes", &ThermFile::General::notes);

    py::class_<ThermFile::RadiationModelParameters>(mod, "RadiationModelParameters")
        .def(py::init<>());

    py::class_<ThermFile::FrameCavityProperties>(mod, "FrameCavityProperties")
        .def(py::init<>())
        .def_readwrite("convection_model", &ThermFile::FrameCavityProperties::convectionModel)
        .def_readwrite("radiation_calculation", &ThermFile::FrameCavityProperties::radiationCalculation)
        .def_readwrite("radiation_model_parameters", &ThermFile::FrameCavityProperties::radiationModelParameters);

    py::class_<ThermFile::CalculationOptions>(mod, "CalculationOptions")
        .def(py::init<>())
        .def_readwrite("simulation_engine", &ThermFile::CalculationOptions::simulationEngine,
                       "The engine THERM runs this file with. pyhygrotherm always solves with HygroThermFEM.")
        .def_readwrite("calculation_mode", &ThermFile::CalculationOptions::calculationMode)
        .def_readwrite("simulate_moisture", &ThermFile::CalculationOptions::simulateMoisture)
        .def_readwrite("simulate_thermal", &ThermFile::CalculationOptions::simulateThermal)
        .def_readwrite("modeling_options", &ThermFile::CalculationOptions::modelingOptions)
        .def_readwrite("initial_conditions", &ThermFile::CalculationOptions::initialConditions,
                       "How the run starts: UniformStart, SteadyStart or TransientStart in `.state`.")
        .def_readwrite("engine_parameters", &ThermFile::CalculationOptions::engineParameters)
        .def_readwrite("mesh_control", &ThermFile::CalculationOptions::meshControl)
        .def_readwrite("heat_transfer_modeling_options", &ThermFile::CalculationOptions::heatTransferModelingOptions)
        .def_readwrite("misc_properties", &ThermFile::CalculationOptions::miscProperties)
        .def_readwrite("frame_cavity_properties", &ThermFile::CalculationOptions::frameCavityProperties);

    py::class_<ThermFile::ExposureType>(mod, "ExposureType")
        .def(py::init<>())
        .def_readwrite("model_purpose", &ThermFile::ExposureType::modelPurpose)
        .def_property("cross_section",
            [](const ThermFile::ExposureType & exp) -> py::object {
                return std::visit([](const auto & val) -> py::object {
                    return py::cast(val);
                }, exp.crossSection);
            },
            [](ThermFile::ExposureType & exp, py::object val) {
                try { exp.crossSection = val.cast<ThermFile::WindowCrossSectionType>(); return; } catch(py::cast_error &) {}
                try { exp.crossSection = val.cast<ThermFile::OpaqueCrossSectionType>(); return; } catch(py::cast_error &) {}
                try { exp.crossSection = val.cast<ThermFile::OtherCrossSectionType>(); return; } catch(py::cast_error &) {}
                throw py::type_error("Expected WindowCrossSectionType, OpaqueCrossSectionType, or OtherCrossSectionType");
            })
        .def_readwrite("assembly_type", &ThermFile::ExposureType::assemblyType);

    py::class_<ThermFile::ModelExposure>(mod, "ModelExposure")
        .def(py::init<>())
        .def_readwrite("model_orientation", &ThermFile::ModelExposure::modelOrientation)
        .def_readwrite("gravity_orientation", &ThermFile::ModelExposure::gravityOrientation)
        .def_readwrite("exposure_type", &ThermFile::ModelExposure::exposureType);

    py::class_<ThermFile::Miscellaneous>(mod, "Miscellaneous")
        .def(py::init<>())
        .def_readwrite("float_tolerance", &ThermFile::Miscellaneous::floatTolerance)
        .def_readwrite("checking_tolerance", &ThermFile::Miscellaneous::checkingTolerance);

    py::class_<ThermFile::Properties>(mod, "Properties")
        .def(py::init<>())
        .def_readwrite("general", &ThermFile::Properties::general)
        .def_readwrite("calculation_options", &ThermFile::Properties::calculationOptions)
        .def_readwrite("model_exposure", &ThermFile::Properties::modelExposure)
        .def_readwrite("miscellaneous", &ThermFile::Properties::miscellaneous);
}
