#pragma once

#include <string>
#include <vector>
#include <optional>

#include "Definitions.hxx"

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

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    ///   Database source information
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    /// @brief Source information from a WINDOW database record.
    struct WINDOW
    {
        std::string Path;
        std::string Name;
        int ID{0};
    };

    /// @brief Tracks the external database origin of a material's data.
    struct Database
    {
        std::optional<WINDOW> Window;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    ///   Material
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    //! \brief Material record is kept for displaying it in screen lists.
    //! After the frame-cavity refactor every material is a solid; the data member is
    //! a plain Solid (no variant). Cavity-typed and radiation-enclosure-typed
    //! materials from legacy files are intercepted and rewritten by the migration
    //! step before this record is constructed.
    struct Material
    {
        std::string UUID{};
        std::string Name{"Default Name"};

        std::optional<std::string> ProjectName;

        bool Protected{false};

        std::string Color{"0xFFFFFF"};

        std::optional<Database> database;

        Solid data;
    };

    Material generate(std::string uuid);
    Material generate();

    /// These functions ensure the sub-trees on the material's solid data exist.
    void ensureHygroThermal(Material & material);
    void ensureIntegrated(Material & material);
    void ensureIntegratedSolarAndVisible(Material & material);
}   // namespace MaterialsLibrary
