#include <algorithm>

#include <lbnl/algorithm.hxx>
#include <lbnl/optional.hxx>

#include "Converters.hxx"

#include "TimeSeriesData/ContentHash.hxx"

namespace BCLibrary
{
    namespace
    {
        using TimeSeriesLibrary::Channel;
        using TimeSeriesLibrary::ChannelRole;
        using TimeSeriesLibrary::TimeSeriesData;

        ///////////////////////////////////////////////////////////////////////////////////
        // Steady-state record conversion
        ///////////////////////////////////////////////////////////////////////////////////

        RadiationOptions convertRadiation(const BCSteadyStateLibrary::RadiationOptions & legacy)
        {
            return std::visit(
              [](const auto & option) -> RadiationOptions {
                  using OptionType = std::decay_t<decltype(option)>;
                  if constexpr(std::is_same_v<OptionType, BCSteadyStateLibrary::AutomaticEnclosure>)
                  {
                      return AutomaticEnclosure{Constant{option.emissivity}};
                  }
                  else if constexpr(std::is_same_v<OptionType, BCSteadyStateLibrary::ManualEnclosure>)
                  {
                      return ManualEnclosure{};
                  }
                  else if constexpr(std::is_same_v<OptionType, BCSteadyStateLibrary::BlackBodyRadiation>)
                  {
                      return BlackBodyRadiation{.temperature = Constant{option.temperature},
                                                .emissivity = Constant{option.emissivity},
                                                .viewFactor = option.viewFactor};
                  }
                  else
                  {
                      return FixedCoefficientRadiation{.temperature = Constant{option.temperature},
                                                       .coefficient = Constant{option.filmCoefficient}};
                  }
              },
              legacy);
        }

        Convection fixedConvection(double temperature, double filmCoefficient)
        {
            return Convection{.model = ConvectionModel::Fixed_Convection_Coefficient,
                              .airTemperature = Constant{temperature},
                              .filmCoefficient = Constant{filmCoefficient},
                              .windSpeed = std::nullopt,
                              .windDirection = std::nullopt};
        }

        struct SteadyDataVisitor
        {
            std::variant<SurfaceExchange, PrescribedState, RadiationSurface, NoExchange>
              operator()(const BCSteadyStateLibrary::Comprehensive & legacy) const
            {
                SurfaceExchange exchange;
                exchange.relativeHumidity = Constant{legacy.relativeHumidity};

                if(legacy.convection.has_value())
                {
                    exchange.convection =
                      fixedConvection(legacy.convection->temperature, legacy.convection->filmCoefficient);
                }

                if(legacy.constantFlux.has_value())
                {
                    exchange.flux = Constant{legacy.constantFlux->flux};
                }

                if(legacy.radiation.has_value() && legacy.radiation->radiation.has_value())
                {
                    exchange.radiation = convertRadiation(legacy.radiation->radiation.value());
                }

                return exchange;
            }

            std::variant<SurfaceExchange, PrescribedState, RadiationSurface, NoExchange>
              operator()(const BCSteadyStateLibrary::Simplified & legacy) const
            {
                SurfaceExchange exchange;
                exchange.relativeHumidity = Constant{legacy.relativeHumidity};
                exchange.convection = fixedConvection(legacy.temperature, legacy.filmCoefficient);
                return exchange;
            }

            std::variant<SurfaceExchange, PrescribedState, RadiationSurface, NoExchange>
              operator()(const BCSteadyStateLibrary::RadiationSurface & legacy) const
            {
                return RadiationSurface{.isDefault = legacy.isDefault,
                                        .temperature = legacy.temperature,
                                        .emissivity = legacy.emissivity};
            }
        };

        ///////////////////////////////////////////////////////////////////////////////////
        // Type record conversion
        ///////////////////////////////////////////////////////////////////////////////////

