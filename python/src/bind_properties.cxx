#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "THMZ/Properties/Properties.hxx"

namespace py = pybind11;

void bind_properties(py::module_ & mod)
{
    py::class_<ThermFile::ConstantInitialConditions>(mod, "ConstantInitialConditions")
        .def(py::init<>())
        .def_readwrite("temperature", &ThermFile::ConstantInitialConditions::temperature)
        .def_readwrite("humidity", &ThermFile::ConstantInitialConditions::humidity);

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
        .def_readwrite("relaxation_parameter", &ThermFile::EngineParameters::relaxationParameter)
        .def_readwrite("time_step", &ThermFile::EngineParameters::timeStep)
        .def_readwrite("number_of_timesteps", &ThermFile::EngineParameters::numberOfTimesteps);

    py::class_<ThermFile::MeshControl>(mod, "MeshControl")
        .def(py::init<>())
        .def_readwrite("mesh_type", &ThermFile::MeshControl::meshType)
        .def_readwrite("mesh_parameter", &ThermFile::MeshControl::meshParameter)
        .def_readwrite("run_error_estimator", &ThermFile::MeshControl::runErrorEstimator)
        .def_readwrite("error_energy_norm", &ThermFile::MeshControl::errorEnergyNorm)
        .def_readwrite("maximum_iterations", &ThermFile::MeshControl::maximumIterations);

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

    py::class_<ThermFile::CalculationOptions>(mod, "CalculationOptions")
        .def(py::init<>())
        .def_readwrite("simulation_engine", &ThermFile::CalculationOptions::simulationEngine)
        .def_readwrite("calculation_mode", &ThermFile::CalculationOptions::calculationMode)
        .def_readwrite("simulate_moisture", &ThermFile::CalculationOptions::simulateMoisture)
        .def_readwrite("simulate_thermal", &ThermFile::CalculationOptions::simulateThermal)
        .def_readwrite("modeling_options", &ThermFile::CalculationOptions::modelingOptions)
        .def_readwrite("transient_calculation_methodology", &ThermFile::CalculationOptions::transientCalculationMethodology)
        .def_readwrite("steady_state_calculation_methodology", &ThermFile::CalculationOptions::steadyStateCalculationMethodology)
        .def_readwrite("constant_initial_conditions_transient", &ThermFile::CalculationOptions::constantInitialConditionsTransient)
        .def_readwrite("constant_initial_conditions_steady_state", &ThermFile::CalculationOptions::constantInitialConditionsSteadyState)
        .def_readwrite("engine_parameters", &ThermFile::CalculationOptions::engineParameters)
        .def_readwrite("mesh_control", &ThermFile::CalculationOptions::meshControl)
        .def_readwrite("heat_transfer_modeling_options", &ThermFile::CalculationOptions::heatTransferModelingOptions)
        .def_readwrite("misc_properties", &ThermFile::CalculationOptions::miscProperties);

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
