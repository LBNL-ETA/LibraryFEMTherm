#pragma once

#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <map>
#include <utility>

#include "Definitions.hxx"
#include "Common.hxx"

namespace MaterialsLibrary
{
    /// @brief Struct representing a 2D point. It is used mainly to represent points in a material tables.
    ///
    /// This struct is used to represent a point in a 2D space with `x` and `y` coordinates.
    struct point
    {
        /// @brief Default constructor.
        ///
        /// Initializes a new instance of the point struct with default x and y values (0.0, 0.0).
        point() = default;

        /// @brief Parameterized constructor for point struct.
        ///
        /// Initializes a new instance of the point struct with provided x and y values.
        /// @param p_x The x-coordinate of the point.
        /// @param p_y The y-coordinate of the point.
        point(const double p_x, const double p_y) : x(p_x), y(p_y)
        {}

        /// @brief The x-coordinate of the point.
        double x{0.0};

        /// @brief The y-coordinate of the point.
        double y{0.0};
    };


    /// \struct HygroThermal
    /// \brief Represents the hygro-thermal properties of a material.
    ///
    /// This structure is used to model the physical and thermal properties of building materials,
    /// including their response to moisture and temperature changes.
    struct HygroThermal
    {
        /// \brief Default thickness of the material in meters.
        std::optional<double> DefaultThickness{};

        /// \brief Material information as a string.
        /// May include name, type, manufacturer, etc.
        std::optional<std::string> MaterialInformation{};

        /// \brief Bulk density of the material in kg/m^3.
        std::optional<double> BulkDensity{};

        /// \brief Porosity of the material as a fraction (0-1).
        std::optional<double> Porosity{};

        /// \brief Specific heat capacity of the dry material in J/(kg·K).
        std::optional<double> SpecificHeatCapacityDry{};

        /// \brief Thermal conductivity of the dry material in W/(m·K).
        std::optional<double> ThermalConductivityDry{};

        /// \brief Surface roughness category.
        std::optional<MaterialRoughness> Roughness{};

        /// \brief Water vapor diffusion resistance factor, dimensionless.
        std::optional<double> WaterVaporDiffusionResistanceFactor{};

        /// \brief Moisture storage function represented as a series of points.
        /// Each point maps a relative humidity to a moisture content value.
        std::optional<std::vector<point>> MoistureStorageFunction{};

        /// \brief Liquid transportation coefficient during suction phase,
        /// represented as a series of points.
        std::optional<std::vector<point>> LiquidTransportationCoefficientSuction{};

        /// \brief Liquid transportation coefficient during redistribution phase,
        /// represented as a series of points.
        std::optional<std::vector<point>> LiquidTransportationCoefficientRedistribution{};

        /// \brief Moisture-dependent thermal conductivity,
        /// represented as a series of points mapping moisture content to conductivity.
        std::optional<std::vector<point>> ThermalConductivityMoistureDependent{};

        /// \brief Temperature-dependent thermal conductivity,
        /// represented as a series of points mapping temperature to conductivity.
        std::optional<std::vector<point>> ThermalConductivityTemperatureDependent{};
    };

    /// \brief Enum representing optical properties of a material.
    struct OpticalProperties
    {
        double Transmittance{0};
        double Reflectance{0};
    };

    /// \brief Representing layer properties on two different surfaces.
    struct OpticalSide
    {
        OpticalProperties Front;
        OpticalProperties Back;
    };

    /// \brief Representing type of the optical ray coming to the layer.
    struct OpticalType
    {
        OpticalSide Direct;
        OpticalSide Diffuse;
    };

    /// \brief Represents only infrared part of the optical range
    struct Infrared
    {
        std::optional<double> Transmittance;
        std::optional<double> EmissivityFront;
        std::optional<double> EmissivityBack;
    };

    struct Integrated
    {
        Integrated() = default;
        Integrated(Infrared IR, OpticalType solar, OpticalType visible);
        Integrated(OpticalType solar, OpticalType visible);

        Infrared IR;
        std::optional<OpticalType> Solar;
        std::optional<OpticalType> Visible;
    };

    struct Optical
    {
        std::optional<Integrated> integrated{Integrated()};
    };

    struct Solid
    {
        std::optional<HygroThermal> hygroThermal{HygroThermal()};
        std::optional<Optical> optical{Optical()};
    };

    struct Cavity
    {
        std::optional<double> EmissivitySide1;
        std::optional<double> EmissivitySide2;

        CavityStandard cavityStandard{CavityStandard::NFRC};
        std::string GasName;
    };

    struct RadiationEnclosure
    {
        double emissivityDefault{0.9};
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    ///   Material
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    using MaterialVariant = std::variant<Solid, Cavity, RadiationEnclosure>;

    //! \brief Material record is kept for displaying it in screen lists.
    struct Material
    {
        Material() = default;

        std::string UUID{};
        std::string Name{"Default Name"};

        //! This is used to assign record to certain project.
        //! This property is not (and should not be) saved into the XML file.
        std::string ProjectName;

        bool Protected{false};

        std::string Color{"0xFFFFFF"};

        MaterialVariant data;
    };

    Material generate(std::string uuid, MaterialType type);
    Material generate(MaterialType type);


    /// These functions will create child elements only if material is already solid.
    void ensureHygroThermal(Material & material);
    void ensureIntegrated(Material & material);
    void ensureIntegratedSolarAndVisible(Material & material);

    bool isSolid(const Material&  material);
    bool isCavity(const Material&  material);
    bool isRadiationEnclosure(const Material&  material);

    std::string getMaterialType(const Material&  material);

    Solid * getSolid(Material&  material);
    const Solid * getSolid(const Material&  material);
    Cavity * getCavity(Material&  material);
    const Cavity * getCavity(const Material&  material);
    RadiationEnclosure * getRadiationEnclosure(Material&  material);
    const RadiationEnclosure * getRadiationEnclosure(const Material&  material);
}   // namespace MaterialsLibrary