        ConvectionModel convertModel(BCTypesLibrary::ConvectionModel legacy)
        {
            switch(legacy)
            {
                case BCTypesLibrary::ConvectionModel::Fixed_Convection_Coefficient:
                    return ConvectionModel::Fixed_Convection_Coefficient;
                case BCTypesLibrary::ConvectionModel::ASHRAE_NFRC_Inside:
                    return ConvectionModel::ASHRAE_NFRC_Inside;
                case BCTypesLibrary::ConvectionModel::ASHRAE_NFRC_Outside:
                    return ConvectionModel::ASHRAE_NFRC_Outside;
                case BCTypesLibrary::ConvectionModel::TARP:
                    return ConvectionModel::TARP;
                case BCTypesLibrary::ConvectionModel::Yazdanian_Klems:
                    return ConvectionModel::Yazdanian_Klems;
                case BCTypesLibrary::ConvectionModel::Kimura:
                    return ConvectionModel::Kimura;
                case BCTypesLibrary::ConvectionModel::Montazeri:
                    return ConvectionModel::Montazeri;
            }
            return ConvectionModel::Fixed_Convection_Coefficient;
        }

        bool modelUsesWindSpeed(ConvectionModel model)
        {
            return model == ConvectionModel::ASHRAE_NFRC_Outside || model == ConvectionModel::Yazdanian_Klems
                   || model == ConvectionModel::Kimura || model == ConvectionModel::Montazeri;
        }

        bool modelUsesWindDirection(ConvectionModel model)
        {
            return model == ConvectionModel::Yazdanian_Klems || model == ConvectionModel::Kimura
                   || model == ConvectionModel::Montazeri;
        }

        //! Source inference for one input: transient records read from the environment;
        //! steady records carry the constant in the legacy record's optional value.
        Source inferSource(bool isTransient, ChannelRole role, const std::optional<double> & legacyValue,
                           double fallback)
        {
            if(isTransient)
            {
                return FromTimeSeries{role};
            }
            return Constant{legacyValue.value_or(fallback)};
        }

        Convection convertConvection(const BCTypesLibrary::ConvectionRecord & legacy, bool isTransient)
        {
            Convection convection;
            convection.model = convertModel(legacy.Model);
            convection.airTemperature =
              inferSource(isTransient, ChannelRole::AirTemperature, legacy.Temperature, 0.0);

            if(convection.model == ConvectionModel::Fixed_Convection_Coefficient)
            {
                convection.filmCoefficient =
                  inferSource(isTransient, ChannelRole::ConvectiveCoefficient, legacy.FilmCoefficient, 0.0);
            }

            if(modelUsesWindSpeed(convection.model))
            {
                convection.windSpeed = inferSource(isTransient, ChannelRole::WindSpeed, legacy.WindSpeed, 0.0);
            }

            if(modelUsesWindDirection(convection.model) && isTransient)
            {
                // The legacy record stores direction as a Leeward/Windward enum usable only
                // for steady runs; numeric per-timestep direction comes from the environment.
                convection.windDirection = FromTimeSeries{ChannelRole::WindDirection};
            }

            return convection;
        }

        RadiationOptions convertRadiation(const BCTypesLibrary::RadiationRecord & legacy, bool isTransient)
        {
            switch(legacy.Model)
            {
                case BCTypesLibrary::RadiationModel::Automatic_Enclosure:
                    return AutomaticEnclosure{Constant{legacy.SurfaceEmissivity.value_or(0.9)}};
                case BCTypesLibrary::RadiationModel::Black_Body_Radiation:
                    return BlackBodyRadiation{
                      .temperature =
                        inferSource(isTransient, ChannelRole::RadiantTemperature, legacy.Temperature, 0.0),
                      .emissivity =
                        inferSource(isTransient, ChannelRole::Emissivity, legacy.SurfaceEmissivity, 0.9),
                      .viewFactor = legacy.ViewFactor.value_or(1.0)};
                case BCTypesLibrary::RadiationModel::Fixed_Radiation_Coefficient:
                    return FixedCoefficientRadiation{
                      .temperature =
                        inferSource(isTransient, ChannelRole::RadiantTemperature, legacy.Temperature, 0.0),
                      .coefficient = inferSource(
                        isTransient, ChannelRole::RadiativeCoefficient, legacy.FilmCoefficient, 0.0)};
            }
            return AutomaticEnclosure{};
        }

        ///////////////////////////////////////////////////////////////////////////////////
        // Timestep file conversion
        ///////////////////////////////////////////////////////////////////////////////////

        void addChannel(TimeSeriesData & data, ChannelRole role, std::vector<double> values)
        {
            // One channel per role: first occurrence wins, matching the legacy content
            // validator's single-convection / single-radiation invariants.
            if(!TimeSeriesLibrary::hasRole(data, role) && !values.empty())
            {
                data.channels.emplace_back(Channel{role, std::move(values)});
            }
        }

        template<typename RowType, typename Getter>
        std::vector<double> column(const std::vector<RowType> & rows, Getter getter)
        {
            return lbnl::transform_to_vector(rows, getter);
        }

        template<typename RowType>
        void addConvectionChannels(TimeSeriesData & data, const std::vector<RowType> & rows)
        {
            if(rows.empty())
            {
                return;
            }

            addChannel(data, ChannelRole::AirTemperature,
                       column(rows, [](const RowType & row) { return row.temperature; }));
            addChannel(data, ChannelRole::RelativeHumidity,
                       column(rows, [](const RowType & row) { return row.humidity; }));

            if constexpr(requires(const RowType & row) { row.windSpeed; })
            {
                addChannel(data, ChannelRole::WindSpeed,
                           column(rows, [](const RowType & row) { return row.windSpeed; }));
            }

            if constexpr(requires(const RowType & row) { row.windDirection; })
            {
                addChannel(data, ChannelRole::WindDirection,
                           column(rows, [](const RowType & row) { return row.windDirection; }));
            }

            if constexpr(requires(const RowType & row) { row.fixedFilmCoefficient; })
            {
                addChannel(data, ChannelRole::ConvectiveCoefficient,
                           column(rows, [](const RowType & row) { return row.fixedFilmCoefficient; }));
            }
        }
    }   // namespace

    namespace
    {
        //! The legacy library expressed adiabatic as a fixed convection with a zero film
        //! coefficient and nothing else. That exchanges nothing - no heat, and by the
        //! Lewis relation no moisture either - so the conversion names it: the dedicated
        //! NoExchange kind, instead of carrying the degenerate zeros forward.
        bool convertsToNoExchange(
          const std::variant<SurfaceExchange, PrescribedState, RadiationSurface, NoExchange> & data)
        {
            const auto * exchange{std::get_if<SurfaceExchange>(&data)};
            if(exchange == nullptr || exchange->radiation.has_value() || exchange->solar.has_value()
               || exchange->flux.has_value())
            {
                return false;
            }
            if(!exchange->convection.has_value())
            {
                return true;
            }
            return lbnl::extend(exchange->convection->filmCoefficient)
              .and_then([](const Source & source) { return lbnl::get_if_opt<Constant>(source); })
              .transform([](const Constant & film) { return film.value == 0.0; })
              .value_or(false);
        }
    }   // namespace

    BoundaryCondition fromSteadyState(const BCSteadyStateLibrary::BoundaryCondition & legacy)
    {
        BoundaryCondition converted;
        converted.UUID = legacy.UUID;
        converted.Name = legacy.Name;
        converted.Protected = legacy.Protected;
        converted.Color = legacy.Color;
        converted.ProjectName = legacy.ProjectName;
        converted.isIGUSurface = legacy.isIGUSurface;
        converted.data = std::visit(SteadyDataVisitor{}, legacy.data);
        if(convertsToNoExchange(converted.data))
        {
            converted.data = NoExchange{};
        }
        return converted;
    }

    BoundaryCondition fromTypeRecord(const BCTypesLibrary::TypeRecord & legacy)
    {
        const bool isTransient{legacy.bcType == BCTypesLibrary::BCType::Transient};

        BoundaryCondition converted;
        converted.UUID = legacy.UUID;
        converted.Name = legacy.Name;
        converted.Protected = legacy.Protected;
        converted.Color = legacy.Color;
        converted.ProjectName = legacy.ProjectName;

        if(legacy.bcModel == BCTypesLibrary::BCModel::Dirichlet)
        {
            PrescribedState prescribed;
            if(legacy.UseTemperature)
            {
                prescribed.temperature =
                  inferSource(isTransient, ChannelRole::PrescribedTemperature, legacy.Temperature, 0.0);
            }
            if(legacy.UseHumidity)
            {
                prescribed.relativeHumidity =
                  inferSource(isTransient, ChannelRole::PrescribedHumidity, legacy.Humidity, 0.5);
            }
            converted.data = prescribed;
            return converted;
        }

        SurfaceExchange exchange;
        exchange.relativeHumidity =
          inferSource(isTransient, ChannelRole::RelativeHumidity, legacy.Humidity, 0.5);

        if(legacy.ConvectionBc.has_value())
        {
            exchange.convection = convertConvection(legacy.ConvectionBc.value(), isTransient);
        }

        if(legacy.RadiationBc.has_value())
        {
            exchange.radiation = convertRadiation(legacy.RadiationBc.value(), isTransient);
        }

        if(legacy.SolarBc.has_value())
        {
            exchange.solar =
              Solar{.irradiance = inferSource(
                      isTransient, ChannelRole::SolarIrradiance, legacy.SolarBc->SolarRadiation, 0.0),
                    .absorptance = legacy.SolarBc->Absorptance.value_or(0.6)};
        }

        if(legacy.UseHeatFlux)
        {
            exchange.flux = inferSource(isTransient, ChannelRole::HeatFlux, legacy.HeatFlux, 0.0);
        }

        converted.data = exchange;
        return converted;
    }

    TimeSeriesLibrary::TimeSeriesData
      environmentFromTimestep(const BCInputFileLibrary::BoundaryConditionTimestep & legacy,
                              const std::string & datasetName)
    {
        TimeSeriesData data;
        data.Name = datasetName;

        addConvectionChannels(data, legacy.convection.tarp);
        addConvectionChannels(data, legacy.convection.ashraeInside);
        addConvectionChannels(data, legacy.convection.ashraeOutside);
        addConvectionChannels(data, legacy.convection.fixedFilmCoefficient);
        addConvectionChannels(data, legacy.convection.yazdanianKlems);
        addConvectionChannels(data, legacy.convection.kimura);
        addConvectionChannels(data, legacy.convection.montazeri);

        addChannel(data, ChannelRole::RadiantTemperature,
                   column(legacy.radiation.fixedRadiation,
                          [](const BCInputFileLibrary::FixedRadiation & row) { return row.temperature; }));
        addChannel(data, ChannelRole::RadiativeCoefficient,
                   column(legacy.radiation.fixedRadiation,
                          [](const BCInputFileLibrary::FixedRadiation & row) { return row.hr; }));
        addChannel(data, ChannelRole::RadiantTemperature,
                   column(legacy.radiation.blackBodyRadiation,
                          [](const BCInputFileLibrary::BlackBodyRadiation & row) { return row.temperature; }));
        addChannel(data, ChannelRole::Emissivity,
                   column(legacy.radiation.blackBodyRadiation,
                          [](const BCInputFileLibrary::BlackBodyRadiation & row) { return row.emissivity; }));

        addChannel(data, ChannelRole::HeatFlux,
                   column(legacy.heatFlux,
                          [](const BCInputFileLibrary::HeatFlux & row) { return row.heatFlux; }));
        addChannel(data, ChannelRole::SolarIrradiance,
                   column(legacy.solarRadiation,
                          [](const BCInputFileLibrary::SolarRadiation & row) { return row.solarRadiation; }));
        addChannel(data, ChannelRole::PrescribedTemperature,
                   column(legacy.temperature,
                          [](const BCInputFileLibrary::FixedTemperature & row) { return row.temperature; }));
        addChannel(data, ChannelRole::PrescribedHumidity,
                   column(legacy.humidity,
                          [](const BCInputFileLibrary::FixedHumidity & row) { return row.humidity; }));

        data.UUID = TimeSeriesLibrary::contentUuid(data);
        return data;
    }
}   // namespace BCLibrary
